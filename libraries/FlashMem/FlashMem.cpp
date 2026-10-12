/*
*	FlashMem.cpp, Copyright Jonathan Mackey 2018-2025
*
*	GNU license:
*	This program is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*	Please maintain this license information along with authorship and copyright
*	notices in any redistribution of this code.
*/
#include "FlashMem.h"


/*
	8MB Flash Memory Map

	block 0		= 0x000000
		page 0	+ 0x000
		page 1	+ 0x100
		page 2	+ 0x200
		page 255 + 0xFF00
		
		sector 0	+ 0x0000
		sector 1	+ 0x1000
		sector 2	+ 0x2000
		sector 15	+ 0xF000
	block 1		= 0x010000
	block 2		= 0x020000
	block 127	= 0x7F0000
*/

enum
{
	ePageProgCmd	= 2,
	eReadDataCmd,
	eWriteDisableCmd,
	eReadStat1Cmd,
	eWriteEnableCmd,
	eFastReadDataCmd = 0x0B,
	eEraseSectorCmd	= 0x20,
	eErase32KBlkCmd	= 0x52,
	eChipEraseCmd	= 0x60,
	eErase64KBlkCmd	= 0xD8,
	eReadJEDECIDCmd	= 0x9F,
	
	eChipBusyBit	=	1,
	eWriteEnabledBit
};

#ifdef __MACH__
/*********************************** FlashMem ************************************/
FlashMem::FlashMem(void)
{
	mCapacity = 0x800000;
	mVolMem = new uint8_t[0x800000];
	ChipErase();
}
#else
/*********************************** FlashMem ************************************/
FlashMem::FlashMem(
	pin_t		inCSPin,
	SPIClass&	inSPI)
	// According to the Winbond doc their chips are rated at 133MHz for 25QxxJV
	// and 104MHz for 25QxxFV
	: mCSPin(inCSPin), mSPI(inSPI), mSPISettings(104000000, MSBFIRST, SPI_MODE0)
	//: mCSPin(inCSPin), mSPI(inSPI), mSPISettings(133000000, MSBFIRST, SPI_MODE0)
{
}
#endif
#ifndef __MACH__
/*********************************** begin ************************************/
void FlashMem::begin(void)
{
	pinMode(mCSPin, OUTPUT);
	digitalWrite(mCSPin, HIGH);
	LoadJEDECInfo();
}

/********************************** Select ************************************/
void FlashMem::Select(void)
{
	mSPI.beginTransaction(mSPISettings);
	digitalWrite(mCSPin, LOW);
}

/********************************* Unselect ***********************************/
void FlashMem::Unselect(void)
{
	digitalWrite(mCSPin, HIGH);
	mSPI.endTransaction();
}

/******************************* LoadJEDECInfo ********************************/
void FlashMem::LoadJEDECInfo(void)
{
	Select();
	uint8_t	transBuf[4];
	transBuf[0] = eReadJEDECIDCmd;
	mSPI.transfer(transBuf, 4);
	mManufacturerID = transBuf[1];
	mMemoryType = transBuf[2];
	mCapacity = 1L<<(transBuf[3]);
	Unselect();
}

/********************************** SendCmd ***********************************/
void FlashMem::SendCmd(
	uint8_t	inCmd)
{
	Select();
	mSPI.transfer(inCmd);
}

/******************************* WaitTillReady ********************************/
bool FlashMem::WaitTillReady(
	uint32_t	inTimeout)
{
	uint32_t	timeout = millis() + inTimeout;
	uint8_t		transBuf[2];

	do {
		Select();
		transBuf[0] = eReadStat1Cmd;
		mSPI.transfer(transBuf, 2);
		Unselect();
		if (millis() < timeout)
		{
			continue;
		}
		return(false);
	} while(transBuf[1] & eChipBusyBit);

	return(true);
}
#endif

/******************************** WriteSector *********************************/
bool FlashMem::WriteSector(
	uint32_t		inSector,
	const uint8_t*	inData)
{
	bool success = true;
	const uint8_t*	dataPtr = inData;
	uint32_t	addr = inSector*kSectorSize;
	const uint32_t	numPages = kSectorSize/0x100;
	EraseSector(addr);
	for (uint32_t i = 0; i < numPages; i++, addr+=0x100, dataPtr+=0x100)
	{
		if (WritePage(addr, dataPtr))
		{
			continue;
		}
		success = false;
		break;
	}
	return(success);
}

#ifndef __MACH__
/********************************* WritePage **********************************/
bool FlashMem::WritePage(
	uint32_t		inAddr,
	const uint8_t*	inData)
{
	bool	success = inAddr < mCapacity && WaitTillReady() && WriteEnable();
	uint8_t		transBuf[4];
	
	if (success)
	{
		uint8_t	pageBuf[256];
		memcpy(pageBuf, inData, 256);
		Select();
		transBuf[0] = ePageProgCmd;
		transBuf[1] = inAddr >> 16;
		transBuf[2] = inAddr >> 8;
		transBuf[3] = 0;
		
		mSPI.transfer(transBuf, 4);
		mSPI.transfer(pageBuf, 256);
		Unselect();
		success = WaitTillReady();
		WriteDisable();
	}
	return(success);
}
#else
/********************************* WritePage **********************************/
bool FlashMem::WritePage(
	uint32_t		inAddr,
	const uint8_t*	inData)
{
	memcpy(&mVolMem[inAddr&0xFFFFF00], inData, 256);
	return true;
}

#endif
/********************************* ReadSector *********************************/
bool FlashMem::ReadSector(
	uint32_t		inSector,
	uint8_t*		outData)
{
	return(Read(inSector*kSectorSize, kSectorSize, outData));
}

#ifndef __MACH__
/*********************************** Read *************************************/
bool FlashMem::Read(
	uint32_t	inAddr,
	uint32_t	inDataLen,
	uint8_t*	outData)
{
	bool	success = inAddr+inDataLen <= mCapacity && WaitTillReady();
	if (success)
	{
		uint8_t		transBuf[5];
		Select();
	#if 1
		transBuf[0] = eFastReadDataCmd;
		transBuf[1] = inAddr >> 16;
		transBuf[2] = inAddr >> 8;
		transBuf[3] = inAddr;
		transBuf[4] = 0;
		mSPI.transfer(transBuf, 5);
	#else
		transBuf[0] = eReadDataCmd;
		transBuf[1] = inAddr >> 16;
		transBuf[2] = inAddr >> 8;
		transBuf[3] = inAddr;
		mSPI.transfer(transBuf, 4);
	#endif
		mSPI.transfer(outData, inDataLen);
		Unselect();
	}
	return(success);
}

/******************************** WriteEnable *********************************/
bool FlashMem::WriteEnable(void)
{
	SendCmd(eWriteEnableCmd);
	Unselect();
	uint8_t		transBuf[2];
	do {
		Select();
		transBuf[0] = eReadStat1Cmd;
		mSPI.transfer(transBuf, 2);
		Unselect();
	} while(transBuf[1] & eChipBusyBit);
	
	return((transBuf[1] & eWriteEnabledBit) != 0);
}

/******************************** WriteDisable ********************************/
void FlashMem::WriteDisable(void)
{
	SendCmd(eWriteDisableCmd);
	Unselect();
	WaitTillReady(1000);
}

/********************************* EraseSector ********************************/
bool FlashMem::EraseSector(
	uint32_t	inAddr)
{

	bool success = WaitTillReady() && WriteEnable();
	if (success)
	{
		Select();
		uint8_t		transBuf[4];
		transBuf[0] = eEraseSectorCmd;
		transBuf[1] = inAddr >> 16;
		transBuf[2] = inAddr >> 8;
		transBuf[3] = inAddr;
		
		mSPI.transfer(transBuf, 4);
		Unselect();
		success = WaitTillReady(1000);	// Max time to erase a sector is 400ms
		WriteDisable();
	}
	return success;
}

/******************************** Erase32KBlock ********************************/
bool FlashMem::Erase32KBlock(
	uint32_t	inAddr)
{

	bool success = WaitTillReady() && WriteEnable();
	if (success)
	{
		Select();
		uint8_t		transBuf[4];
		transBuf[0] = eErase32KBlkCmd;
		transBuf[1] = inAddr >> 16;
		transBuf[2] = inAddr >> 8;
		transBuf[3] = inAddr;
		
		mSPI.transfer(transBuf, 4);
		Unselect();
		success = WaitTillReady(2000);	// Max time to erase a 32K block is 1.6s
		WriteDisable();
	}
	return success;
}

/******************************* Erase64KBlock ********************************/
bool FlashMem::Erase64KBlock(
	uint32_t	inAddr)
{

	bool success = WaitTillReady() && WriteEnable();
	if (success)
	{
		Select();
		uint8_t		transBuf[4];
		transBuf[0] = eErase64KBlkCmd;
		transBuf[1] = inAddr >> 16;
		transBuf[2] = inAddr >> 8;
		transBuf[3] = inAddr;
		
		mSPI.transfer(transBuf, 4);
		Unselect();
		success = WaitTillReady(2500);	// Max time to erase a 64K block is 2s
		WriteDisable();
	}
	return success;
}

/********************************* ChipErase **********************************/
bool FlashMem::ChipErase(void)
{
	bool success = WaitTillReady() && WriteEnable();
	if (success)
	{
		SendCmd(eChipEraseCmd);
		Unselect();
		// From AC Electrical Characteristics:
		//	Chip Erase Time 08 = 6 seconds
		//	Chip Erase Time 32 = 15 seconds
		//	Chip Erase Time 64 = 100 seconds
		success = WaitTillReady(150 * 1000UL);	// Max time to erase a 8MB chip (W25Q64)
		WriteDisable();
	}
	return success;
}
#else
/*********************************** Read *************************************/
bool FlashMem::Read(
	uint32_t	inAddr,
	uint32_t	inDataLen,
	uint8_t*	outData)
{
	memcpy(outData, &mVolMem[inAddr], inDataLen);
	return true;
}

/********************************* EraseSector ********************************/
bool FlashMem::EraseSector(
	uint32_t	inAddr)
{
	memset(&mVolMem[inAddr], 0xFF, 0x1000);
	return true;
}

/******************************** Erase32KBlock ********************************/
bool FlashMem::Erase32KBlock(
	uint32_t	inAddr)
{
	memset(&mVolMem[inAddr], 0xFF, 0x8000);
	return true;
}

/******************************* Erase64KBlock ********************************/
bool FlashMem::Erase64KBlock(
	uint32_t	inAddr)
{
	memset(&mVolMem[inAddr], 0xFF, 0x10000);
	return true;
}

/********************************* ChipErase **********************************/
bool FlashMem::ChipErase(void)
{
	memset(mVolMem, 0xFF, 0x800000);
	return true;
}
#endif
