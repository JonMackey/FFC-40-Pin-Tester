/*
*	XCachedMenuItemProvider.h, Copyright Jonathan Mackey 2026
*
*	This abstract class provides a string cache for menu items.  The number of
*	cache entries must be equal to or greater than the number of menu items.
*	The required subclass must implement LoadNthItemStr().
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
#ifndef XCachedMenuItemProvider_h
#define XCachedMenuItemProvider_h

#include "XMenuItemProvider.h"
#include "PlatformDefs.h"

// The macro below should be used to define the item str cache size.
// maxStrLen should be an even number AND it should include the null terminator.
#define XItemStrCacheSize(numItems, maxStrLen) ((sizeof(uint16_t)*(uint16_t)numItems)+(maxStrLen*(uint16_t)numItems))

class XCachedMenuItemProvider : public XMenuItemProvider
{
public:
							XCachedMenuItemProvider(
								uint8_t*				inItemStrCache,
								uint16_t				inNumEntries,
								uint16_t				inMaxStrLen);
protected:
	uint16_t		mNumEntries;
	uint16_t		mMaxStrLen;
	uint32_t		mEntrySize;
	uint8_t*		mItemStrCache;

	virtual const char*		GetNthItemStr(
								uint16_t				inItemIndex);
	virtual const char*		LoadNthItemStr(
								uint16_t				inItemIndex,
								char*					outItemStr) = 0;
								
	void					ClearCache(void);
};
#endif // XCachedMenuItemProvider_h
