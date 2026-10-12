/*
*	FlashFSFile.h, Copyright Jonathan Mackey 2025
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


/*
*	This class manages access to file data stored on NOR Flash.  The granularity
*	of access is the size of a sector.  This class was written to support audio
*	codecs without double buffering as would happen with classes like FatFs.
*	This class also allows for storing the file system data on EEPROM while the
*	file data is stored on flash.
*/	
#ifndef FlashFSFile_h
#define FlashFSFile_h

#include <inttypes.h>
#include "FlashFS.h"

class FlashMem;

class FlashFSFile
{
public:
							FlashFSFile(
								FlashFS&				inFlashFS,
								FlashMem&				inFlashMem);
	bool					CreateFile(
								uint32_t				inTag,
								uint32_t				inType,
								uint16_t				inNumSectorsReq = 0);
	virtual bool			OpenFile(
								uint32_t				inTag);
	bool					DeleteFile(
								uint32_t				inTag);
	void					CloseFile(void);
	
	void					Rewind(void);
	bool					ReadNextSector(
								uint8_t*				outData);
	bool					AtEOF(void) const;
	bool					WriteNextSector(
								const uint8_t*			inData);
	uint16_t				GetSectorCount(void) const;
	uint32_t				GetFileLength(void) const;
	void					SetFileLength(
								uint32_t				inLength);
	uint32_t				GetCreationDate(void) const;
	void					SetCreationDate(
								uint32_t				inDate);
	uint32_t				GetFileType(void) const;
	void					SetFileType(
								uint32_t				inType);
	// SD Utility functions
	bool					CopyFileToFlash(
								const char*				inSrcPath,
								uint32_t				inDstTag);
	bool					CopyFileFromFlash(
								uint32_t				inSrcTag,
								const char*				inDstPath);
protected:
	FlashFS&		mFlashFS;
	FlashMem&		mFlashMem;
	SFileEntry		mFileEntry;
	SSectorsEntry	mSectorsEntry;
	uint32_t		mTag;
	uint16_t		mSectorsEntryIndex;
	uint16_t		mFileEntryIndex;
	uint16_t		mCurrentSector;
	
	bool					OpenFileByIndex(
								uint16_t				inIndex);
};

#endif // FlashFSFile_h

