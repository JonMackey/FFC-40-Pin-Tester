/*
*	XFSMenuItemProvider.cpp, Copyright Jonathan Mackey 2026
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
#include "XFSMenuItemProvider.h"
#include "XFont.h"
#include "FlashFS.h"
#ifdef __MACH__
#include <stdio.h>
#include <string>
#endif

/**************************** XFSMenuItemProvider *****************************/
XFSMenuItemProvider::XFSMenuItemProvider(
	uint8_t*		inItemStrCache,
	uint16_t		inNumEntries,
	const uint32_t*	inValidTypeList)
	: XCachedMenuItemProvider(inItemStrCache, inNumEntries, XFSMenuItemProvider::eMaxStrLen),
	  mFlashFS(nullptr), mValidTypeList(inValidTypeList)
{
}

/******************************* LoadNthItemStr *******************************/
/*
*	This is called by XCachedMenuItemProvider
*/
const char* XFSMenuItemProvider::LoadNthItemStr(
	uint16_t	inItemIndex,
	char*		outFileTagStr)
{
	SFileEntry	fileEntry;
	if (mFlashFS && mFlashFS->GetNthFileEntry(inItemIndex, mValidTypeList, fileEntry))
	{
		SFileInfoEntry	infoEntry;
		mFlashFS->GetFileInfoEntry(fileEntry, infoEntry);
		*(uint32_t*)outFileTagStr = Swap32(infoEntry.tag);
		outFileTagStr[4] = 0;
	} else
	{
		outFileTagStr[0] = 0;
	}
	return(outFileTagStr);
}


/***************************** GetWidestItemWidth *****************************/
/*
*	It's assumed that inXFont is the current font (XFont::SetFont() was called.)
*/
uint16_t XFSMenuItemProvider::GetWidestItemWidth(
	XFont*	inXFont)
{
	/*
	*	The cache needs to be cleared whenever GetWidestItemWidth is called so
	*	that stale items can be removed.  (e.g. adding or removing a file)
	*/
	ClearCache();
	if (inXFont && mFlashFS)
	{
		SFileEntry		fileEntry;
		SFileInfoEntry	infoEntry;
		uint16_t		widest = 0;
		uint32_t		fileCount = 0;
		
		if (mFlashFS->GetNthFileEntry(0, mValidTypeList, fileEntry))
		{
			char		tagStr[5];
			
			tagStr[4] = 0;
			do
			{
				mFlashFS->GetFileInfoEntry(fileEntry, infoEntry);
				*(uint32_t*)tagStr = Swap32(infoEntry.tag);
				fileCount++;
				uint16_t	thisHeight, thisWidth;
				inXFont->MeasureStr(tagStr, thisHeight, thisWidth);
				if (widest < thisWidth)
				{
					widest = thisWidth;
				}
			} while(mFlashFS->GetNextFileEntry(fileEntry));
		}
		mWidestItemWidth = widest;
		mItemCount = fileCount;
		UpdateMenuItemList();
	}
	return(mWidestItemWidth);
}
