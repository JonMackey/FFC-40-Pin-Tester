/*
*	FlashFS.h, Copyright Jonathan Mackey 2025
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
#ifndef FlashFS_h
#define FlashFS_h

#include <inttypes.h>


/*
*	This class manages the sector indexes associated with a file.  The files are
*	referenced by a unique 32 bit tag rather than a name.
*	The file system uses a data stream to read and write entries.  This data
*	stream should be in SRAM.  On startup the data stream is initialized from
*	NOR Flash or EEPROM.  The data stream should be saved in its entirety only
*	when changes are made such as adding or deleting files.
*
*	This class allows the sector index data to be stored anywhere.
*
*	The root owns linked lists of entries.
*	Note that SxxxEntry structs must be the same size.
*
*	The SRootEntry is index 0.
*	The unfragmented SSectorsEntry is index 1.  This entry is also always the
*	tail in the root.freeSectors linked list.
*/	
struct SSectorsEntry
{
	uint16_t	next;			// Index of the next run.  0 if tail.
	uint16_t	start;			// Sector index
	uint16_t	len;			// Number of sectors in run
	uint16_t	end;			// start+len
};
struct SFileEntry
{
	uint16_t	next;			// Index of the next SFileEntry.  0 if tail.
	uint16_t	sectors;		// Index of the first SSectorsEntry
	uint16_t	info;			// Index of the SFileInfoEntry
	uint16_t	dateType;		// Index of the SFileDateTypeEntry
};

struct SFileInfoEntry
{
	uint32_t	tag;			// Unique tag
	uint32_t	length;			// Length of file
};

struct SFileDateTypeEntry
{
	uint32_t	creation;		// Unix time
	uint32_t	type;			// File type
};

struct SRootEntry
{
	uint16_t	head;			// Index of the first SFileEntry.  0 if none.
	// freeHead is only non-zero when there's entry fragmentation.
	uint16_t	freeHead;		// Index of the first free entry.
	uint16_t	freeSectors;	// Index of the first free SSectorsEntry.
	uint16_t	formatted;		// 1234 when root has been formatted
	static const uint16_t	kRootFormatted;
};

#define Swap32(data) ((((data ^ (data >> 16 | (data << 16))) & 0xFF00FFFF) >> 8) ^ (data >> 8 | data << 24))

class DataStream;
class FlashFSFile;

class FlashFS
{
	friend class FlashFSFile;
public:
							/*
							*	inEntries can't be NULL when LoadFrom is used.
							*	inEntries can be NULL when Initialize is used.
							*/
							FlashFS(
								DataStream*				inEntries = nullptr);
							/*
							*	Called only when LoadFrom/SaveTo are not used.
							*/
	void					Initialize(
								DataStream*				inEntries,
								bool					inFormat = false);
							/*
							*	Load and initialize from another data stream.
							*/
	void					LoadFrom(
								DataStream*				inDataStream);
							/*
							*	SaveTo another data stream for later use by
							*	LoadFrom.
							*/
	void					SaveTo(
								DataStream*				inDataStream);
								
	uint16_t				GetFileCount(void) const
								{return(mFileCount);}
	uint16_t				AddFileEntry(
								uint32_t				inTag,
								uint32_t				inType,
								uint16_t				inNumSectorsReq);
	bool					RemoveFileEntry(
								uint32_t				inTag);
	void					RemoveByIndex(
								uint16_t				inIndex);
	bool					AddSectors(
								uint32_t				inTag,
								uint16_t				inNumSectorsReq);
	bool					AddSectorsByIndex(
								uint16_t				inFileEntryIndex,
								uint16_t				inNumSectorsReq);
							/*
							*	Returns false if inToTag already exists.
							*/
	bool					ChangeTag(
								uint32_t				inFromTag,
								uint32_t				inToTag);
	bool					FileEntryExists(
								uint32_t				inTag) const;
	uint16_t				GetFileEntryIndex(
								uint32_t				inTag) const;
	bool					GetNthFileEntry(
								uint16_t				inIndex,
								const uint32_t*			inValidTypeList,	// nullptr if all types OK
								SFileEntry&				outFileEntry) const;
	bool					GetNextFileEntry(
								SFileEntry&				ioFileEntry) const;
	void					GetFileInfoEntry(
								const SFileEntry&		inFileEntry,
								SFileInfoEntry&			outFileInfoEntry) const;
	void					GetFileDateTypeEntry(
								const SFileEntry&		inFileEntry,
								SFileDateTypeEntry&		outFileDateTypeEntry) const;
	uint32_t				GetFileType(
								uint32_t				inTag) const;
	uint16_t				GetDirtyIndex(void) const;
	uint16_t				CountFreeSectors(void) const;
	void					DumpRoot(void) const;
protected:
	DataStream*			mEntries;
	SRootEntry			mRoot;
	uint16_t			mFileCount;
	uint16_t			mEntryCount;	// Also the next unfragmented entry index
	uint16_t			mMaxEntryCount;
	uint16_t			mFreeEntryCount;
	mutable uint16_t	mDirtyIndex;
	
	uint16_t				CountEntries(
								uint16_t				inHeadIndex) const;
	uint16_t				AllocateEntry(void);
	void					FreeEntry(
								uint16_t				inIndex);
	uint16_t				CountSectors(
								uint16_t				inSectorsIndex) const;
	uint16_t				AllocateSectors(
								uint16_t				inNumSectorsReq);
	void					FreeSectors(
								uint16_t				inIndex);
	void					ReadEntry(
								uint16_t				inIndex,	// Physical entry index
								void*					inEntry) const;
	void					WriteEntry(
								uint16_t				inIndex,	// Physical entry index
								const void*				inEntry) const;
};

#endif // FlashFS_h

