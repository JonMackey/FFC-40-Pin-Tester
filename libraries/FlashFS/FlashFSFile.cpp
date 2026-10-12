/*
*	FlashFSFile.cpp, Copyright Jonathan Mackey 2025
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

#include <stdio.h>
#include "FlashFSFile.h"
#include "DataStream.h"
#include "FlashMem.h"
#include "FileUtils.h"
#ifndef __MACH__
#include "UnixTime.h"
#include "SdFat.h"
#endif

/******************************** FlashFSFile *********************************/
FlashFSFile::FlashFSFile(
	FlashFS&	inFlashFS,
	FlashMem&	inFlashMem)
	: mFlashFS(inFlashFS), mFlashMem(inFlashMem), mTag(0)
{
}

/********************************* CreateFile *********************************/
/*
*	Creates and opens a file.
*	inNumSectorsReq are the number of sectors to pre-allocate.
*	If a file with the same tag already exists, it's deleted.
*	Returns true if the file was created and opened.
*/
bool FlashFSFile::CreateFile(
	uint32_t	inTag,
	uint32_t	inType,
	uint16_t	inNumSectorsReq)
{
	mTag = 0;
	return(OpenFileByIndex(mFlashFS.AddFileEntry(inTag, inType, inNumSectorsReq)));
}

/********************************** OpenFile **********************************/
bool FlashFSFile::OpenFile(
	uint32_t	inTag)
{
	return(OpenFileByIndex(mFlashFS.GetFileEntryIndex(inTag)));
}

/****************************** OpenFileByIndex *******************************/
bool FlashFSFile::OpenFileByIndex(
	uint16_t	inIndex)
{
	mFileEntryIndex = inIndex;
	if (inIndex)
	{
		mFlashFS.ReadEntry(inIndex, &mFileEntry);
		SFileInfoEntry	infoEntry;
		mFlashFS.ReadEntry(mFileEntry.info, &infoEntry);
		mTag = infoEntry.tag;
		mCurrentSector = 0xFFFF;
	}
	return(mTag != 0);
}

/********************************* DeleteFile *********************************/
bool FlashFSFile::DeleteFile(
	uint32_t	inTag)
{
	uint16_t	fileIndex = mFlashFS.GetFileEntryIndex(inTag);
	if (fileIndex)
	{
		CloseFile();
		mFlashFS.RemoveByIndex(fileIndex);
	}
	return(fileIndex != 0);
}

/********************************** CloseFile *********************************/
void FlashFSFile::CloseFile(void)
{
	mTag = 0;
	mFileEntryIndex = 0;
	mSectorsEntryIndex = 0;
}
	
/*********************************** Rewind ***********************************/
void FlashFSFile::Rewind(void)
{
	/*
	*	If the file is open
	*/
	if (mTag)
	{
		mCurrentSector = 0xFFFF;
	}
}

/******************************* ReadNextSector *******************************/
bool FlashFSFile::ReadNextSector(
	uint8_t*	outData)
{
	bool	success = false;
	/*
	*	If the file is open
	*/
	if (mTag)
	{
		if (mCurrentSector == 0xFFFF)
		{
			if (mFileEntry.sectors)
			{
				mSectorsEntryIndex = mFileEntry.sectors;
				mFlashFS.ReadEntry(mSectorsEntryIndex, &mSectorsEntry);
				mCurrentSector = mSectorsEntry.start;
			}
		} else
		{
			mCurrentSector++;
			if (mCurrentSector >= mSectorsEntry.end)
			{
				if (mSectorsEntry.next)
				{
					mSectorsEntryIndex = mSectorsEntry.next;
					mFlashFS.ReadEntry(mSectorsEntryIndex, &mSectorsEntry);
					mCurrentSector = mSectorsEntry.start;
				} else
				{
					mCurrentSector = 0xFFFF;
				}
			}
		}
		if (mCurrentSector != 0xFFFF)
		{
			success = mFlashMem.ReadSector(mCurrentSector, outData);
		}
	}
	return(success);
}

/************************************ AtEOF ***********************************/
/*
*	Returns true if there is no next sector.
*
*	Note that ReadNextSector will rewind once EOF is hit.  (e.g. the last sector
*	is read by calling ReadNextSector, i.e. at EOF, then ReadNextSector is
*	called again.)
*/
bool FlashFSFile::AtEOF(void) const
{
	return(mCurrentSector != 0xFFFF &&
			mSectorsEntry.next == 0 &&
				(mCurrentSector+1) == mSectorsEntry.end);
}

/****************************** WriteNextSector *******************************/
bool FlashFSFile::WriteNextSector(
	const uint8_t*	inData)
{
	bool	success = false;
	/*
	*	If the file is open
	*/
	if (mTag)
	{
		/*
		*	If this is the first write...
		*/
		if (mCurrentSector == 0xFFFF)
		{
			/*
			*	If this file has sectors THEN
			*	start from the first sector
			*/
			if (mFileEntry.sectors)
			{
				mSectorsEntryIndex = mFileEntry.sectors;
				mFlashFS.ReadEntry(mSectorsEntryIndex, &mSectorsEntry);
				mCurrentSector = mSectorsEntry.start;
			/*
			*	Else, if a single sector was allocated for this file.
			*/
			} else if (mFlashFS.AddSectorsByIndex(mFileEntryIndex, 1))
			{
				// Reload the file entry to get the new sectors entry
				mFlashFS.ReadEntry(mFileEntryIndex, &mFileEntry);
				mSectorsEntryIndex = mFileEntry.sectors;
				mFlashFS.ReadEntry(mSectorsEntryIndex, &mSectorsEntry);
				mCurrentSector = mSectorsEntry.start;
			} // Else fail
			
		/*
		*	Else try to move to the next sector...
		*/
		} else
		{
			mCurrentSector++;
			/*
			*	If at the end of the sectors entry...
			*/
			if (mCurrentSector >= mSectorsEntry.end)
			{
				/*
				*	If there is another sectors entry THEN
				*	load it and make its starting index the current selector.
				*/
				if (mSectorsEntry.next)
				{
					mSectorsEntryIndex = mSectorsEntry.next;
					mFlashFS.ReadEntry(mSectorsEntryIndex, &mSectorsEntry);
					mCurrentSector = mSectorsEntry.start;
				/*
				*	Else try to add an additional sector.
				*	If an additional sector was successfully added...
				*/
				} else if (mFlashFS.AddSectorsByIndex(mFileEntryIndex, 1))
				{
					// Reload the sectors entry
					mFlashFS.ReadEntry(mSectorsEntryIndex, &mSectorsEntry);
					/*
					*	If the added sector was added to a new sectors entry...
					*	(because mSectorsEntry wasn't extended)
					*/
					if (mCurrentSector >= mSectorsEntry.end)
					{
						/*
						*	If there is another sectors entry THEN
						*	load it and make its starting index the current selector.
						*/
						if (mSectorsEntry.next)
						{
							mSectorsEntryIndex = mSectorsEntry.next;
							mFlashFS.ReadEntry(mSectorsEntryIndex, &mSectorsEntry);
							mCurrentSector = mSectorsEntry.start;
						/*
						*	Else fail.
						*/
						} else
						{
							mCurrentSector = 0xFFFF;
						}
					}
				/*
				*	Else fail.
				*/
				} else
				{
					mCurrentSector = 0xFFFF;
				}
			}
		}
		if (mCurrentSector != 0xFFFF)
		{
			success = mFlashMem.WriteSector(mCurrentSector, inData);
		}
	}
	return(success);
}

/******************************* GetSectorCount *******************************/
uint16_t FlashFSFile::GetSectorCount(void) const
{
	return(mTag != 0 ? mFlashFS.CountSectors(mFileEntry.sectors) : 0);
}

/******************************** GetFileLength *******************************/
uint32_t FlashFSFile::GetFileLength(void) const
{
	uint32_t	length = 0;
	if (mTag)
	{
		SFileInfoEntry	infoEntry;
		mFlashFS.ReadEntry(mFileEntry.info, &infoEntry);
		length = infoEntry.length;
	}
	return(length);
}

/******************************** SetFileLength *******************************/
void FlashFSFile::SetFileLength(
	uint32_t	inLength)
{
	if (mTag)
	{
		SFileInfoEntry	infoEntry;
		mFlashFS.ReadEntry(mFileEntry.info, &infoEntry);
		infoEntry.length = inLength;
		mFlashFS.WriteEntry(mFileEntry.info, &infoEntry);
	}
}

/****************************** GetCreationDate *******************************/
uint32_t FlashFSFile::GetCreationDate(void) const
{
	uint32_t	creationDate = 0;
	if (mTag)
	{
		SFileDateTypeEntry	dateTypeEntry;
		mFlashFS.ReadEntry(mFileEntry.dateType, &dateTypeEntry);
		creationDate = dateTypeEntry.creation;
	}
	return(creationDate);
}

/****************************** SetCreationDate *******************************/
void FlashFSFile::SetCreationDate(
	uint32_t	inDate)
{
	if (mTag)
	{
		SFileDateTypeEntry	dateTypeEntry;
		mFlashFS.ReadEntry(mFileEntry.dateType, &dateTypeEntry);
		dateTypeEntry.creation = inDate;
		mFlashFS.WriteEntry(mFileEntry.dateType, &dateTypeEntry);
	}
}

/******************************** GetFileType *********************************/
uint32_t FlashFSFile::GetFileType(void) const
{
	uint32_t	fileType = 0;
	if (mTag)
	{
		SFileDateTypeEntry	dateTypeEntry;
		mFlashFS.ReadEntry(mFileEntry.dateType, &dateTypeEntry);
		fileType = dateTypeEntry.type;
	}
	return(fileType);
}

/******************************** SetFileType *********************************/
void FlashFSFile::SetFileType(
	uint32_t	inType)
{
	if (mTag)
	{
		SFileDateTypeEntry	dateTypeEntry;
		mFlashFS.ReadEntry(mFileEntry.dateType, &dateTypeEntry);
		dateTypeEntry.type = inType;
		mFlashFS.WriteEntry(mFileEntry.dateType, &dateTypeEntry);
	}
}

/****************************** CopyFileToFlash *******************************/
bool FlashFSFile::CopyFileToFlash(
	const char*	inSrcPath,
	uint32_t	inDstTag)
{
#ifdef __MACH__
	FILE*    file = fopen(inSrcPath, "r");
	bool	fileOpened = file != nullptr;
#else
	SdFile file;
	bool	fileOpened = file.open(inSrcPath, O_RDONLY);
#endif
	bool	success = false;
	uint8_t	buffer[FlashMem::kSectorSize];
	if (fileOpened)
	{
		uint32_t	fileType = FileUtils::FileTypeFromFilename(inSrcPath);
	#ifdef __MACH__
		{
			uint32_t	tag = inDstTag;
			char		fourByteTag[5];
			char		fileTypeStr[5];
			fourByteTag[4] = 0;
			*(uint32_t*)fourByteTag = Swap32(tag);
			if (fileType)
			{
				fileTypeStr[4] = 0;
				*(uint32_t*)fileTypeStr = Swap32(fileType);
			} else
			{
				fileTypeStr[0] = '?';
				fileTypeStr[1] = 0;
			}
			fprintf(stderr, "CreateFile(\'%s\') type = \'%s\'\n", fourByteTag, fileTypeStr);
		}
		fseek(file, 0, SEEK_END);
    	uint32_t fileSize = (uint32_t)ftell(file);
		fseek(file, 0, SEEK_SET);
	#else
		uint32_t fileSize = file.fileSize();
	#endif


		if (CreateFile(inDstTag, fileType, (fileSize/FlashMem::kSectorSize) + ((fileSize%FlashMem::kSectorSize) ? 1 : 0)))
		{
		#ifdef __MACH__
			size_t bytesRead = fread(buffer, 1, FlashMem::kSectorSize, file);
		#else
			size_t bytesRead = file.read(buffer, FlashMem::kSectorSize);
		#endif
			size_t	totalBytesRead = bytesRead;
			while (bytesRead > 0)
			{
				if (bytesRead < FlashMem::kSectorSize)
				{
					memset(buffer+bytesRead, 0, FlashMem::kSectorSize - bytesRead);
				}
				if (WriteNextSector(buffer))
				{
				#ifdef __MACH__
					bytesRead = fread(buffer, 1, FlashMem::kSectorSize, file);
				#else
					bytesRead = file.read(buffer, FlashMem::kSectorSize);
				#endif
					totalBytesRead += bytesRead;
					continue;
				}
				break;
			}
#ifdef __MACH__
			fprintf(stderr, "totalBytesRead = %d, fileSize = %d\n", (int)totalBytesRead, (int)fileSize);
#endif
			SetFileLength(fileSize);
			CloseFile();
			success = true;
#ifdef __MACH__
		} else
		{
			uint32_t	tag = inDstTag;
			char		fourByteTag[5];
			fourByteTag[4] = 0;
			*(uint32_t*)fourByteTag = Swap32(tag);
			fprintf(stderr, "CreateFile(\'%s\') failed\n", fourByteTag);
#endif
		}
	#ifdef __MACH__
		fclose(file);
	#else
		file.close();
	#endif
	}
	return(success);
}

/****************************** CopyFileFromFlash *****************************/
bool FlashFSFile::CopyFileFromFlash(
	uint32_t	inSrcTag,
	const char*	inDstPath)
{
	bool	success = false;
	uint8_t	buffer[FlashMem::kSectorSize];
	/*
	*	Perform a low level open by calling FlashFSFile::OpenFile.  Sub classes
	*	may override OpenFile.
	*
	*	If the file was opened OK...
	*/
	if (FlashFSFile::OpenFile(inSrcTag))
	{
		#ifdef __MACH__
			{
				uint32_t	tag = inSrcTag;
				char		fourByteTag[5];
				fourByteTag[4] = 0;
				*(uint32_t*)fourByteTag = Swap32(tag);
				fprintf(stderr, "OpenFile(\'%s\')\n", fourByteTag);
			}
			FILE*    file = fopen(inDstPath, "w");
			bool	fileOpened = file != nullptr;
		#else
			SdFile file;
			SdFile::dateTimeCallback(UnixTime::SDFatDateTimeCB);
			bool	fileOpened = file.open(inDstPath, O_WRONLY | O_TRUNC | O_CREAT);
		#endif
		if (fileOpened)
		{
			uint32_t	bytesToWrite = GetFileLength();
			uint32_t	bytesRead;
			size_t		totalBytesWritten = 0;
			while (bytesToWrite && ReadNextSector(buffer))
			{
				if (bytesToWrite > FlashMem::kSectorSize)
				{
					bytesRead = FlashMem::kSectorSize;
					bytesToWrite -= FlashMem::kSectorSize;
				} else
				{
					bytesRead = bytesToWrite;
					bytesToWrite = 0;
				}
			#ifdef __MACH__
				size_t bytesWritten = fwrite(buffer, 1, bytesRead, file);
			#else
				size_t bytesWritten = file.write(buffer, bytesRead);
			#endif
				if (bytesWritten == bytesRead)
				{
					totalBytesWritten += bytesWritten;
					continue;
				}
				break;
			}
		#ifdef __MACH__
			fclose(file);
		#else
			file.close();
		#endif
			success = totalBytesWritten == GetFileLength();
#ifdef __MACH__
			fprintf(stderr, "totalBytesWritten = %d\n", (int)totalBytesWritten);
#endif
#ifdef __MACH__
		} else
		{
			fprintf(stderr, "fopen failed\n");
#endif
		}
		CloseFile();
#ifdef __MACH__
	} else
	{
		uint32_t	tag = inSrcTag;
		char		fourByteTag[5];
		fourByteTag[4] = 0;
		*(uint32_t*)fourByteTag = Swap32(tag);
		fprintf(stderr, "OpenFile(\'%s\') failed\n", fourByteTag);
#endif
	}
	return(success);
}

