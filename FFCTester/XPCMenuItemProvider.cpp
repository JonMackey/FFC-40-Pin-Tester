/*
*	XPCMenuItemProvider.cpp, Copyright Jonathan Mackey 2026
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
#include "XPCMenuItemProvider.h"
#include "XFont.h"
#include "PinConfigs.h"
#ifdef __MACH__
#include <stdio.h>
#include <string>
#endif


/**************************** XPCMenuItemProvider *****************************/
XPCMenuItemProvider::XPCMenuItemProvider(
	uint8_t*		inItemStrCache,
	uint16_t		inNumEntries)
	: XCachedMenuItemProvider(inItemStrCache, inNumEntries,
		XPCMenuItemProvider::eMaxStrLen),
	  mPinConfigs(nullptr)
{
}

/******************************* LoadNthItemStr *******************************/
/*
*	This is called by XCachedMenuItemProvider
*/
const char* XPCMenuItemProvider::LoadNthItemStr(
	uint16_t	inItemIndex,
	char*		outFilename)
{
	if (mPinConfigs != nullptr &&
		mPinConfigs->GoToNthLink(inItemIndex))
	{
		strcpy(outFilename, mPinConfigs->GetCurrentName());
	}

	return(outFilename);
}

/***************************** GetWidestItemWidth *****************************/
/*
*	It's assumed that inXFont is the current font (XFont::SetFont() was called.)
*/
uint16_t XPCMenuItemProvider::GetWidestItemWidth(
	XFont*	inXFont)
{
	ClearCache();
	if (inXFont != nullptr &&
		mPinConfigs != nullptr)
	{
		mItemCount = mPinConfigs->GetCount();
		bool okToContinue = mPinConfigs->GoToNthLink(0);
		uint16_t	widest = 0;
		uint16_t	thisHeight, thisWidth;
		for (; okToContinue; okToContinue = mPinConfigs->Next(false))
		{
			inXFont->MeasureStr(mPinConfigs->GetCurrentName(), thisHeight, thisWidth);
			if (widest < thisWidth)
			{
				widest = thisWidth;
			}
		}
		mWidestItemWidth = widest;
	} else
	{
		mItemCount = 0;
	}
	UpdateMenuItemList();
	return(mWidestItemWidth);
}
