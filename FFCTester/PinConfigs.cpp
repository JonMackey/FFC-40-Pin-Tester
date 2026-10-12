/*
*	PinConfigs.cpp, Copyright Jonathan Mackey 2026
*	Class to manage pin configurations.
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
*
*/
#include "PinConfigs.h"
#include "DataStream.h"
#include <string.h>
#include "CSVUtils.h"

#ifndef __MACH__
#include <Arduino.h>
#include "SdFat.h"
#include "sdios.h"
#include "UnixTime.h"
#else
#include <stdio.h>
#endif

const uint16_t SPinConfigRoot::kRootFormatted = 1234;

/********************************* PinConfigs *********************************/
PinConfigs::PinConfigs(void)
	: mDataStream(nullptr), mCurrentIndex(0), mCount(0)
{
}

/************************************ begin ***********************************/
void PinConfigs::begin(
	DataStream*	inDataStream,
	bool		inFormat)
{
	mDataStream = inDataStream;
	mDataStream->Seek(-(int32_t)sizeof(SPinConfigRoot), DataStream::eSeekEnd);
	mMaxLinkCount = mDataStream->GetPos()/sizeof(SPinsDataLink);
	ReadRoot();
	if (mRoot.formatted == SPinConfigRoot::kRootFormatted &&
		!inFormat)
	{
		if (mRoot.tail)
		{
			uint16_t	count = 1;
			ReadLink(mRoot.tail, &mCurrent);
			while (mCurrent.prev)
			{
				count++;
				ReadLink(mCurrent.prev, &mCurrent);
			}
			mCurrentIndex = mRoot.head;
			mCount = count;
		}
	/*
	*	Else, format the root
	*/
	} else
	{
		FormatRoot();
	}
}

/********************************* FormatRoot *********************************/
void PinConfigs::FormatRoot(void)
{
	mRoot.head = 0;
	mRoot.tail = 0;
	mRoot.freeHead = 0;
	mRoot.formatted = SPinConfigRoot::kRootFormatted;
	WriteRoot();
	mCount = 0;
	mCurrentIndex = 0;
}

/******************************** GetNextIndex ********************************/
/*
*	Gets the next sorted physical index without loading
*/
uint16_t PinConfigs::GetNextIndex(
	bool	inWrap) const
{
	uint16_t	index = 0;
	if (mCurrentIndex != 0)
	{
		if (mCurrent.next)
		{
			index = mCurrent.next;
		} else if (inWrap &&
			mCurrent.prev)
		{
			index = mRoot.head;
		}
	}
	return(index);
}

/************************************ Next ************************************/
bool PinConfigs::Next(
	bool	inWrap)
{
	uint16_t	nextindex = GetNextIndex(inWrap);
	bool	success = nextindex != 0;
	if (success)
	{
		GoToLink(nextindex);
	}
	return(success);
}

/****************************** GetPreviousIndex ******************************/
uint16_t PinConfigs::GetPreviousIndex(
	bool	inWrap) const
{
	uint16_t	index = 0;
	if (mCurrentIndex != 0)
	{
		if (mCurrent.prev)
		{
			index = mCurrent.prev;
		} else if (inWrap &&
			mCurrent.next)
		{
			index = mRoot.tail;
		}
	}
	return(index);
}

/********************************** Previous **********************************/
bool PinConfigs::Previous(
	bool	inWrap)
{
	uint16_t	prevIndex = GetPreviousIndex(inWrap);
	bool	success = prevIndex != 0;
	if (success)
	{
		GoToLink(prevIndex);
	}
	return(success);
}

/********************************** GoToLink **********************************/
/*
*	Does no bounds checking.
*/
void PinConfigs::GoToLink(
	uint16_t	inRecIndex)
{
	if (mCurrentIndex != inRecIndex)
	{
		mCurrentIndex = inRecIndex;
		ReadLink(inRecIndex, &mCurrent);
	}
}

/******************************** GoToNthLink *********************************/
/*
*	Returns true if there is an Nth link.
*/
bool PinConfigs::GoToNthLink(
	uint16_t	inLogIndex)
{
	bool	success = mRoot.head != 0;
	if (success)
	{
		GoToLink(mRoot.head);
		
		for (uint16_t i = 0; i < inLogIndex; i++)
		{
			if (mCurrent.next)
			{
				GoToLink(mCurrent.next);
			} else
			{
				success = false;
				break;
			}
		}
	}
	return(success);
}

/******************************** GoToNamedLink *******************************/
/*
*	Returns true if there is a link inName.
*/
bool PinConfigs::GoToNamedLink(
	const char*	inName)
{
	bool	success = false;
	if (inName != nullptr)
	{
		int16_t leftIndex = 0;
		int16_t	currLogIndex = GetLogicalIndex();
		if (currLogIndex >= 0)
		{
			int16_t current = 0;
			int16_t rightIndex = mCount -1;
			while (leftIndex <= rightIndex)
			{
				current = (leftIndex + rightIndex) / 2;
				GoToRelativeLink(current - currLogIndex);
				currLogIndex = current;
				
				int	cmpResult = strcmp(mCurrent.data.name, inName);
				if (cmpResult == 0)
				{
					success = true;
					break;
				} else if (cmpResult > 0)
				{
					rightIndex = current - 1;
				} else
				{
					leftIndex = current + 1;
				}
			}
		}
	}
	return(success);
}

/****************************** GoToRelativeLink ******************************/
/*
*	Starting from the current link, go to the logical position relative to
*	the current link by inRelLogIndex. 
*/
bool PinConfigs::GoToRelativeLink(
	int16_t	inRelLogIndex)
{
	bool success = true;
	if (inRelLogIndex > 0)
	{
		do
		{
			success = Next(false);
			inRelLogIndex--;
		} while (success && inRelLogIndex);
	} else if (inRelLogIndex < 0)
	{
		do
		{
			success = Previous(false);
			inRelLogIndex++;
		} while (success && inRelLogIndex);
	}
	return(success);
}

/****************************** GetLogicalIndex *******************************/
/*
*	Starting from the root head, count indexes till the current link is hit.
*	-1 is returned if there is no current.  This should only happen when there
*	are no links.
*/
int16_t PinConfigs::GetLogicalIndex(void) const
{
	int16_t	logIndex = 0;
	if (mCurrentIndex)
	{
		SPinsDataLink	thisLink;
		uint16_t	prev = mCurrent.prev;
		while (prev)
		{
			logIndex++;
			ReadLink(prev, &thisLink);
			prev = thisLink.prev;
		}
	} else
	{
		logIndex = -1;
	}
	return(logIndex);
}

/********************************** AddConfig *********************************/
uint16_t PinConfigs::AddConfig(
	const char*		inName,
	const uint8_t*	inConfig)
{
	SPinsDataLink	link;
	link.data.SetName(inName);
	link.data.SetConfig(inConfig);
	return(AddLink(link));
}

/********************************** AddLink ***********************************/
/*
*	The added link becomes the current link.
*/
uint16_t PinConfigs::AddLink(
	SPinsDataLink&	inLink)
{
	int16_t leftIndex = 0;
	int16_t	currLogIndex = GetLogicalIndex();
	if (currLogIndex >= 0)
	{
		int16_t current = 0;
		int16_t rightIndex = mCount -1;
		const char*	linkName = inLink.data.name;
		while (leftIndex <= rightIndex)
		{
			current = (leftIndex + rightIndex) / 2;
			GoToRelativeLink(current - currLogIndex);
			currLogIndex = current;
			
			int	cmpResult = strcmp(mCurrent.data.name, linkName);
			if (cmpResult == 0)
			{
				leftIndex = current;
				break;
			} else if (cmpResult > 0)
			{
				rightIndex = current - 1;
			} else
			{
				leftIndex = current + 1;
			}
		}
	}
	uint16_t	newIndex = mRoot.freeHead;
	if (newIndex)
	{
		SPinsDataLink	freeLink;
		ReadLink(newIndex, &freeLink);
		mRoot.freeHead = freeLink.next;
		WriteRoot();
		mCount++;
	} else if (mMaxLinkCount > mCount)
	{
		mCount++;
		newIndex = mCount;
	}

	if (newIndex)
	{
		if (mCurrentIndex)
		{
			leftIndex--;
			/*
			*	If the new link is to be inserted after an existing link...
			*/
			if (leftIndex >= 0)
			{
				GoToRelativeLink(leftIndex - currLogIndex);
				inLink.prev = mCurrentIndex;
				inLink.next = mCurrent.next;
				mCurrent.next = newIndex;
				WriteLink(mCurrentIndex, &mCurrent);
				if (inLink.next != 0)
				{
					GoToLink(inLink.next);
					mCurrent.prev = newIndex;
					WriteLink(inLink.next, &mCurrent);
				} else
				{
					mRoot.tail = newIndex;
					WriteRoot();
				}
			/*
			*	Else, this is the new head.
			*	Load and update the root and the previous head link.
			*/
			} else
			{
				GoToLink(mRoot.head);
				inLink.prev = 0;
				inLink.next = mRoot.head;
				mRoot.head = newIndex;
				mCurrent.prev = newIndex;
				WriteLink(mCurrentIndex, &mCurrent);
				WriteRoot();
			}
		/*
		*	Else the list is empty.
		*	begin a new list.
		*/
		} else
		{
			inLink.prev = 0;
			inLink.next = 0;
			mRoot.head = newIndex;
			mRoot.tail = newIndex;
			WriteRoot();
		}
		WriteLink(newIndex, &inLink);
		GoToLink(newIndex);
	}
	return(newIndex);
}

/******************************* RemoveCurrent ********************************/
/*
*	Removes the current SPinsDataLink by adding it to the free linked list.
*	Returns true if the current SPinsDataLink was removed.
*	If there is a next SPinsDataLink, the next SPinsDataLink becomes the
*	current else the previous becomes the current, if one exists.
*	If no current then the current index is 0.
*/
bool PinConfigs::RemoveCurrent(void)
{
	bool	success = mCurrentIndex != 0;
	if (success)
	{
		// Save the current prev and next indexes
		uint16_t	prev = mCurrent.prev;
		uint16_t	next = mCurrent.next;
	
		// Add current to the free list as the new head.
		mCurrent.next = mRoot.freeHead;
		mCurrent.prev = 0;	// The free list is one way.
		WriteLink(mCurrentIndex, &mCurrent);
		mRoot.freeHead = mCurrentIndex;

		/*
		*	If the previous link isn't the root THEN
		*	update the previous link's next field.
		*/
		if (prev != 0)
		{
			ReadLink(prev, &mCurrent);
			mCurrent.next = next;
			WriteLink(prev, &mCurrent);
		/*
		*	Else, update the root's head.
		*/
		} else
		{
			mRoot.head = next;
		}
		/*
		*	If there is a next link THEN
		*	update the next link's prev field.
		*	The next SPinsDataLink becomes the new current link.
		*/
		if (next != 0)
		{
			ReadLink(next, &mCurrent);
			mCurrent.prev = prev;
			WriteLink(next, &mCurrent);
			mCurrentIndex = next;
		/*
		*	Else update the root tail to point to the prev link.
		*	The prev link becomes the new current link.
		*/
		} else
		{
			mRoot.tail = prev;
			mCurrentIndex = prev;
		}
		
		// Write the updated root.
		WriteRoot();
		mCount--;
	}
	return(success);
}

/******************************** IsValidIndex ********************************/
/*
*	Returns true if the passed physical link index is valid.  In order to
*	determine this, the link list needs to be walked till the index is found.
*	You can't simply check to see if it's less than the logical count because
*	physical indexes don't move when a link is removed, the index is only added
*	to the freeHead chain.
*/
bool PinConfigs::IsValidIndex(
	uint16_t	inRecIndex)
{
	bool isValid = false;
	if (inRecIndex > 0)
	{
		SPinsDataLink	link;
		uint16_t	next = mRoot.head;
		while (next)
		{
			if (next != inRecIndex)
			{
				ReadLink(next, &link);
				next = link.next;
				continue;
			}
			isValid = true;
			break;
		}
	}
	return(isValid);
}

/********************************** ReadLink **********************************/
void PinConfigs::ReadLink(
	uint16_t		inIndex,
	SPinsDataLink*	inLink) const
{
	// inIndex is always > 0.  0 is reserved.
	mDataStream->Seek((inIndex-1)*sizeof(SPinsDataLink), DataStream::eSeekSet);
	mDataStream->Read(sizeof(SPinsDataLink), inLink);
}

/********************************** WriteLink *********************************/
void PinConfigs::WriteLink(
	uint16_t				inIndex,
	const SPinsDataLink*	inLink) const
{
	// inIndex is always > 0.  0 is reserved.
	mDataStream->Seek((inIndex-1)*sizeof(SPinsDataLink), DataStream::eSeekSet);
	mDataStream->Write(sizeof(SPinsDataLink), inLink);
}

/********************************** ReadRoot **********************************/
void PinConfigs::ReadRoot(void)
{
	mDataStream->Seek(-(int32_t)sizeof(SPinConfigRoot), DataStream::eSeekEnd);
	mDataStream->Read(sizeof(SPinConfigRoot), &mRoot);
}

/********************************** WriteRoot *********************************/
void PinConfigs::WriteRoot(void)
{
	mDataStream->Seek(-(int32_t)sizeof(SPinConfigRoot), DataStream::eSeekEnd);
	mDataStream->Write(sizeof(SPinConfigRoot), &mRoot);
}

/****************************** SetCurrentConfig ******************************/
void PinConfigs::SetCurrentConfig(
	const uint8_t*	inConfig)
{
	if (mCurrentIndex)
	{
		mCurrent.data.SetConfig(inConfig);
		WriteLink(mCurrentIndex, &mCurrent);
	}
}

/******************************* UInt32ToHexStr *******************************/
char* PinConfigs::UInt32ToHexStr(
	uint32_t	inNum,
	char*		inBuffer)
{
	static const char kHexChars[] = "0123456789ABCDEF";
	uint8_t i = 8;
	while (i)
	{
		i--;
		inBuffer[i] = kHexChars[inNum & 0xF];
		inNum >>= 4;
	}
	inBuffer[8] = 0;
	return(inBuffer);
}

#if 0
void PinConfigs::DumpConfigs(void) const
{
	if (mCurrentIndex)
	{
		uint16_t	next = mRoot.head;
		SPinsDataLink	link;
		while (next)
		{
			ReadLink(next, &link);
			fprintf(stderr, "%s", link.data.name);
			uint32_t*	configPtr = (uint32_t*)link.data.config;
			char hexBuf[10];
			for (uint32_t i = 0; i < 5; i++, configPtr++)
			{
				UInt32ToHexStr(*configPtr, hexBuf);
				fprintf(stderr, ",%s", hexBuf);
			}
			fprintf(stderr, "\n");
			next = link.next;
		}
	}
}
#endif
#ifdef __MACH__
/******************************** SaveToArchive *******************************/
/*
*	outArchive should be the size returned by GetArchiveSize().
*/
void PinConfigs::SaveToArchive(
	void*	outArchive) const
{
	SPinsData*	archivePtr = (SPinsData*)outArchive;
	uint16_t	next = mRoot.head;
	SPinsDataLink	link;
	while (next)
	{
		ReadLink(next, &link);
		memcpy(archivePtr, &link.data, sizeof(SPinsData));
		next = link.next;
		archivePtr++;
	}
}

/****************************** LoadFromArchive *******************************/
void PinConfigs::LoadFromArchive(
	uint32_t		inArchiveSize,
	const void*		inArchive)
{
	uint32_t	linksToAdd = inArchiveSize/sizeof(SPinsData);
	const SPinsData*	archivePtr = (const SPinsData*)inArchive;
	uint8_t		config[40];
	FormatRoot();
	for (; linksToAdd; linksToAdd--)
	{
		archivePtr->GetConfig(config);
		AddConfig(archivePtr->name, config);
		archivePtr++;
	}
}
#endif

/********************************* LoadFromSD *********************************/
/*
*	This routine attemps to open a CSV file.  The original file should be
*	created using SaveToSD().  The csv contains six fields: Name, Pins1to8,
*	Pins9to16, Pins17to24, Pins25to32, Pins33to40. The five PinsXXtoXX fields
*	are hex values representing the settings for 40 pins, compressed as 4 bits
*	for each pin.
*	Each 4 bit value is a number from 0 to 2:
*										0 = unused
*										1 = used
*										2 = used + bridged
*/
bool PinConfigs::LoadFromSD(
	const char*	inPath,
	bool		inMerge)
{
	SdFile*	file;	// Treated as FILE* for MacOS
#ifndef __MACH__
	SdFile sdFile;
	bool	fileOpened = sdFile.open(inPath, O_RDONLY);
	file = &sdFile;
#else
	file = fopen(inPath, "r+");
	bool	fileOpened = file != nullptr;
#endif
	if (fileOpened)
	{
		SPinsDataLink	link;
		CSVUtils		csv(file);
		uint32_t*		configPtr;
	
		if (inMerge == false)
		{
			FormatRoot();
		}
		char thisChar = csv.SkipLine();	// Skip the csv header line.
		
		while (thisChar != 0)
		{
			memset(link.data.name, 0, sizeof(link.data.name));
			configPtr = (uint32_t*)link.data.config;
			if (((thisChar = csv.ReadStr(sizeof(link.data.name), link.data.name)) == ',') &&
				((thisChar = csv.ReadUint32AsHex(configPtr++)) == ',') &&
				((thisChar = csv.ReadUint32AsHex(configPtr++)) == ',') &&
				((thisChar = csv.ReadUint32AsHex(configPtr++)) == ',') &&
				((thisChar = csv.ReadUint32AsHex(configPtr++)) == ',') &&
				((thisChar = csv.ReadUint32AsHex(configPtr)) == '\n' || thisChar == 0))
			{
				if (GoToNamedLink(link.data.name))
				{
					memcpy(mCurrent.data.config, link.data.config, sizeof(link.data.config));
				} else
				{
					AddLink(link);
				}
			}
		}
	#ifndef __MACH__
		file->close();
	#else
		fclose(file);
	#endif
	}
	return(fileOpened);
}

/********************************** SaveToSD **********************************/
bool PinConfigs::SaveToSD(
	const char*	inPath)
{
	bool	success = mCount > 0;
	if (success)
	{
		SdFile*	file;	// Treated as FILE* for MacOS
	#ifndef __MACH__
		SdFile sdFile;
		SdFile::dateTimeCallback(UnixTime::SDFatDateTimeCB);
		success = sdFile.open(inPath, O_WRONLY | O_TRUNC | O_CREAT);
		file = &sdFile;
	#else
		file = fopen(inPath, "w");
		success = file != nullptr;
	#endif
		if (success)
		{
			CSVUtils	csv(file);
			
			csv.WriteStr("Name,Pins1to8,Pins9to16,Pins17to24,Pins25to32,Pins33to40\n");
			char	quotedStr[132];
			SPinsDataLink	link;
			uint16_t	next = mRoot.head;
			while (next)
			{
				ReadLink(next, &link);
				csv.WriteStr(csv.QuoteForCSV(link.data.name, quotedStr));
				uint32_t*	configPtr = (uint32_t*)link.data.config;
				char hexBuf[10];
				for (uint32_t i = 0; i < 5; i++, configPtr++)
				{
					csv.WriteChar(',');
					csv.WriteStr(UInt32ToHexStr(*configPtr, hexBuf));
				}
				csv.WriteChar('\n');
				next = link.next;
			}
		#ifndef __MACH__
			file->close();
		#else
			fclose(file);
		#endif
		}
	}
	return(success);
}

/********************************** SetName ***********************************/
void SPinsData::SetName(
	const char*	inName)
{
	strcpy(name, inName);
}

/********************************** SetConfig *********************************/
/*
*	Each byte of inConfig[40] currently has a range of 3 bits. To save space,
*	inConfig[40] is converted to config[20] by packing the lower nibble of two
*	pin configs to a single byte.
*/
void SPinsData::SetConfig(
	const uint8_t*	inConfig)
{
	for (uint32_t i = 0; i < 20; i++)
	{
		config[i] =  *(inConfig++);
		config[i] |= (*(inConfig++) << 4);		 
	}
}

/********************************** GetConfig *********************************/
void SPinsData::GetConfig(
	uint8_t*	inConfig) const
{
	for (uint32_t i = 0; i < 20; i++)
	{
		uint8_t	val = config[i];
		*(inConfig++) = val & 0xF;
		*(inConfig++) = val >> 4;
	}
}

