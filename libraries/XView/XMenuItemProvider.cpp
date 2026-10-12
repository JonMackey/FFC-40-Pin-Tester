/*
*	XMenuItemProvider.cpp, Copyright Jonathan Mackey 2026
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
#include "XMenu.h"
#include "XMenuItem.h"
#include "XMenuItemProvider.h"
#ifdef __MACH__
#include <stdio.h>
#endif

const char	XMenuItemProvider::kUpArrowStr[] = "<";
const char	XMenuItemProvider::kDownArrowStr[] = ">";

/****************************** XMenuItemProvider *****************************/
XMenuItemProvider::XMenuItemProvider(void)
	: mUnusedMenuItems(nullptr)
{
}

/****************************** UpdateMenuItems *******************************/
/*
*	Constraint: The number of XMenuItems must be greater than 2 in order for
*	scrolling to work.
*/
bool XMenuItemProvider::UpdateMenuItems(
	XMenuItem*	inItemSelected)
{
	bool	changeMade = inItemSelected == nullptr;
	if (inItemSelected &&
		mItemIncrement > 0)
	{
		if (inItemSelected->IsUpArrow())
		{
			if (mFirstItemIndex > mItemIncrement + 1)
			{
				mFirstItemIndex -= mItemIncrement;
			} else
			{
				mFirstItemIndex = 0;
			}
			changeMade = true;
		} else if (inItemSelected->IsDownArrow())
		{
			if (!mFirstItemIndex)
			{
				mFirstItemIndex = 1;
			}
			mFirstItemIndex += mItemIncrement;
			if (mFirstItemIndex + mItemIncrement + 1 >= mItemCount)
			{
				mFirstItemIndex = mItemCount - mItemIncrement - 2;
			}
			changeMade = true;
		}
	}
	if (changeMade)
	{
		XMenuItem*	thisItem = mMenu->GetMenuItems();
		if (mFirstItemIndex > 0)
		{
			thisItem->SetString(kUpArrowStr);
			thisItem->SetTag(0xFFFE);
			thisItem->SetState(XMenuItem::eOff);
			thisItem = thisItem->NextItem();
		}
		const char*	thisItemStr = GetNthItemStr(mFirstItemIndex);
		uint16_t	thisItemIndex = mFirstItemIndex;
		for (; thisItem; )
		{
			if (thisItem->NextItem() ||
				thisItemIndex+1 == mItemCount ||
				thisItemIndex >= mItemCount)
			{
				thisItem->SetString(thisItemStr);
				thisItem->SetTag(thisItemIndex);
				thisItem->SetState(thisItemStr != nullptr ? XMenuItem::eOff : XMenuItem::eDisabled);
			} else
			{
				thisItem->SetString(kDownArrowStr);
				thisItem->SetTag(0xFFFF);
				thisItem->SetState(XMenuItem::eOff);
			}
			thisItem = thisItem->NextItem();
			if (thisItem)
			{
				thisItemIndex++;
				thisItemStr = GetNthItemStr(thisItemIndex);
				continue;
			}
			break;
		}
	}
	return(changeMade);
}

/***************************** UpdateMenuItemList *****************************/
/*
*	Adjusts the XMenuItem list to be less than or equal to the mItemCount with
*	a minimum of one item in the list.
*	Any unused XMenuItems are stored in mUnusedMenuItems.
*
*	This routine should be called from the end of GetWidestItemWidth.
*	GetWidestItemWidth sets mItemCount whenever the popup is to be shown.
*/
void XMenuItemProvider::UpdateMenuItemList(void)
{
	if (mMenu)
	{
		XMenuItem*	thisItem;
		/*
		*	If there are unused items THEN
		*	find the end of the menu item list then concatenate the unused
		*	items.
		*/
		if (mUnusedMenuItems)
		{
			thisItem = mMenu->GetMenuItems();
			while (thisItem)
			{
				XMenuItem*	nextItem = thisItem->NextItem();
				if (nextItem)
				{
					thisItem = nextItem;
					continue;
				}
				break;
			}
			thisItem->SetNextItem(mUnusedMenuItems);
			mUnusedMenuItems = nullptr;
		}
		/*
		*	Adjust the length of the menu item list to be less than or equal to
		*	mItemCount.
		*/
		uint16_t	count = mItemCount;
		if (count < 1)
		{
			count = 1;
		}
		/*
		*	If the item count changed since the last time the menu was displayed THEN
		*	update the index.
		*/
		if (mFirstItemIndex >= count)
		{
			mFirstItemIndex = 0;
		}
		uint32_t	menuItemCount = 0;
		thisItem = mMenu->GetMenuItems();
		XMenuItem*	prevItem = nullptr;
		while (thisItem)
		{
			if (count)
			{
				menuItemCount++;
				count--;
				prevItem = thisItem;
				thisItem = thisItem->NextItem();
				continue;
			}
			mUnusedMenuItems = thisItem;
			prevItem->SetNextItem(nullptr);
			break;
		}
		if (menuItemCount > 3)
		{
			mItemIncrement = menuItemCount - 3;
		} else
		{
			mItemIncrement = 0;
		}
	}
}
