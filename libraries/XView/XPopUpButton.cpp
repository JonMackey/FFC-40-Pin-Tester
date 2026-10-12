/*
*	XPopUpButton.cpp, Copyright Jonathan Mackey 2023
*	A popup menu button.
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

#include "XPopUpButton.h"
#include "XRootView.h"
#include "DisplayController.h"
#include "XMenuItem.h"
#ifdef __MACH__
#include <stdio.h>
#endif

static const struct SPopUpDim
{
	int16_t	size;
	int16_t	gap;
	int16_t	radius;
	int16_t	a[3];	// arrow x0, y0, y1 (x1 = 0)
} kPopUpDim[] = {
	{20, 5, 5, {2,2,4}},
	{16, 4, 4, {2,2,4}},
	{12, 4, 3, {1,1,2}}
};

/******************************** XPopUpButton ********************************/
XPopUpButton::XPopUpButton(
	int16_t							inX,
	int16_t							inY,
	uint16_t						inWidth,
	uint16_t						inHeight,
	uint16_t						inTag,
	XMenu*							inMenu,
	XView*							inNextView,
	XFont::Font*					inFont,
	XPopUpButton::EPopUpSize		inPopUpSize,
	uint16_t						inFGColor,
	uint16_t						inBGColor,
	char*							inAltStrStore)
	: XMenuButton(inX, inY, inWidth, inHeight, inTag, inMenu, inNextView, nullptr, inFont,
	  inFGColor, inBGColor), mPopUpSize(inPopUpSize), mIgnoreNextMouseUp(false),
	  mOffsetForShowMenu(0), mAltStrStore(inAltStrStore), mLastMenuItemTag(0xFFFF)
{
	XFont*	xFont = mLabel.MakeFontCurrent();
	if (xFont)
	{
		const SPopUpDim& popUpDim = kPopUpDim[inPopUpSize];
		mLabel.MoveBy(popUpDim.gap, 0);
		mLabel.AdjustSize(-popUpDim.size-(popUpDim.gap*3), 0);
		mOffsetForShowMenu = xFont->GetFontHeader().height - popUpDim.gap;
	}
	if (inMenu)
	{
		inMenu->SetSuperViewAnchor(XMenu::eAnchorCentered);
		inMenu->SetAutoClearSelected(false);
		if (inMenu->GetMenuItems())
		{
			/*
			*	For menus with a dynamic menu item provider, calling
			*	SelectMenuItem here does nothing.  For menus with a dynamic menu
			*	item provider call SelectFirstMenuItem after the provider has
			*	been initialized.
			*/
			SelectMenuItem(inMenu->GetMenuItems());
		}
	}
}

/********************************** DrawSelf **********************************/
void XPopUpButton::DrawSelf(void)
{
	if (mVisible)
	{
		XFont*	xFont = mLabel.MakeFontCurrent();
		if (xFont)
		{
			DisplayController*	display = xFont->GetDisplay();
			uint16_t	iconColor, arrowColor;
			int16_t	x = 0;
			int16_t	y = 0;
			LocalToGlobal(x, y);
			const SPopUpDim& popUpDim = kPopUpDim[mPopUpSize];
			if (mEnabled)
			{
				iconColor = 0xFC01;
				arrowColor = 0xFFFF;
			} else
			{
				iconColor = 0xFFDF;
				arrowColor = 0xBDF7;
			}
			display->SetFGColor(0xB5B6);
			display->SetBGColor(mBGColor);
			display->DrawRoundedRect(x, y, mWidth, mHeight, mHeight/4, 255, true);
			display->SetFGColor(mFGColor);
			uint16_t	fillColor = display->DrawRoundedRect(x+1, y+1, mWidth-2, mHeight-2, mHeight/4, mState ? 200 : 255);
			int16_t	adjY = ((int16_t)mHeight - popUpDim.size)/2;
			if (adjY < 0)
			{
				adjY = 0;
			}
			y += adjY;
			display->SetFGColor(iconColor);
			display->SetBGColor(fillColor);
			x += (mWidth-popUpDim.size-popUpDim.gap);
			int16_t	radius = popUpDim.size/4;
			display->DrawRoundedRect(x, y, popUpDim.size, popUpDim.size, radius, 255);

			int16_t	halfWidth = popUpDim.size/2;
			display->SetFGColor(arrowColor);
			display->SetBGColor(iconColor);

			x+=(halfWidth-1);
			y+=(halfWidth-1);
			display->DrawLine(x-popUpDim.a[0], y-popUpDim.a[1], x, y-popUpDim.a[2], 2);
			x++;
			display->DrawLine(x+popUpDim.a[0], y-popUpDim.a[1], x, y-popUpDim.a[2], 2);
			display->FillRect(x-1, y-popUpDim.a[2], 1, 1, arrowColor);
			y++;
			display->DrawLine(x+popUpDim.a[0], y+popUpDim.a[1], x, y+popUpDim.a[2], 2);
			x--;
			display->DrawLine(x-popUpDim.a[0], y+popUpDim.a[1], x, y+popUpDim.a[2], 2);
			display->FillRect(x+1, y+popUpDim.a[2], 1, 1, arrowColor);

			mLabel.SetBGColor(fillColor);
			mLabel.DrawSelf();
		}
	}
}

/********************************* MouseDown **********************************/
void XPopUpButton::MouseDown(
	int16_t	inGlobalX,
	int16_t	inGlobalY)
{
	if (mState == eOn)
	{
		int16_t	localX = inGlobalX;
		int16_t	localY = inGlobalY;
		GlobalToLocal(localX, localY);
		bool	menuHit = mMenu->HitTest(inGlobalX, inGlobalY);
		/*
		*	If the anchoring menu button was hit THEN
		*	close the menu.
		*/
		if (!menuHit && XView::HitSelf(localX, localY))
		{
			SetState(eOff, false);
		} else
		{
			/*
			*	If the mouse down was within one of the menu items THEN
			*	highlight the menu item.
			*/
			if (menuHit)
			{
				mMenu->MouseDown(inGlobalX, inGlobalY);
				// If a separator item or a disabled item was hit, ignore it.
				mIgnoreNextMouseUp = mMenu->GetSelectedItem(false) == nullptr;
			} else
			{
				SetState(eOff, false);
			}
		}
		if (mState == eOff)
		{
			DrawSelf();
			XRootView::GetInstance()->SetModalView(mSavedModalView);
			mSavedModalView = nullptr;
			mMenu->Hide();
		}
	/*
	*	Else, prepare to display the list of menu items.
	*	
	*/
	} else
	{
		SetState(eOn);
		/*
		*	mOffsetForShowMenu is a hack to get the menu text to align with
		*	this button's mLabel text.
		*/
		MoveBy(-mOffsetForShowMenu, 0);
		mMenu->Show();
		MoveBy(mOffsetForShowMenu, 0);
		mIgnoreNextMouseUp = true;
		mSavedModalView = XRootView::GetInstance()->ModalView();
		XRootView::GetInstance()->SetModalView(this);
	}
}

/********************************** MouseUp ***********************************/
void XPopUpButton::MouseUp(
	int16_t	inGlobalX,
	int16_t	inGlobalY)
{
	if (mIgnoreNextMouseUp)
	{
		mIgnoreNextMouseUp = false;
	} else
	{
		/*
		*	If the mouse up didn't occur over the selected item THEN
		*	clear the selection.
		*/
		mMenu->MouseUp(inGlobalX, inGlobalY);
		XMenuItem*	selectedItem = mMenu->GetSelectedItem(false);
		if (selectedItem)
		{
			SelectMenuItem(selectedItem);
			SetState(eOff);
			if (XRootView::GetInstance()->ModalView() == this)
			{
				XRootView::GetInstance()->SetModalView(mSavedModalView);
				mSavedModalView = nullptr;
			}
			mMenu->Hide();
			if (mLastMenuItemTag != selectedItem->Tag())
			{
				mLastMenuItemTag = selectedItem->Tag();
				HandleChange(this, eMenuItemTagChanged);
			}
		}
	}
}

/**************************** SelectFirstMenuItem *****************************/
/*
*	Used by menus with a XMenuItemProvider to display the first item as the
*	selected item.  When this routine isn't called, the popup menu appears
*	blank.
*
*	This should only be called during initialization.
*/
void XPopUpButton::SelectFirstMenuItem(void)
{
	if (mMenu)
	{
		XMenuItem*	menuItem = mMenu->SelectFirstMenuItem();
		if (menuItem)
		{
			SetSelectedItemString(menuItem->GetString());
		}
	}
}

/******************************* SelectMenuItem *******************************/
void XPopUpButton::SelectMenuItem(
	uint16_t	inTag,
	bool		inUpdate)
{
	SelectMenuItem(mMenu->FindMenuItemWithTag(inTag), inUpdate);
}

/******************************* SelectMenuItem *******************************/
void XPopUpButton::SelectMenuItem(
	XMenuItem*	inMenuItem,
	bool		inUpdate)
{
	mMenu->SelectMenuItem(inMenuItem);
	SetSelectedItemString(inMenuItem ? inMenuItem->GetString() : nullptr, inUpdate);
}

/******************************** HasSelection ********************************/
bool XPopUpButton::HasSelection(void) const
{
	return(mLabel.GetString() != nullptr && mLabel.GetString()[0] != 0);
}

/**************************** SetSelectedItemString ***************************/
/*
*	Added to support menu item string caching.	XCachedMenuItemProvider reuses
*	the cache entries as you scroll the item list, so the XPopUpButton label
*	can't use the menu item for long term storage of the selected item string.
*/
void XPopUpButton::SetSelectedItemString(
	const char*	inString,
	bool		inUpdate)
{
	const char*	curStr = mLabel.GetString();
	bool strChanged = false;
	if (inString != nullptr)
	{
		if (curStr != nullptr)
		{
			strChanged = strcmp(curStr, inString) != 0;
		} else
		{
			strChanged = inString[0] != 0;
		}
	} else if (curStr != nullptr)
	{
		strChanged = curStr[0] != 0;
	}
	if (strChanged)
	{
		mLastMenuItemTag = 0xFFFF;
		if (mAltStrStore)
		{
			if (inString != nullptr)
			{
				strcpy(mAltStrStore, inString);
			} else
			{
				mAltStrStore[0] = 0;
			}
			mLabel.SetString(mAltStrStore);
		} else
		{
			mLabel.SetString(inString);
		}
		if (inUpdate)
		{
			DrawSelf();
		}
		if (mMenu)
		{
			/*
			*	If the old selected item isn't cleared, the old selected item text
			*	may appear when drawing after the menu is displayed but no new item
			*	is selected.
			*/
			mMenu->ClearSelectedItem();
		}
	}
}

/************************** ClearSelectedItemString ***************************/
void XPopUpButton::ClearSelectedItemString(
	bool		inUpdate)
{
	SetSelectedItemString("", inUpdate);
}

/************************** ClearItemStringIfSelected *************************/
bool XPopUpButton::ClearItemStringIfSelected(
	const char*	inString,
	bool		inUpdate)
{
	bool	cleared = inString &&
						mLabel.GetString() &&
							strcmp(mLabel.GetString(), inString) == 0;
	if (cleared)
	{
		ClearSelectedItemString(inUpdate);
	}
	return(cleared);
}

/************************* ChangeItemStringIfSelected *************************/
bool XPopUpButton::ChangeItemStringIfSelected(
	const char*	inFromStr,
	const char*	inToStr,
	bool		inUpdate)
{
	bool	changed = inFromStr && inToStr &&
							mLabel.GetString() &&
								strcmp(mLabel.GetString(), inFromStr) == 0;
	if (changed)
	{
		SetSelectedItemString(inToStr, inUpdate);
	}
	return(changed);
}

#ifndef Swap32
#define Swap32(data) ((((data ^ (data >> 16 | (data << 16))) & 0xFF00FFFF) >> 8) ^ (data >> 8 | data << 24))
#endif
/************************* GetSelectedItemAs4CharCode *************************/
/*
*	Note that the selected item string may contain less than four characters.
*	Any missing characters are interpreted as nulls.
*	e.g. the string "G" would be converted as 'G', 0, 0, 0.  The resulting value
*	would be 0x47 00 00 00 or 0x47000000.
*/
uint32_t XPopUpButton::GetSelectedItemAs4CharCode(void) const
{
	uint32_t	value = 0;
	const char*	itemStr = mLabel.GetString();
	uint32_t	itemStrLen = itemStr ? (uint32_t)strlen(itemStr) : 0;
	if (itemStrLen > 0 &&
		itemStrLen <= 4)
	{
		/*
		*	Because the itemStrLen may be less than 4, Swap32 can't be used.
		*	The itemStr needs to be reversed and copied to a buffer padded
		*	with zeros.
		*/
		char			valueStr[4] = {0};
		char*			strPtr = &valueStr[3];
		
		for (uint8_t thisChar = *(itemStr++); thisChar; thisChar = *(itemStr++))
		{
			*(strPtr--) = thisChar;
		}
		
		value = *(const uint32_t*)valueStr;
	}
	return(value);
}
