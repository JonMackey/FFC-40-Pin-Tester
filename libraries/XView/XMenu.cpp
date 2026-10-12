/*
*	XMenu.cpp, Copyright Jonathan Mackey 2023-2026
*	Base class for a menu.
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
#include "XRootView.h"
#include "DisplayController.h"
#ifdef __MACH__
#include <stdio.h>
#endif

// These colors should probably be moved to a theme file so they can be changed
// per project.
static const uint16_t	kViewBGColor = 0xEF3C;
static const uint16_t	kSelectedItemBGColor = 0xFCA6;
static const uint16_t	kSelectedItemTextColor = 0xFFFF;
static const uint16_t	kItemTextColor = 0;
static const uint16_t	kDisabledItemColor = 0xCE79;	// Text & separators

/*********************************** XMenu ************************************/
/*
*	Constructor for static fixed length menus
*/
XMenu::XMenu(
	uint16_t				inTag,
	XFont::Font*			inFont,
	XMenuItem*				inMenuItems,
	XView*					inAnchorView,
	uint16_t				inBGColor)
	: XView(0, 0, 0, 0, inTag, nullptr, nullptr, inAnchorView, false),
	  mFont(inFont), mMenuItems(inMenuItems), mSelectedItem(nullptr),
	  mAutoClearSelected(true),  mSuperViewAnchor(XMenu::eAnchorToBottom),
	  mBGColor(inBGColor), mMenuItemProvider(nullptr)
{
}

/*********************************** XMenu ************************************/
/*
*	Constructor for variable length menus
*/
XMenu::XMenu(
	uint16_t				inTag,
	XFont::Font*			inFont,
	XMenuItem*				inMenuItems,
	uint16_t				inNumMenuItems,
	uint16_t				inBGColor,
	XMenuItemProvider*		inMenuItemProvider)
	: XView(0, 0, 0, 0, inTag, nullptr, nullptr, nullptr, false),
	  mFont(inFont), mMenuItems(inMenuItems), mSelectedItem(nullptr),
	  mAutoClearSelected(true),  mSuperViewAnchor(XMenu::eAnchorToBottom),
	  mBGColor(inBGColor), mMenuItemProvider(nullptr)
{
	SetMenuItemProvider(inMenuItemProvider);
	/*
	*	Initialize the menu item linked list...
	*/
	for (int16_t i = inNumMenuItems-2; i >= 0; i--)
	{
		inMenuItems[i].SetNextItem(&inMenuItems[i+1]);
	}
}

/************************************ Show ************************************/
void XMenu::Show(void)
{
	mVisible = true;
	/*
	*	Clear the last selected item, if any.
	*/
	if (mAutoClearSelected)
	{
		ClearSelectedItem();
	}
	/*
	*	XMenu's view coordinates are global (display controller coordinates).
	*	This makes hit testing and drawing/erasing easier to manage.
	*
	*	Find the widest menu item string...
	*/
	XFont*	xFont = mFont->GetXFont();
	DisplayController*	display = xFont->GetDisplay();
	uint16_t	displayWidth = display->GetColumns();
	xFont->SetFont(mFont);
	mTextInset = xFont->GetFontHeader().height;
	mTextItemInsetY = mTextInset/3;
	mItemHeight = xFont->GetFontHeader().ascent*2;
	/*
	*	The fractional item heights are rounded up.  Each fractional height is
	*	rounded up from the previous fractional height.
	*	Ex: item height = 22
	*				1/2 = 11 or (22+1)/2
	*				1/4 = 6 or (11+1)/2
	*				1/8 = 3 or (6+1)/2
	*/
	mHalfItemHeight = (mItemHeight+1)/2;	// Used as the height of a separator item
	mQuarterItemHeight = (mHalfItemHeight+1)/2;		// Used as the view's corner radius
	mEighthItemHeight = (mQuarterItemHeight+1)/2;	// Used as the selected item's corner radius
	/*
	*	Position the menu based on the mSuperViewAnchor hint and the location of
	*	the menu relative to the bounds of the display.
	*/
	{
		uint16_t	viewWidth = GetWidestItemWidth() + (mTextInset*2);
		uint16_t	numSeparators = 0;
		uint16_t viewHeight = (mQuarterItemHeight*2) +
								(GetItemCount(&numSeparators) * mItemHeight) -
									(numSeparators*mHalfItemHeight);
		if (viewHeight > display->GetRows())
		{
			viewHeight = display->GetRows();
		}
		/*
		*	Note that the section below that determines where to anchor the
		*	menu needs to be changed to support submenus.  NOT IMPLEMENTED YET.
		*
		*	Scenario: a menu with a submenu is hit.  The item is highlighted,
		*	then the submenu is opened.  The superview/anchor of the menu being
		*	opened is an XMenu.  Currently the superview is the size of the
		*	entire menu. It should probably be temporarily resized to the size
		*	of just the selected item.  After the submenu is dismissed, this
		*	routine should be called again to redraw the items overwritten.
		*/
		int16_t	globalX = 0;
		int16_t	globalY = 0;
		// mSuperView is the anchor view
		mSuperView->LocalToGlobal(globalX, globalY);
		if (mSuperViewAnchor == eAnchorToBottom)
		{
			globalY += mSuperView->Height();
			// Clip the viewHeight to the display bottom if needed.
			uint16_t	maxHeight = display->GetRows() - globalY;
			if (viewHeight > maxHeight)
			{
				viewHeight = maxHeight;
			}
		/*
		*	Else, eAnchorCentered
		*	The selected item is centered over the superview
		*/
		} else
		{
			if (viewWidth < mSuperView->Width())
			{
				viewWidth = mSuperView->Width();
			}
			// Get the top of the selected item
			int16_t	itemTop = GetItemTop(mSelectedItem ? mSelectedItem : mMenuItems);
			// Center it over the superview 
			itemTop-=((mSuperView->Height() - mItemHeight)/2);
			globalY -= itemTop;
			if (globalY < 0)
			{
				globalY = 0;
			}
			if (globalY + viewHeight > display->GetRows() - 4)
			{
				globalY = display->GetRows() - viewHeight - 4;
			}
			
		}
		if (viewWidth > displayWidth)
		{
			viewWidth = displayWidth;
		}

		if (mSuperViewAnchor == eAnchorToBottom)
		{
			/*
			*	If the menu anchor is on the right side of the display, the menu will
			*	draw to the left, else it draws to the right.
			*/
			if (globalX > (displayWidth/2))
			{
				globalX = globalX + mSuperView->Width() - viewWidth;
				if (globalX < 0)
				{
					globalX = 0;
				}
			}
		}
		if ((globalX + viewWidth) > displayWidth)
		{
			globalX = displayWidth - viewWidth;
		} else if (globalX < 0)
		{
			globalX = 0;
		}
		mX = globalX;
		mY = globalY;
		mWidth = viewWidth;
		mHeight = viewHeight;
	}

	if (mMenuItemProvider)
	{
		mMenuItemProvider->UpdateMenuItems();
	}

	/*
	*	Draw the menu
	*/
	{
		mItemWidth = mWidth - (mTextInset*2);
		mItemFrameWidth =  mWidth - (mQuarterItemHeight*2);
		display->SetFGColor(0xB5B6);
		display->SetBGColor(mBGColor);
		display->DrawRoundedRect(mX, mY, mWidth, mHeight, mQuarterItemHeight, 255, true);
		display->SetFGColor(kViewBGColor);
		display->DrawRoundedRect(mX+1, mY+1, mWidth-2, mHeight-2, mQuarterItemHeight);
		int16_t		itemY = mY+mQuarterItemHeight;
		for (XMenuItem*	thisItem = mMenuItems; thisItem; thisItem = thisItem->mNextItem)
		{
			itemY = DrawItem(itemY, thisItem);
		}
	}
}

/************************************ Hide ************************************/
void XMenu::Hide(void)
{
	// mX, mY in XMenu is global.  Convert to local first.
	mSuperView->GlobalToLocal(mX, mY);
	XView::Hide();
}

/********************************* GetItemTop *********************************/
int16_t XMenu::GetItemTop(
	XMenuItem*	inItem)
{
	int16_t		itemTop = mQuarterItemHeight;
	for (XMenuItem*	thisItem = mMenuItems; thisItem;
			thisItem = thisItem->mNextItem)
	{
		if (thisItem != inItem)
		{
			if (thisItem->IsSeparator())
			{
				itemTop += mHalfItemHeight;
			} else
			{
				itemTop += mItemHeight;
			}
		} else
		{
			break;
		}
	}
	return(itemTop);
}

/********************************** DrawItem **********************************/
uint16_t XMenu::DrawItem(
	int16_t		inGlobalY,
	XMenuItem*	inItem,
	bool		inRedrawBG)
{
	XFont*	xFont = mFont->GetXFont();
	DisplayController*	display = xFont->GetDisplay();
	display->SetBGColor(kViewBGColor);
	if (!inItem->IsSeparator())
	{
		char firstChar = inItem->GetFirstChar();
		if (inItem->IsOff())
		{
			if (inRedrawBG || firstChar == '<' || firstChar == '>')
			{
				display->FillRect(mX+mQuarterItemHeight, inGlobalY, mItemFrameWidth, mItemHeight, kViewBGColor);
			}
			xFont->SetBGTextColor(kViewBGColor);
			xFont->SetTextColor(inItem->IsEnabled() ? kItemTextColor : kDisabledItemColor);
		} else
		{
			display->SetFGColor(kSelectedItemBGColor);
			display->DrawRoundedRect(mX+mQuarterItemHeight, inGlobalY, mItemFrameWidth, mItemHeight, mEighthItemHeight);
			xFont->SetBGTextColor(kSelectedItemBGColor);
			xFont->SetTextColor(kSelectedItemTextColor);
		}

		switch (firstChar)
		{
			case 0:		// Null string ptr.  Nothing to draw.
				break;
			case '<':	// Interpreted as an up arrow
			{
				uint16_t	xCtr = mX + (mWidth/2);
				uint16_t	yCtr = inGlobalY + (mItemHeight/2);
				display->SetFGColor(XFont::eBlack);
				display->DrawLine(xCtr-6, yCtr+3, xCtr, yCtr-3, 2);
				display->DrawLine(xCtr, yCtr-3, xCtr+6, yCtr+3, 2);
				break;
			}
			case '>':	// Interpreted as a down arrow
			{
				uint16_t	xCtr = mX + (mWidth/2);
				uint16_t	yCtr = inGlobalY + mTextItemInsetY + (mItemHeight/2);
				display->SetFGColor(XFont::eBlack);
				display->DrawLine(xCtr-6, yCtr-3, xCtr, yCtr+3, 2);
				display->DrawLine(xCtr, yCtr+3, xCtr+6, yCtr-3, 2);
				/*
				*	Without the FillRect below, the last line drawn won't draw
				*	completely.  The fill rect below doesn't visually change
				*	anything but seems to force the display to complete the
				*	previous draw.  This bug is on the ILI9488 display.
				*/
				display->FillRect(mX+mTextInset, inGlobalY+mTextItemInsetY,
							1, mItemHeight-2, kViewBGColor);	// Previous fill bug "fix"
				break;
			}
			default:	// Draw the string
				xFont->DrawAligned(inItem->GetString(),
								mX+mTextInset, inGlobalY+mTextItemInsetY,
								mItemWidth, XFont::eAlignLeft, true);
				break;
		}
		inGlobalY+=mItemHeight;
	/*
	*	Else draw the separator
	*/
	} else
	{
		display->FillRect(mX+mTextInset, inGlobalY+mQuarterItemHeight, mItemWidth, 1, kDisabledItemColor);
		inGlobalY+=mHalfItemHeight;
	}
	return(inGlobalY);
}

/********************************* MouseDown **********************************/
/*
*	A mouse down has occured within the view bounds.
*	If an enabled item is hit, redraw it as highlighted/on.
*	If another item is already highlighted, then draw it as unhighlighted/off.
*/
void XMenu::MouseDown(
	int16_t	inGlobalX,
	int16_t	inGlobalY)
{
	/*
	*	Finding the item hit isn't a simple division by the item height because
	*	separator items are not the same height as other items AND there's a
	*	top and bottom inset of mQuarterItemHeight.  This means that each item
	*	needs to be inspected.
	*
	*	Also, the item top (itemY) needs to be calculated for the same reason.
	*/
	int16_t		itemY = mY+mQuarterItemHeight;
	bool	noSelectedItem = mSelectedItem == nullptr;
	bool	redrawAllMenuItems = false;
	mSelectedItem = nullptr;
	/*
	*	If the click could be on an item...
	*/
	if (inGlobalY > itemY)
	{
		int16_t		nextItemY = 0;
		for (XMenuItem*	thisItem = mMenuItems; thisItem;
				thisItem = thisItem->mNextItem, itemY = nextItemY)
		{
			if (thisItem->IsSeparator())
			{
				nextItemY = itemY+mHalfItemHeight;
			} else
			{
				nextItemY = itemY+mItemHeight;
			}
			/*
			*	If there is a new selected item OR
			*	the click is below this item... 
			*/
			if (mSelectedItem ||
				inGlobalY > nextItemY)
			{
				/*
				*	If this item is the old selected item THEN
				*	redraw it as unhighlighted.
				*/
				if (thisItem->IsOn())
				{
					thisItem->SetState(XMenuItem::eOff);
					DrawItem(itemY, thisItem, true);
					/*
					*	If there is a new selected item THEN
					*	break, there is no reason to continue.
					*/
					if (mSelectedItem)
					{
						break;
					}
				}
				continue;
			}
			/*
			*	thisItem is the item clicked, and is tentatively the new
			*	selected item.
			*/
			mSelectedItem = thisItem;
			if (thisItem->IsOff())
			{
				switch(thisItem->GetFirstChar())
				{
					case '-':
						break;
					case '<':
					case '>':
						if (mMenuItemProvider)
						{
							redrawAllMenuItems = mMenuItemProvider->UpdateMenuItems(thisItem);
						}
						break;
					default:
						if (thisItem->IsEnabled())
						{
							thisItem->SetState(XMenuItem::eOn);
							DrawItem(itemY, thisItem);
						}
						break;
				}
				if (noSelectedItem)
				{
					break;	// There is no other item to unhighlight.
				}
			} else
			{
				break;	// There is no other item to unhighlight.
			}
		}
		if (mSelectedItem &&
			(!mSelectedItem->IsEnabled() ||
				mSelectedItem->IsSeparator() ||
				redrawAllMenuItems))
		{
			mSelectedItem = nullptr;
			if (redrawAllMenuItems)
			{
				itemY = mY+mQuarterItemHeight;
				for (XMenuItem*	thisItem = mMenuItems; thisItem; thisItem = thisItem->mNextItem)
				{
					itemY = DrawItem(itemY, thisItem);
				}
			}
		}
	}
}

/********************************** MouseUp ***********************************/
/*
*	A mouse up has occured within the view bounds.
*	The selected item is cleared if the mouse is no longer over it (provided
*	mAutoClearSelected is set.)
*
*	This routine doesn't do any drawing.
*/
void XMenu::MouseUp(
	int16_t	inGlobalX,
	int16_t	inGlobalY)
{
	int16_t		itemY = mY+mQuarterItemHeight;

	if (mSelectedItem &&
		inGlobalY > itemY)
	{
		XMenuItem*	thisItem = mMenuItems;
		for (; thisItem; thisItem = thisItem->mNextItem)
		{
			if (thisItem->IsSeparator())
			{
				itemY += mHalfItemHeight;
				continue;
			}
			itemY += mItemHeight;
			
			if (inGlobalY > itemY)
			{
				continue;
			}
			break;
		}
		if (thisItem != mSelectedItem &&
			mAutoClearSelected)
		{
			ClearSelectedItem();
		}
	} else if (mAutoClearSelected)
	{
		ClearSelectedItem();
	}
	if (mSelectedItem)
	{
		HandleChange(this, mSelectedItem->Tag());
	}
}

/**************************** SetMenuItemProvider *****************************/
void XMenu::SetMenuItemProvider(
	XMenuItemProvider*	inMenuItemProvider)
{
	mMenuItemProvider = inMenuItemProvider;
	if (mMenuItemProvider)
	{
		mMenuItemProvider->SetMenu(this);
	}
}

/******************************* InsertMenuItem *******************************/
/*
*	Should not be used when mMenuItemProvider isn't null
*/
void XMenu::InsertMenuItem(
	XMenuItem*	inMenuItem,	// inMenuItem->mNextItem must null
	uint16_t	inAfterTag)
{
	if (inMenuItem)
	{
		if (mMenuItems)
		{
			if (inAfterTag)
			{
				XMenuItem*	thisItem = mMenuItems;
				for (; thisItem; thisItem = thisItem->mNextItem)
				{
					if (thisItem->mTag != inAfterTag)
					{
						continue;
					}
					inMenuItem->mNextItem = thisItem->mNextItem;
					thisItem->mNextItem = inMenuItem;
					return;
				}
			}
			/*
			*	An item with the tag wasn't found or the tag value is zero...
			*	Insert as the first item.
			*/
			inMenuItem->mNextItem = mMenuItems;
		}
		mMenuItems = inMenuItem;
	}
}

/*************************** RemoveMenuItemWithTag ****************************/
/*
*	Returns the item removed or nullptr if not found.
*	Should not be used when mMenuItemProvider isn't null
*/
XMenuItem* XMenu::RemoveMenuItemWithTag(
	uint16_t	inTag)
{
	XMenuItem*	prevItem = nullptr;
	XMenuItem*	thisItem = mMenuItems;
	for (; thisItem; thisItem = thisItem->mNextItem)
	{
		if (thisItem->mTag != inTag)
		{
			prevItem = thisItem;
			continue;
		}
		if (prevItem)
		{
			prevItem->mNextItem = thisItem->mNextItem;
		} else
		{
			mMenuItems = thisItem->mNextItem;
		}
		thisItem->mNextItem = nullptr;
		break;
	}
	return(thisItem);
}

/***************************** GetWidestItemWidth *****************************/
/*
*	Returns the widest item width.
*/
uint16_t XMenu::GetWidestItemWidth(void)
{
	XFont*	xFont = mFont->GetXFont();
	xFont->SetFont(mFont);
	uint16_t	widest = 20;
	if (mMenuItemProvider == nullptr)
	{
		for (XMenuItem*	thisItem = mMenuItems; thisItem; thisItem = thisItem->mNextItem)
		{
			const char*	thisItemStr = thisItem->GetString();
			
			if (thisItemStr != nullptr && thisItemStr[0] != '-')
			{
				uint16_t	thisHeight, thisWidth;
				xFont->MeasureStr(thisItem->GetString(), thisHeight, thisWidth);
				if (widest < thisWidth)
				{
					widest = thisWidth;
				}
			}
		}
	} else
	{
		widest = mMenuItemProvider->GetWidestItemWidth(xFont);
	}
	return(widest);
}

/******************************** GetItemCount ********************************/
/*
*	Used by Show() to calculate the fixed height of the menu.
*	Note that if there's a mMenuItemProvider, this is NOT total number of items,
*	it's just the total number of items that can be displayed at one time.
*/
uint16_t XMenu::GetItemCount(
	uint16_t*	outNumSeparators) const
{
	uint16_t	itemCount = 0;
	uint16_t	numSeparators = 0;
	for (XMenuItem*	thisItem = mMenuItems; thisItem; thisItem = thisItem->mNextItem)
	{
		itemCount++;
		if (!thisItem->IsSeparator())
		{
			continue;
		}
		numSeparators++;
	}
	if (outNumSeparators)
	{
		*outNumSeparators = numSeparators;
	}
	return(itemCount);
}

/**************************** FindMenuItemWithTag *****************************/
/*
*	This doesn't currently work with XPopUpButtons with a XMenuItemProvider
*	because it only looks at the cached XMenuItems.
*/
XMenuItem* XMenu::FindMenuItemWithTag(
	uint16_t	inTag) const
{
	XMenuItem*	itemWithTag = nullptr;
	for (XMenuItem*	thisItem = mMenuItems; thisItem; thisItem = thisItem->mNextItem)
	{
		if (thisItem->Tag() != inTag)
		{
			continue;
		}
		itemWithTag = thisItem;
		break;
	}
	return(itemWithTag);
}

/****************************** GetSelectedItem *******************************/
XMenuItem* XMenu::GetSelectedItem(
	bool	inClearSelection)
{
	XMenuItem*	selectedItem = mSelectedItem;
	if (inClearSelection)
	{
		ClearSelectedItem();
	}
	return(selectedItem);
}

/**************************** SelectFirstMenuItem *****************************/
/*
*	Used by XPopUpButtons with a XMenuItemProvider to load the first item as
*	the selected item.
*
*	This should only be called during initialization.
*/
XMenuItem* XMenu::SelectFirstMenuItem(void)
{
	XMenuItem*	menuItem = nullptr;
	if (mMenuItemProvider)
	{
		XFont*	xFont = mFont->GetXFont();
		xFont->SetFont(mFont);
		mMenuItemProvider->SetMenu(this);
		mMenuItemProvider->GetWidestItemWidth(xFont);	// Updates the provider item count
		mMenuItemProvider->UpdateMenuItems();	// Loads the item string cache
		SelectMenuItem(mMenuItems);
		menuItem = mMenuItems;
	}
	return(menuItem);
}

/******************************* SelectMenuItem *******************************/
void XMenu::SelectMenuItem(
	XMenuItem*	inMenuItem)
{
	if (mSelectedItem != inMenuItem)
	{
		ClearSelectedItem();
	}
	if (inMenuItem)
	{
		mSelectedItem = inMenuItem;
		inMenuItem->SetState(XMenuItem::eOn);
	}
}

/***************************** ClearSelectedItem ******************************/
void XMenu::ClearSelectedItem(void)
{
	/*
	*	Clear the last selected item, if any.
	*/
	if (mSelectedItem &&
		mSelectedItem->GetState() == XMenuItem::eOn)
	{
		mSelectedItem->SetState(XMenuItem::eOff);
		mSelectedItem = nullptr;
	}
}

