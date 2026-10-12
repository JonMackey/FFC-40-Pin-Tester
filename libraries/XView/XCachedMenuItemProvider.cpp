/*
*	XCachedMenuItemProvider.cpp, Copyright Jonathan Mackey 2026
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
#include "XCachedMenuItemProvider.h"
#ifdef __MACH__
#include <stdio.h>
#endif

struct SItemStrCacheEntry
{
	uint16_t			idx;
	char				itemStr[2];
};


/**************************** XCachedMenuItemProvider *****************************/
XCachedMenuItemProvider::XCachedMenuItemProvider(
	uint8_t*	inItemStrCache,
	uint16_t	inNumEntries,
	uint16_t	inMaxStrLen)
	: mItemStrCache(inItemStrCache), mNumEntries(inNumEntries),
		mMaxStrLen(inMaxStrLen)
{
	mEntrySize = sizeof(uint16_t) + inMaxStrLen;
}

/********************************* ClearCache *********************************/
void XCachedMenuItemProvider::ClearCache(void)
{
	SItemStrCacheEntry*	itemCacheEntry;
	uint8_t*	cacheEntryPtr = mItemStrCache;
	for (uint32_t entryIndex = 0; entryIndex < mNumEntries; entryIndex++)
	{
		itemCacheEntry = (SItemStrCacheEntry*)cacheEntryPtr;
		cacheEntryPtr += mEntrySize;
		itemCacheEntry->idx = 0;	// 0 == Unused
		itemCacheEntry->itemStr[0] = 0;
	}
}

/******************************** GetNthItemStr *******************************/
const char* XCachedMenuItemProvider::GetNthItemStr(
	uint16_t	inItemIndex)
{
	char*	itemStrPtr = nullptr;
	bool	foundIdx = false;
	/*
	*	If inItemIndex is valid...
	*/
	if (inItemIndex < mItemCount)
	{
		/*
		*	See if the index being requested is already be in the mItemStrCache.
		*/
		uint32_t	entryIndex = 0;
		uint8_t*	cacheEntryPtr = mItemStrCache;
		SItemStrCacheEntry*	itemCacheEntry = (SItemStrCacheEntry*)cacheEntryPtr;
		SItemStrCacheEntry*	lowItemCacheEntry = itemCacheEntry;
		SItemStrCacheEntry*	highItemCacheEntry = itemCacheEntry;
		
		inItemIndex++;	// Bump the item index to be in the 1 to n range so that
						// 0 can signify that the entry is unused. 
		/*
		*	Go through all of the cache entries to see if inItemIndex is already
		*	loaded.  At the same time note the entry index of the highest and
		*	the lowest item index.
		*/
		for (; entryIndex < mNumEntries; entryIndex++)
		{
			if (inItemIndex != itemCacheEntry->idx)
			{
				if (itemCacheEntry->idx < lowItemCacheEntry->idx)
				{
					lowItemCacheEntry = itemCacheEntry;
				} else if (itemCacheEntry->idx > highItemCacheEntry->idx)
				{
					highItemCacheEntry = itemCacheEntry;
				}
				cacheEntryPtr += mEntrySize;
				itemCacheEntry = (SItemStrCacheEntry*)cacheEntryPtr;
				continue;
			}
			itemStrPtr = itemCacheEntry->itemStr;
			foundIdx = true;
			break;
		}
		/*
		*	If inItemIndex wasn't found THEN
		*	call the subclass to load it via LoadNthItemStr()
		*/ 
		if (!foundIdx)
		{
			{
				int32_t	indexDeltaLow = inItemIndex - lowItemCacheEntry->idx;
				if (indexDeltaLow < 0)indexDeltaLow = -indexDeltaLow;
				int32_t	indexDeltaHigh = inItemIndex - highItemCacheEntry->idx;
				if (indexDeltaHigh < 0)indexDeltaHigh = -indexDeltaHigh;
				/*
				*	Use the cache entry currently containing the item index
				*	furthest from inItemIndex to store item string.
				*/
				itemCacheEntry = (indexDeltaHigh > indexDeltaLow) ?
										highItemCacheEntry : lowItemCacheEntry;
			}
			itemCacheEntry->idx = inItemIndex;
			itemStrPtr = itemCacheEntry->itemStr;
			LoadNthItemStr(inItemIndex-1, itemStrPtr);
		}
	//	fprintf(stderr, "Requested %hd %s\n",  inItemIndex-1, foundIdx ? "":"loaded");
	}
	return(itemStrPtr);
}
