/*
*	FlashFS.cpp, Copyright Jonathan Mackey 2025
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
#include "FlashFS.h"
#include "DataStream.h"
#include "UnixTime.h"
#include <stdio.h>
#include <string.h>
#ifndef __MACH__
#include <Arduino.h>
#endif
#include "FileUtils.h"

const uint16_t SRootEntry::kRootFormatted = 1234;

/********************************** FlashFS ***********************************/
FlashFS::FlashFS(
	DataStream*	inEntries)
	: mEntries(inEntries), mFileCount(0), mEntryCount(0), mMaxEntryCount(0),
		mFreeEntryCount(0)
{
}

/********************************* Initialize *********************************/
void FlashFS::Initialize(
	DataStream*	inEntries,
	bool		inFormat)
{
#ifdef __MACH__
	{
		fprintf(stderr, "Initialize, inFormat = %s\n", inFormat ? "true":"false");
	}
#endif	
	mEntries = inEntries;
	mEntries->Seek(0, DataStream::eSeekEnd);
	mMaxEntryCount = mEntries->GetPos()/sizeof(SFileEntry);
	mDirtyIndex = 0xFFFF;
	if (mEntries &&
		mMaxEntryCount > 10)
	{
		ReadEntry(0, &mRoot);

		if (mRoot.formatted == SRootEntry::kRootFormatted &&
			!inFormat)
		{
			mFreeEntryCount = CountEntries(mRoot.freeHead);
			uint16_t	entryCount = 1;
			entryCount += mFreeEntryCount;
			entryCount += CountEntries(mRoot.freeSectors);
			{
				SFileEntry	fileEntry;
				uint16_t	fileCount = 0;
				fileEntry.next = mRoot.head;
				while (fileEntry.next)
				{
					fileCount++;
					ReadEntry(fileEntry.next, &fileEntry);
					entryCount += CountEntries(fileEntry.sectors);
					entryCount += 3;
				}
				mFileCount = fileCount;
			}
			mEntryCount = entryCount;
			mFreeEntryCount = mFreeEntryCount + mMaxEntryCount - mEntryCount;
		/*
		*	Else, format the root
		*/
		} else
		{							// {next, start, len, end}
			SSectorsEntry freeSectors = {0,0,2048,2048};
			mRoot.head = 0;
			mRoot.freeHead = 0;
			mRoot.freeSectors = 1;
			mRoot.formatted = SRootEntry::kRootFormatted;
			WriteEntry(0, &mRoot);
			WriteEntry(1, &freeSectors);
			mFileCount = 0;
			mEntryCount = 2;
			mFreeEntryCount = mMaxEntryCount - 2;
		}
	}
}

#ifdef __MACH__
static char tmpBuf[10];
const char* SpBr(
	uint16_t	inNum)
{
	snprintf(tmpBuf, 10, "[%hd]", inNum);
	return(tmpBuf);
}
/********************************** DumpRoot **********************************/
void FlashFS::DumpRoot(void) const
{
	SSectorsEntry	sectorsEntry;
	SFileEntry		fileEntry;
	SFileInfoEntry	fileInfoEntry;
	SFileDateTypeEntry	fileDateTypeEntry;
	uint16_t		entryIndex = mRoot.head;
	uint16_t		sectorsIndex;
	uint16_t		entryCount = 1;
	uint16_t		indexList[255];
	char			fourByteTag[5];
	char			creationDateStr[12];
	char			modifiedDateStr[12];
	char			creationTimeStr[10];
	char			modifiedTimeStr[10];
	fourByteTag[4] = 0;
	memset(indexList, 0, 255);
	indexList[0] = 1;
	fprintf(stderr, "DumpRoot()\n"
					"[0] SRootEntry %hd %hd %hd\n",
					mRoot.head, mRoot.freeHead, mRoot.freeSectors);
	fprintf(stderr, "{\n"
					"\tmRoot.head, mFileCount = %hd\n", mFileCount);
	if (entryIndex)
	{
		while (entryIndex)
		{
			indexList[entryIndex]++;
			if (entryCount < entryIndex)
			{
				entryCount = entryIndex;
			}
			ReadEntry(entryIndex, &fileEntry);
			ReadEntry(fileEntry.info, &fileInfoEntry);
			ReadEntry(fileEntry.dateType, &fileDateTypeEntry);
			if (fileEntry.info)
			{
				indexList[fileEntry.info]++;
				if (entryCount < fileEntry.info)
				{
					entryCount = fileEntry.info;
				}
			}
			if (fileEntry.dateType)
			{
				indexList[fileEntry.dateType]++;
				if (entryCount < fileEntry.dateType)
				{
					entryCount = fileEntry.dateType;
				}
			}
			*(uint32_t*)fourByteTag = Swap32(fileInfoEntry.tag);
			fprintf(stderr, "\t[%hd] SFileEntry %hd %hd\n",
				entryIndex, fileEntry.next, fileEntry.sectors);
			fprintf(stderr, "\t\t[%hd] SFileInfoEntry \'%s\', %d\n",
									fileEntry.info, fourByteTag, fileInfoEntry.length);
			UnixTime::CreateTimeStr(fileDateTypeEntry.creation, creationTimeStr);
			UnixTime::CreateDateStr(fileDateTypeEntry.creation, creationDateStr);
			if (fileDateTypeEntry.type)
			{
				*(uint32_t*)fourByteTag = Swap32(fileDateTypeEntry.type);
			} else
			{
				fourByteTag[0] = '?';
				fourByteTag[1] = 0;
			}
			fprintf(stderr, "\t\t[%hd] SFileDateTypeEntry\n\t\t\t%s %s\n\t\t\t\'%s\'\n",
									fileEntry.dateType, creationDateStr, creationTimeStr,
									fourByteTag);
			entryIndex = fileEntry.next;
			sectorsIndex = fileEntry.sectors;
			fprintf(stderr, "\t{\n");
			while (sectorsIndex)
			{
				indexList[sectorsIndex]++;
				if (entryCount < sectorsIndex)
				{
					entryCount = sectorsIndex;
				}
				ReadEntry(sectorsIndex, &sectorsEntry);
				fprintf(stderr, "\t\t%5s SSectorsEntry %3hd %3hd:%hd R%hd\n",
					SpBr(sectorsIndex), sectorsEntry.next, sectorsEntry.start, sectorsEntry.end, sectorsEntry.len);
				sectorsIndex = sectorsEntry.next;
			}
			fprintf(stderr, "\t}\n");
		}
	} else
	{
		fprintf(stderr, "\tnone\n");
	}
	fprintf(stderr, "}\n");
	
	fprintf(stderr, "{\n"
					"\tmRoot.freeHead\n\t");
	entryIndex = mRoot.freeHead;
	{
		uint32_t	freeEntryCount = 0;
		if (entryIndex)
		{
			while (entryIndex)
			{
				indexList[entryIndex]++;
				if (entryCount < entryIndex)
				{
					entryCount = entryIndex;
				}
				ReadEntry(entryIndex, &fileEntry);
				fprintf(stderr, "%4hd", entryIndex);
				entryIndex = fileEntry.next;
				freeEntryCount++;
				if ((freeEntryCount % 16) == 0)
				{
					fprintf(stderr, "\n\t");
				}
			}
		} else
		{
			fprintf(stderr, "none");
		}
	}
	fprintf(stderr, "\n"
					"}\n");

	sectorsIndex = mRoot.freeSectors;
	fprintf(stderr, "{\n"
					"\tmRoot.freeSectors\n");
	while (sectorsIndex)
	{
		indexList[sectorsIndex]++;
		if (entryCount < sectorsIndex)
		{
			entryCount = sectorsIndex;
		}
		ReadEntry(sectorsIndex, &sectorsEntry);
		fprintf(stderr, "\t%5s SSectorsEntry %3hd %3hd:%hd R%hd\n",
			SpBr(sectorsIndex), sectorsEntry.next, sectorsEntry.start, sectorsEntry.end, sectorsEntry.len);
		sectorsIndex = sectorsEntry.next;
	}
	fprintf(stderr, "}\n\n");
	
	fprintf(stderr, "mDirtyIndex = %hd\n", mDirtyIndex);
	fprintf(stderr, "mEntryCount = %hd\n", mEntryCount);
	fprintf(stderr, "Actual entry count = %hd\n", entryCount+1);
	fprintf(stderr, "mFreeEntryCount = %hd\n", mFreeEntryCount);
	fprintf(stderr, "mMaxEntryCount = %hd\nIndex errors:\n", mMaxEntryCount);
	uint16_t	errorCount = 0;
	for (entryIndex = 0; entryIndex < entryCount; entryIndex++)
	{
		if (indexList[entryIndex] != 1)
		{
			errorCount++;
			fprintf(stderr, "\t[%hd] = %hd\n", entryIndex, indexList[entryIndex]);
		}
	}
	if (errorCount == 0)
	{
		fprintf(stderr, "\tnone\n");
	}
	fprintf(stderr, "\n-----------\n\n");
}
#else
/********************************** DumpRoot **********************************/
void FlashFS::DumpRoot(void) const
{
	SSectorsEntry	sectorsEntry;
	SFileEntry		fileEntry;
	SFileInfoEntry	fileInfoEntry;
	SFileDateTypeEntry	fileDateTypeEntry;
	uint16_t		entryIndex = mRoot.head;
	uint16_t		sectorsIndex;
	uint16_t		entryCount = 1;
	uint16_t		indexList[255];
	char			fourByteTag[5];
	char			creationDateStr[12];
	char			modifiedDateStr[12];
	char			creationTimeStr[10];
	char			modifiedTimeStr[10];
	fourByteTag[4] = 0;
	memset(indexList, 0, 255);
	indexList[0] = 1;
	Serial.printf("DumpRoot()\n"
					"[0] SRootEntry %d %d %d\n",
					(int)mRoot.head, (int)mRoot.freeHead, (int)mRoot.freeSectors);
	Serial.printf("{\n"
					"\tmRoot.head, mFileCount = %d\n", (int)mFileCount);
	if (entryIndex)
	{
		while (entryIndex)
		{
			indexList[entryIndex]++;
			if (entryCount < entryIndex)
			{
				entryCount = entryIndex;
			}
			ReadEntry(entryIndex, &fileEntry);
			ReadEntry(fileEntry.info, &fileInfoEntry);
			ReadEntry(fileEntry.dateType, &fileDateTypeEntry);
			if (fileEntry.info)
			{
				indexList[fileEntry.info]++;
				if (entryCount < fileEntry.info)
				{
					entryCount = fileEntry.info;
				}
			}
			if (fileEntry.dateType)
			{
				indexList[fileEntry.dateType]++;
				if (entryCount < fileEntry.dateType)
				{
					entryCount = fileEntry.dateType;
				}
			}
			*(uint32_t*)fourByteTag = Swap32(fileInfoEntry.tag);
			Serial.printf("\t[%d] SFileEntry %d %d\n",
				(int)entryIndex, (int)fileEntry.next, (int)fileEntry.sectors);
			Serial.printf("\t\t[%d] SFileInfoEntry \'%s\', %d\n",
									(int)fileEntry.info, fourByteTag, fileInfoEntry.length);
			UnixTime::CreateTimeStr(fileDateTypeEntry.creation, creationTimeStr);
			UnixTime::CreateDateStr(fileDateTypeEntry.creation, creationDateStr);
			if (fileDateTypeEntry.type)
			{
				*(uint32_t*)fourByteTag = Swap32(fileDateTypeEntry.type);
			} else
			{
				fourByteTag[0] = '?';
				fourByteTag[1] = 0;
			}
			Serial.printf("\t\t[%d] SFileDateTypeEntry\n\t\t\t%s %s\n\t\t\t\'%s\'\n",
									(int)fileEntry.dateType, creationDateStr, creationTimeStr,
									fourByteTag);
			entryIndex = fileEntry.next;
			sectorsIndex = fileEntry.sectors;
			Serial.printf("\t{\n");
			while (sectorsIndex)
			{
				indexList[sectorsIndex]++;
				if (entryCount < sectorsIndex)
				{
					entryCount = sectorsIndex;
				}
				ReadEntry(sectorsIndex, &sectorsEntry);
				Serial.printf("\t\t[%d] SSectorsEntry %3d %3d:%d R%d\n",
					(int)sectorsIndex, (int)sectorsEntry.next, (int)sectorsEntry.start, (int)sectorsEntry.end, (int)sectorsEntry.len);
				sectorsIndex = sectorsEntry.next;
			}
			Serial.printf("\t}\n");
		}
	} else
	{
		Serial.printf("\tnone\n");
	}
	Serial.printf("}\n");
	
	Serial.printf("{\n"
					"\tmRoot.freeHead\n\t");
	entryIndex = mRoot.freeHead;
	{
		uint32_t	freeEntryCount = 0;
		if (entryIndex)
		{
			while (entryIndex)
			{
				indexList[entryIndex]++;
				if (entryCount < entryIndex)
				{
					entryCount = entryIndex;
				}
				ReadEntry(entryIndex, &fileEntry);
				Serial.printf("%4d", (int)entryIndex);
				entryIndex = fileEntry.next;
				freeEntryCount++;
				if ((freeEntryCount % 16) == 0)
				{
					Serial.printf("\n\t");
				}
			}
		} else
		{
			Serial.printf("none");
		}
	}
	Serial.printf("\n"
					"}\n");

	sectorsIndex = mRoot.freeSectors;
	Serial.printf("{\n"
					"\tmRoot.freeSectors\n");
	while (sectorsIndex)
	{
		indexList[sectorsIndex]++;
		if (entryCount < sectorsIndex)
		{
			entryCount = sectorsIndex;
		}
		ReadEntry(sectorsIndex, &sectorsEntry);
		Serial.printf("\t[%d] SSectorsEntry %3d %3d:%d R%d\n",
			(int)sectorsIndex, (int)sectorsEntry.next, (int)sectorsEntry.start, (int)sectorsEntry.end, (int)sectorsEntry.len);
		sectorsIndex = sectorsEntry.next;
	}
	Serial.printf("}\n\n");
	
	Serial.printf("mDirtyIndex = %d\n", (int)mDirtyIndex);
	Serial.printf("mEntryCount = %d\n", (int)mEntryCount);
	Serial.printf("Actual entry count = %d\n", (int)entryCount+1);
	Serial.printf("mFreeEntryCount = %d\n", (int)mFreeEntryCount);
	Serial.printf("mMaxEntryCount = %d\nIndex errors:\n", (int)mMaxEntryCount);
	uint16_t	errorCount = 0;
	for (entryIndex = 0; entryIndex < entryCount; entryIndex++)
	{
		if (indexList[entryIndex] != 1)
		{
			errorCount++;
			Serial.printf("\t[%d] = %d\n", (int)entryIndex, (int)indexList[entryIndex]);
		}
	}
	if (errorCount == 0)
	{
		Serial.printf("\tnone\n");
	}
	Serial.printf("\n-----------\n\n");
}
#endif

/******************************** CountEntries ********************************/
/*
*	Returns the number of SFileEntry or SSectorsEntry in a linked list.
*/
uint16_t FlashFS::CountEntries(
	uint16_t	inHeadIndex) const
{
	uint16_t	count = 0;
	SFileEntry	entry;
	entry.next = inHeadIndex;
	while(entry.next)
	{
		count++;
		ReadEntry(entry.next, &entry);
	}
	return(count);
}

/******************************** AddFileEntry ********************************/
/*
*	If an entry with this tag already exists, the existing entry will
*	be removed before the add takes place.
*/
uint16_t FlashFS::AddFileEntry(
	uint32_t	inTag,
	uint32_t	inType,
	uint16_t	inNumSectorsReq)
{
#ifdef __MACH__
	{
		uint32_t	tag = inTag;
		char		fourByteTag[5];
		char		typeStr[5];
		fourByteTag[4] = 0;
		*(uint32_t*)fourByteTag = Swap32(tag);
		if (inType)
		{
			typeStr[4] = 0;
			*(uint32_t*)typeStr = Swap32(inType);
		} else
		{
			typeStr[0] = '?';
			typeStr[1] = 0;
		}
		fprintf(stderr, "AddFileEntry \'%s\' \'%s\' %hd\n", fourByteTag, typeStr, inNumSectorsReq);
	}
#endif	
	uint16_t	entryIndex = 0;
	RemoveFileEntry(inTag);
	
	if (mFreeEntryCount > 2)
	{
		entryIndex = AllocateEntry();
		if (entryIndex)
		{
			mFileCount++;
			SFileEntry	fileEntry;
			SFileInfoEntry	infoEntry;
			SFileDateTypeEntry	dateTypeEntry;
			fileEntry.next = mRoot.head;
			fileEntry.sectors = 0;
			fileEntry.info = AllocateEntry();
			fileEntry.dateType = AllocateEntry();
			infoEntry.tag = inTag;
			infoEntry.length = 0;
			dateTypeEntry.creation = UnixTime::Time();
			dateTypeEntry.type = inType;
			WriteEntry(entryIndex, &fileEntry);
			WriteEntry(fileEntry.info, &infoEntry);
			WriteEntry(fileEntry.dateType, &dateTypeEntry);
			mRoot.head = entryIndex;
			WriteEntry(0, &mRoot);
			
			// Now allocate the sectors
			fileEntry.sectors = AllocateSectors(inNumSectorsReq);
			if (fileEntry.sectors ||
				inNumSectorsReq == 0)
			{
				WriteEntry(entryIndex, &fileEntry);
			} else
			{
				// Failed to allocate the sectors.
				// Remove the file entry
				RemoveByIndex(entryIndex);
				entryIndex = 0;
			}
		}
	}
	return(entryIndex);
}

/********************************* AddSectors *********************************/
bool FlashFS::AddSectors(
	uint32_t	inTag,
	uint16_t	inNumSectorsReq)
{
#ifdef __MACH__
	bool	success = false;
	uint16_t	entryIndex = GetFileEntryIndex(inTag);
	if (entryIndex)
	{
		uint32_t	tag = inTag;
		char	fourByteTag[5];
		fourByteTag[4] = 0;
		*(uint32_t*)fourByteTag = Swap32(tag);
		fprintf(stderr, "AdjustSectors \'%s\' by %hd\n", fourByteTag, inNumSectorsReq);
		success = AddSectorsByIndex(entryIndex, inNumSectorsReq);
	}
	return(success);
#else
	return(AddSectorsByIndex(GetFileEntryIndex(inTag), inNumSectorsReq));
#endif
}

/****************************** AddSectorsByIndex *****************************/
/*
*	This adds inNumSectorsReq additional sectors to the file entry inFileEntryIndex.
*/
bool FlashFS::AddSectorsByIndex(
	uint16_t	inFileEntryIndex,
	uint16_t	inNumSectorsReq)
{
	bool	success = false;
	if (inFileEntryIndex)
	{
		if (inNumSectorsReq)
		{
			uint16_t	addedSectorsIndex = AllocateSectors(inNumSectorsReq);
			/*
			*	If the requested sectors were allocated...
			*/
			if (addedSectorsIndex)
			{
				SFileEntry	fileEntry;
				ReadEntry(inFileEntryIndex, &fileEntry);
				success = true;
				
				/*
				*	If this file already owns sectors
				*/
				if (fileEntry.sectors)
				{
					uint16_t		sectorsEntryIndex = fileEntry.sectors;
					SSectorsEntry	sectorsEntry;
					SSectorsEntry	addedSectorsEntry;
					/*
					*	Find the last sector entry in this file's sectors list.
					*/
					while (sectorsEntryIndex)
					{
						ReadEntry(sectorsEntryIndex, &sectorsEntry);
						/*
						*	If not at the last entry THEN
						*	continue looking.
						*/
						if (sectorsEntry.next)
						{
							sectorsEntryIndex = sectorsEntry.next;
							continue;
						}
						ReadEntry(addedSectorsIndex, &addedSectorsEntry);
						/*
						*	If the added sector entry starts at the end of the
						*	last sector entry THEN
						*	have the last sector entry adopt the sectors of the
						*	the added sector entry.
						*/
						if (sectorsEntry.end == addedSectorsEntry.start)
						{
							sectorsEntry.end += addedSectorsEntry.len;	// Adopt the first entry's sectors
							sectorsEntry.len += addedSectorsEntry.len;	//
							sectorsEntry.next = addedSectorsEntry.next;	// Link in any additional entries
							FreeEntry(addedSectorsIndex);				// Free the first entry
						} else
						{
							sectorsEntry.next = addedSectorsIndex;	// Append the sectors entry(s)
						}
						break;
					}
					WriteEntry(sectorsEntryIndex, &sectorsEntry);
				/*
				*	Else this file doesn't own any sectors yet, so simply
				*	adopt them as the sectors list.
				*/
				} else
				{
					fileEntry.sectors = addedSectorsIndex;
					WriteEntry(inFileEntryIndex, &fileEntry);
				}
			}
		} else
		{
			success = true;
		}
	}
	return(success);
}

/******************************** AllocateEntry *******************************/
uint16_t FlashFS::AllocateEntry(void)
{
	uint16_t	entryIndex = 0;
	if (mFreeEntryCount > 0)
	{
		entryIndex = mRoot.freeHead;
		if (entryIndex)
		{
			SFileEntry	fileEntry;
			ReadEntry(entryIndex, &fileEntry);
			mRoot.freeHead = fileEntry.next;
			WriteEntry(0, &mRoot);
		} else if (mEntryCount < mMaxEntryCount)
		{
			entryIndex = mEntryCount;
			mEntryCount++;
		}
		mFreeEntryCount--;
	}
	return(entryIndex);
}

/********************************** FreeEntry *********************************/
void FlashFS::FreeEntry(
	uint16_t	inIndex)
{
	SFileEntry	fileEntry;
	fileEntry.next = mRoot.freeHead;
	mRoot.freeHead = inIndex;
	WriteEntry(inIndex, &fileEntry);
	WriteEntry(0, &mRoot);
	mFreeEntryCount++;
}

/****************************** FileEntryExists *******************************/
bool FlashFS::FileEntryExists(
	uint32_t	inTag) const
{
	return(GetFileEntryIndex(inTag) != 0);
}

/********************************** ChangeTag *********************************/
bool FlashFS::ChangeTag(
	uint32_t	inFromTag,
	uint32_t	inToTag)
{
	bool	success = false;
	if (FileEntryExists(inToTag) == false)
	{
		uint16_t		entryIndex = mRoot.head;
		SFileEntry		fileEntry;
		SFileInfoEntry	infoEntry;
		while (entryIndex)
		{
			ReadEntry(entryIndex, &fileEntry);
			ReadEntry(fileEntry.info, &infoEntry);
			if (infoEntry.tag != inFromTag)
			{
				entryIndex = fileEntry.next;
				continue;
			}
			success = true;
			infoEntry.tag = inToTag;
			WriteEntry(fileEntry.info, &infoEntry);
			break;
		}
	}
	return(success);
}

/****************************** GetFileEntryIndex *****************************/
/*
*	Returns 0 if inTag not found.
*/
uint16_t FlashFS::GetFileEntryIndex(
	uint32_t	inTag) const
{
	uint16_t		entryIndex = mRoot.head;
	SFileEntry		fileEntry;
	SFileInfoEntry	infoEntry;
	while (entryIndex)
	{
		ReadEntry(entryIndex, &fileEntry);
		ReadEntry(fileEntry.info, &infoEntry);
		if (infoEntry.tag != inTag)
		{
			entryIndex = fileEntry.next;
			continue;
		}
		break;
	}
	return(entryIndex);
}

/****************************** GetNthFileEntry *******************************/
bool FlashFS::GetNthFileEntry(
	uint16_t		inIndex,
	const uint32_t*	inValidTypeList,
	SFileEntry&		outFileEntry) const
{
	uint16_t		entryIndex = mRoot.head;
	for (uint16_t i = 0; entryIndex; i++)
	{
		ReadEntry(entryIndex, &outFileEntry);
		if (inValidTypeList)
		{
			SFileDateTypeEntry	dateTypeEntry;
			ReadEntry(outFileEntry.dateType, &dateTypeEntry);

			if (FileUtils::IsValidType(dateTypeEntry.type, inValidTypeList) == false)
			{
				inIndex++;	// Ignore this file in terms of the index count.
			}
		}
		if (i != inIndex)
		{
			entryIndex = outFileEntry.next;
			continue;
		}
		return(true);
	}
	return(false);
}

/****************************** GetNextFileEntry ******************************/
bool FlashFS::GetNextFileEntry(
	SFileEntry&	ioFileEntry) const
{
	if (ioFileEntry.next)
	{
		ReadEntry(ioFileEntry.next, &ioFileEntry);
		return(true);
	}
	return(false);
}

/****************************** GetFileInfoEntry ******************************/
void FlashFS::GetFileInfoEntry(
	const SFileEntry&	inFileEntry,
	SFileInfoEntry&		outFileInfoEntry) const
{
	ReadEntry(inFileEntry.info, &outFileInfoEntry);
}

/**************************** GetFileDateTypeEntry ****************************/
void FlashFS::GetFileDateTypeEntry(
	const SFileEntry&		inFileEntry,
	SFileDateTypeEntry&		outFileDateTypeEntry) const
{
	ReadEntry(inFileEntry.dateType, &outFileDateTypeEntry);
}

/******************************** GetFileType *********************************/
/*
*	Gets the file type without having to open the file with a FlashFSFile object.
*/
uint32_t FlashFS::GetFileType(
	uint32_t	inTag) const
{
	uint32_t	fileType = 0;
	if (inTag)
	{
		uint16_t	fileIndex = GetFileEntryIndex(inTag);
		if (fileIndex)
		{
			SFileEntry	fileEntry;
			ReadEntry(fileIndex, &fileEntry);
			SFileDateTypeEntry	dateTypeEntry;
			ReadEntry(fileEntry.dateType, &dateTypeEntry);
			fileType = dateTypeEntry.type;
		}
	}
	return(fileType);
}

/****************************** RemoveFileEntry *******************************/
bool FlashFS::RemoveFileEntry(
	uint32_t	inTag)
{
	bool	success = false;
	uint16_t	entryIndex = GetFileEntryIndex(inTag);
	if (entryIndex)
	{
		success = true;
		RemoveByIndex(entryIndex);
#ifdef __MACH__
		uint32_t	tag = inTag;
		char	fourByteTag[5];
		fourByteTag[4] = 0;
		*(uint32_t*)fourByteTag = Swap32(tag);
		fprintf(stderr, "RemoveFileEntry \'%s\'\n", fourByteTag);
#endif
	}
	return(success);
}

/******************************** RemoveByIndex *******************************/
void FlashFS::RemoveByIndex(
	uint16_t	inIndex)
{
	if (inIndex)
	{
		SFileEntry	fileEntry;
		ReadEntry(inIndex, &fileEntry);
		if (mRoot.head == inIndex)
		{
			mRoot.head = fileEntry.next;
		/*
		*	Else, fileEntry is not the root head.  Find the file entry that
		*	is before fileEntry to remove fileEntry from the list.
		*/
		} else
		{
			SFileEntry	prevFileEntry;
			uint16_t	fileEntryIndex = mRoot.head;
			while (fileEntryIndex)
			{
				ReadEntry(fileEntryIndex, &prevFileEntry);
				if (prevFileEntry.next != inIndex)
				{
					fileEntryIndex = prevFileEntry.next;
					continue;
				}
				break;
			}
			prevFileEntry.next = fileEntry.next;
			WriteEntry(fileEntryIndex, &prevFileEntry);
		}
		mFileCount--;
		FreeEntry(fileEntry.info);
		FreeEntry(fileEntry.dateType);
		FreeEntry(inIndex);
		
		if (fileEntry.sectors)
		{
			SSectorsEntry	sectorsEntry;
			uint16_t		sectorsEntryIndex = fileEntry.sectors;
			uint16_t		nextSectorsEntryIndex;
			
			while (sectorsEntryIndex)
			{
				ReadEntry(sectorsEntryIndex, &sectorsEntry);
				nextSectorsEntryIndex = sectorsEntry.next;
				FreeSectors(sectorsEntryIndex);
				sectorsEntryIndex = nextSectorsEntryIndex;
			}
		}
	}
}

/****************************** CountFreeSectors ******************************/
uint16_t FlashFS::CountFreeSectors(void) const
{
	return(CountSectors(mRoot.freeSectors));
}

/******************************** CountSectors ********************************/
uint16_t FlashFS::CountSectors(
	uint16_t	inSectorsIndex) const
{
	uint16_t	count = 0;
	if (inSectorsIndex)
	{
		SSectorsEntry	sectorsEntry;
		sectorsEntry.next = inSectorsIndex;
		while (sectorsEntry.next)
		{
			ReadEntry(sectorsEntry.next, &sectorsEntry);
			count += sectorsEntry.len;
		}
	}

	return(count);
}

/****************************** AllocateSectors *******************************/
/*
*	Returns the SSectorsEntry index.  This may be the head of a linked list of
*	SSectorsEntry if the sectors are fragmented.
*	Returns 0 if the allocation fails due to a lack of sectors or entry space.
*/
uint16_t FlashFS::AllocateSectors(
	uint16_t	inNumSectorsReq)
{
	uint16_t	sectorsEntryIndex = 0;
	
	/*
	*	If there is an allocation request AND
	*	there are enough root freeSectors to satisfy the request AND
	*	there is at least one free entry in case one needs to be allocated THEN
	*	allow the allocation to proceed.
	*/
	if (inNumSectorsReq &&
		CountFreeSectors() >= inNumSectorsReq &&
		mFreeEntryCount > 0)
	{
		bool			rootIsDirty = false;
		SSectorsEntry	freeSectors;
		uint16_t		freeSectorsIndex = mRoot.freeSectors;
		
		/*
		*	Loop till the allocation request is satisfied
		*/
		while (inNumSectorsReq != 0)
		{
			ReadEntry(freeSectorsIndex, &freeSectors);
			/*
			*	If this freeSectors entry will partially or completely satisfy
			*	the sectors request AND
			*	this isn't the protected root freeSectors tail THEN
			*	adopt this freeSectors entry.
			*/
			if (freeSectors.len <= inNumSectorsReq &&
				freeSectorsIndex > 1)
			{
				mRoot.freeSectors = freeSectors.next;	// Remove entry from freeSectors
				freeSectors.next = sectorsEntryIndex;	// Add to adopted list (if any)
				sectorsEntryIndex = freeSectorsIndex;	
				freeSectorsIndex = mRoot.freeSectors;	// setup for next loop
				inNumSectorsReq -= freeSectors.len;		// Decrement the outstanding request
				WriteEntry(sectorsEntryIndex, &freeSectors);
				rootIsDirty = true;
			/*
			*	Else allocate the remaining sectors from the current
			*	free sectors entry.
			*/
			} else
			{
				SSectorsEntry	sectorsEntry;
				uint16_t		nextEntryIndex = sectorsEntryIndex;
				/*
				*	Allocate a SSectorsEntry (guaranteed to exist)
				*/
				sectorsEntryIndex = AllocateEntry();
				sectorsEntry.next = nextEntryIndex;
				sectorsEntry.start = freeSectors.start;
				sectorsEntry.len = inNumSectorsReq;
				sectorsEntry.end = freeSectors.start+inNumSectorsReq;
				freeSectors.start += inNumSectorsReq;
				freeSectors.len -= inNumSectorsReq;
				WriteEntry(freeSectorsIndex, &freeSectors);
				WriteEntry(sectorsEntryIndex, &sectorsEntry);
				if (rootIsDirty)
				{
					WriteEntry(0, &mRoot);
				}
				break;
			}
		}
	}
	return(sectorsEntryIndex);
}

/******************************** FreeSectors *********************************/
void FlashFS::FreeSectors(
	uint16_t	inIndex)
{
	if (inIndex)
	{
		SSectorsEntry	freeSectorsEntry;
		SSectorsEntry	sectorsEntry;
		uint16_t		freeSectorsEntryIndex = mRoot.freeSectors;
		bool			freeSectorsChanged = false;
		uint16_t		prevSectorsEntryIndex;
		uint16_t		nextSectorsEntryIndex;
		uint16_t		sectorsEntryIndex;
		
		ReadEntry(inIndex, &sectorsEntry);
				
		/*
		*	See if the sectors of the SSectorsEntry being freed can be adopted
		*	by an existing freeSectors entry.
		*/
		while (freeSectorsEntryIndex)
		{
			ReadEntry(freeSectorsEntryIndex, &freeSectorsEntry);
			if (freeSectorsEntry.start == sectorsEntry.end)
			{
				freeSectorsEntry.start = sectorsEntry.start;
				freeSectorsEntry.len += sectorsEntry.len;
				break;
			} else if (freeSectorsEntry.end == sectorsEntry.start)
			{
				freeSectorsEntry.len += sectorsEntry.len;
				freeSectorsEntry.end += sectorsEntry.len;
				break;
			}
			freeSectorsEntryIndex = freeSectorsEntry.next;
		}
		
		if (freeSectorsEntryIndex)
		{
			freeSectorsChanged = true;
			WriteEntry(freeSectorsEntryIndex, &freeSectorsEntry);
			FreeEntry(inIndex);
		} else
		{
			sectorsEntry.next = mRoot.freeSectors;
			mRoot.freeSectors = inIndex;
			WriteEntry(0, &mRoot);
			WriteEntry(inIndex, &sectorsEntry);
		}
		
		while (freeSectorsChanged)
		{
			freeSectorsChanged = false;
			sectorsEntryIndex = mRoot.freeSectors;
			prevSectorsEntryIndex = 0;
			while (sectorsEntryIndex > 1)
			{
				ReadEntry(sectorsEntryIndex, &sectorsEntry);
				freeSectorsEntryIndex = nextSectorsEntryIndex = sectorsEntry.next;
				while (freeSectorsEntryIndex)
				{
					ReadEntry(freeSectorsEntryIndex, &freeSectorsEntry);
					if (freeSectorsEntry.start == sectorsEntry.end)
					{
						freeSectorsEntry.start = sectorsEntry.start;
						freeSectorsEntry.len += sectorsEntry.len;
						break;
					} else if (freeSectorsEntry.end == sectorsEntry.start)
					{
						freeSectorsEntry.len += sectorsEntry.len;
						freeSectorsEntry.end += sectorsEntry.len;
						break;
					}
					freeSectorsEntryIndex = freeSectorsEntry.next;
				}
				
				if (freeSectorsEntryIndex)
				{
					freeSectorsChanged = true;
					WriteEntry(freeSectorsEntryIndex, &freeSectorsEntry);
					/*
					*	if this sectorsEntry is not the head THEN
					*	remove this sectorsEntry in the list.
					*/
					if (prevSectorsEntryIndex)
					{
						SSectorsEntry	prevSectorsEntry;
						ReadEntry(prevSectorsEntryIndex, &prevSectorsEntry);
						prevSectorsEntry.next = sectorsEntry.next;
						WriteEntry(prevSectorsEntryIndex, &prevSectorsEntry);
					/*
					*	Else this sectorsEntry is the root freeSectors head.
					*	Make the next sectors entry the new freeSectors head.
					*/
					} else
					{
						mRoot.freeSectors = nextSectorsEntryIndex;
					}
					/*
					*	Move this sectorsEntry to the root freeHead.
					*/
					FreeEntry(sectorsEntryIndex);
				} else
				{
					prevSectorsEntryIndex = sectorsEntryIndex;
				}					
				sectorsEntryIndex = nextSectorsEntryIndex;
			}
		}
	}
}

/********************************** LoadFrom **********************************/
/*
*	Loads the mEntries data stream from inDataStream.  The first 32 bits of
*	inDataStream contains the number of entries to copy to mEntries.
*/
void FlashFS::LoadFrom(
	DataStream*	inDataStream)
{
	if (mEntries &&
		inDataStream &&
		inDataStream != mEntries)
	{
		uint8_t	buffer[1024];
		uint32_t	entryCount;
		inDataStream->Seek(0, DataStream::eSeekSet);
		inDataStream->Read(sizeof(uint32_t), &entryCount);
		/*
		*	If the entryCount is valid
		*/
		if (entryCount != 0 &&
			entryCount < 512)
		{
			uint32_t	bytesToCopy = entryCount*sizeof(SFileEntry);
			mEntries->Seek(0, DataStream::eSeekSet);
			while (bytesToCopy)
			{
				uint32_t	bytesCopied = (bytesToCopy > 1024) ? 1024 : bytesToCopy;
				inDataStream->Read(bytesCopied, buffer);
				mEntries->Write(bytesCopied, buffer);
				bytesToCopy -= bytesCopied;
			}
		}
		Initialize(mEntries);
	}
}

/*********************************** SaveTo ***********************************/
/*
*	Copies the dirty entries to inDataStream.  The frist 32 bits written to 
*	inDataStream contains the total number of entries in mEntries.  This value
*	is used by LoadFrom so that only the mEntryCount entries need to be copied
*	on startup.
*
*	The mDirtyIndex is reset after saving.
*/
void FlashFS::SaveTo(
	DataStream*	inDataStream)
{
#ifdef __MACH__
	{
		fprintf(stderr, "Save, mDirtyIndex = %hd\n", mDirtyIndex);
	}
#endif	
	if (mEntries &&
		inDataStream &&
		inDataStream != mEntries &&
		mDirtyIndex != 0xFFFF)
	{
		uint8_t	buffer[1024];
		uint32_t	entryCount = mEntryCount;
		uint32_t	bytesToCopy = sizeof(SFileEntry) + (mDirtyIndex*sizeof(SFileEntry));
		mEntries->Seek(0, DataStream::eSeekSet);
		inDataStream->Seek(0, DataStream::eSeekSet);
		inDataStream->Write(sizeof(uint32_t), &entryCount);
		while (bytesToCopy)
		{
			uint32_t	bytesCopied = (bytesToCopy > 1024) ? 1024 : bytesToCopy;
			mEntries->Read(bytesCopied, buffer);
			inDataStream->Write(bytesCopied, buffer);
			bytesToCopy -= bytesCopied;
		}
		mDirtyIndex = 0xFFFF;
	}
}

/********************************** ReadEntry *********************************/
/*
*	Reads one of SFileEntry, SSectorsEntry, or SRootEntry
*/
void FlashFS::ReadEntry(
	uint16_t		inIndex,
	void*			inEntry) const
{
	mEntries->Seek(inIndex*sizeof(SFileEntry), DataStream::eSeekSet);
	mEntries->Read(sizeof(SFileEntry), inEntry);
}

/********************************* WriteEntry *********************************/
/*
*	Writes one of SFileEntry, SSectorsEntry, or SRootEntry
*/
void FlashFS::WriteEntry(
	uint16_t		inIndex,
	const void*		inEntry) const
{
	mEntries->Seek(inIndex*sizeof(SFileEntry), DataStream::eSeekSet);
	mEntries->Write(sizeof(SFileEntry), inEntry);
	if (mDirtyIndex < inIndex ||
		mDirtyIndex == 0xFFFF)
	{
		mDirtyIndex = inIndex;
	}
}


