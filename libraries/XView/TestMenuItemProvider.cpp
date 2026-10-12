/*
*	TestMenuItemProvider.cpp, Copyright Jonathan Mackey 2026
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
#include "TestMenuItemProvider.h"
#include "XFont.h"
#ifdef __MACH__
#include <stdio.h>
#endif

const char*	kTestStrs[] =
{
	"DustCollectorFull.wav",
	"FilterNeedsCleaning.wav",
	"SomeoneAtFrontdoor.wav",
	"SomeoneAtBackdoor.wav",
	"YourLightsAreOn.wav",
	"Power on.wav",
	"Power off.wav",
	nullptr
};

/**************************** TestMenuItemProvider ****************************/
TestMenuItemProvider::TestMenuItemProvider(void)
{
}

/******************************** GetNthItemStr *******************************/
const char* TestMenuItemProvider::GetNthItemStr(
	uint16_t	inIndex)
{
	return(inIndex < mItemCount ? kTestStrs[inIndex] : nullptr);
}

/***************************** GetWidestItemWidth *****************************/
/*
*	It's assumed that inXFont is the current font (XFont::SetFont() was called.)
*	This routine also updates mItemCount
*/
uint16_t TestMenuItemProvider::GetWidestItemWidth(
	XFont*	inXFont)
{
	if (mWidestItemWidth == 0 && inXFont)
	{
		uint16_t	widest = 0;
		uint32_t	itemCount = 0;
		uint16_t	thisHeight, thisWidth;
		
		for (; kTestStrs[itemCount] != nullptr; itemCount++)
		{
			inXFont->MeasureStr(kTestStrs[itemCount], thisHeight, thisWidth);
			if (widest < thisWidth)
			{
				widest = thisWidth;
			}
		}
		mWidestItemWidth = widest;
		mItemCount = itemCount;
		UpdateMenuItemList();
	}
	
	return(mWidestItemWidth);
}

