/*
*	PinConfigs.h, Copyright Jonathan Mackey 2026
*	Class to manage pin configurations.
*
*	The storage for this class isn't tied to any specific storage medium AND
*	when adding/removing configurations, the existing configurations will not
*	move.  This is implemented using a generic stream, which means it can be
*	stored anywhere. It's also a simple linked list, so minimal changes are
*	needed to add and remove configurations.
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
#ifndef PinConfigs_h
#define PinConfigs_h

#include <inttypes.h>

class DataStream;

struct SPinsData
{
	uint8_t		config[20];
	char		name[64];
	
	void					SetName(
								const char*				inName);
	void					SetConfig(
								const uint8_t*			inConfig);
	void					GetConfig(
								uint8_t*				outConfig) const;
};

/*
*	next and prev below, when multiplied by the size of SPinsDataLink, is a
*	physical offset from the start of the data stream.
*/
struct SPinsDataLink
{
	uint16_t	prev;	// Index of the previous SPinsDataLink.  0 if head.
	uint16_t	next;	// Index of the next SPinsDataLink.  0 if tail.
	SPinsData	data;
};

struct SPinConfigRoot
{
	uint16_t	tail;	// Index of the last SPinsDataLink.
	uint16_t	head;	// Index of the first SPinsDataLink.  0xFFFF if none.
	/*
	*	freeHead is the first free SPinsDataLink.  This will only be non-zero
	*	when a SPinsDataLink is removed within the set of defined
	*	configurations. The length of the data stream determines the capacity. 
	*	freeHead only indicates that there is fragmentation within the link data.
	*/
	uint16_t	freeHead;
	uint16_t	formatted;		// 1234 when root has been formatted
	static const uint16_t	kRootFormatted;
};

/*
*	Logical indexes are 0 to N
*	Physical indexes are 1 to N with 0 reserved as invalid/doesn't exist
*/


class PinConfigs
{
public:
							PinConfigs(void);
	void					begin(
								DataStream*				inDataStream,
								bool					inFormat = false);
	uint16_t				GetCount(void) const
								{return(mCount);}
	const char*				GetCurrentName(void) const
								{return(mCurrent.data.name);}
	void					GetCurrentConfig(
								uint8_t*				outConfig) const
								{mCurrent.data.GetConfig(outConfig);}
	void					SetCurrentConfig(
								const uint8_t*			inConfig);
	uint16_t				GetCurrentIndex(void) const		// Unsorted physical link index
								{return(mCurrentIndex);}
	int16_t					GetLogicalIndex(void) const;	// Sorted logical index (0 to N)
	bool					IsValidIndex(
								uint16_t				inRecIndex); // Unsorted physical link index
							/*
							*	Load the next sorted SPinsDataLink
							*/
	bool					Next(
								bool					inWrap = true);
							/*
							*	Gets the next sorted physical index without loading
							*/
	uint16_t				GetNextIndex(
								bool					inWrap = true) const;
							/*
							*	Load the previous sorted SPinsDataLink
							*/
	bool					Previous(
								bool					inWrap = true);
							/*
							*	Gets the previous sorted physical index without loading
							*/
	uint16_t				GetPreviousIndex(
								bool					inWrap = true) const;
	void					GoToLink(
								uint16_t				inRecIndex);	// Unsorted physical link index
	bool					GoToNthLink(
								uint16_t				inLogIndex);	// Sorted logical index
	bool					GoToNamedLink(
								const char*				inName);
	bool					RemoveCurrent(void);
	uint16_t				AddConfig(
								const char*				inName,
								const uint8_t*			inConfig);
#ifdef __MACH__
	uint32_t				GetArchiveSize(void) const
								{return(mCount*sizeof(SPinsData));}
	void					SaveToArchive(
								void*					outArchive) const;
	void					LoadFromArchive(
								uint32_t				inArchiveSize,
								const void*				inArchive);
#endif
	bool					LoadFromSD(
								const char*				inPath,
								bool					inMerge);
	bool					SaveToSD(
								const char*				inPath);

	void					DumpConfigs(void) const;
protected:
	DataStream*			mDataStream;
	SPinConfigRoot		mRoot;
	SPinsDataLink		mCurrent;
	uint16_t			mCurrentIndex;
	uint16_t			mCount;
	uint16_t			mMaxLinkCount;
	
	uint16_t				AddLink(
								SPinsDataLink&			inLink);
	void					FormatRoot(void);
	void					ReadRoot(void);
	void					WriteRoot(void);
	void					ReadLink(
								uint16_t				inIndex,	// Physical link index
								SPinsDataLink*			inLink) const;
	void					WriteLink(
								uint16_t				inIndex,	// Physical link index
								const SPinsDataLink*	inLink) const;
	bool					GoToRelativeLink(
								int16_t					inRelLogIndex);	// Relative sorted logical index
	static char*			UInt32ToHexStr(
								uint32_t				inNum,
								char*					inBuffer);
};

#endif // PinConfigs_h
