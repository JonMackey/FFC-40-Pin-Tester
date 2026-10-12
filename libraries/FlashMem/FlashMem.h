/*
*	FlashMem.h, Copyright Jonathan Mackey 2018-2025
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
#ifndef FlashMem_H
#define FlashMem_H
#ifndef __MACH__
#include <SPI.h>
#endif
#include "PlatformDefs.h"

class FlashMem
{
public:
#ifdef __MACH__
							FlashMem(void);
#else
							FlashMem(
								pin_t					inCSPin,
								SPIClass&				inSPI);
	void					begin(void);
	void					LoadJEDECInfo(void);
	uint8_t					GetManufacturerID(void)
								{return(mManufacturerID);}
	uint8_t					GetMemoryType(void)
								{return(mMemoryType);}
#endif
	uint32_t				GetCapacity(void)
								{return(mCapacity);}
	bool					ChipErase(void);
	bool					Erase64KBlock(
								uint32_t				inAddr);
	bool					Erase32KBlock(
								uint32_t				inAddr);
	bool					EraseSector(
								uint32_t				inAddr);
	bool					WriteSector(
								uint32_t				inSector,
								const uint8_t*			inData);
	bool					WritePage(
								uint32_t				inAddr,
								const uint8_t*			inData);
	bool					ReadSector(
								uint32_t				inSector,
								uint8_t*				outData);
	bool					Read(
								uint32_t				inAddr,
								uint32_t				inDataLen,
								uint8_t*				outData);
//	inline const uint32_t	GetSectorSize(void) const
//								{return(kSectorSize);}

	static const uint32_t		kSectorSize = 0x1000;
protected:
	uint32_t	mCapacity;
#ifdef __MACH__
	uint8_t*		mVolMem;
#else
	pin_t		mCSPin;
	SPIClass&	mSPI;
	SPISettings	mSPISettings;
	uint8_t		mManufacturerID;
	uint8_t		mMemoryType;

	void					SendCmd(
								uint8_t					inCmd);
	void					Select(void);
	void					Unselect(void);
	bool					WaitTillReady(
								uint32_t				inTimeout = 100);
	bool					WriteEnable(void);
	void					WriteDisable(void);
#endif
};

#endif
