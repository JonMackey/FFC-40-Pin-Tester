/*
*	XPopUpButton.h, Copyright Jonathan Mackey 2023
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
#ifndef XPopUpButton_h
#define XPopUpButton_h

#include "XMenuButton.h"

class XPopUpButton : public XMenuButton
{
public:
	enum EPopUpSize
	{
		eLargePopUpSize,	// 20x20 icon
		eNormalPopUpSize,	// 16x16 icon
		eSmallPopUpSize		// 12x12 icon
	};
							XPopUpButton(
								int16_t					inX,
								int16_t					inY,
								uint16_t				inWidth,
								uint16_t				inHeight,	// Pass 0 to autosize height based on font
								uint16_t				inTag,
								XMenu*					inMenu,
								XView*					inNextView = nullptr,
								XFont::Font*			inFont = nullptr,
								EPopUpSize				inPopUpSize = eLargePopUpSize,
								uint16_t				inFGColor = XFont::eWhite,
								uint16_t				inBGColor =  XFont::eBlack,
								char*					inAltStrStore = nullptr);
	virtual void			DrawSelf(void);
	virtual void			MouseDown(
								int16_t					inGlobalX,
								int16_t					inGlobalY);
	virtual void			MouseUp(
								int16_t					inGlobalX,
								int16_t					inGlobalY);
	void					SelectMenuItem(
								uint16_t				inTag,
								bool					inUpdate = false);
							/*
							*	The value returned is only valid after a
							*	selection has been made.
							*/
//	uint16_t				SelectedMenuItemTag(void) const
//								{return(mLastMenuItemTag);}
	void					SelectFirstMenuItem(void);
	void					SetSelectedItemString(
								const char*				inString,
								bool					inUpdate = false);
	void					ClearLastMenuItemTag(void)
								{mLastMenuItemTag = 0xFFFF;}
	bool					ClearItemStringIfSelected(
								const char*				inString,
								bool					inUpdate = false);								
	void					ClearSelectedItemString(
								bool					inUpdate = false);
	bool					ChangeItemStringIfSelected(
								const char*				inFromStr,
								const char*				inToStr,
								bool					inUpdate = false);
	const char*				GetSelectedItemString(void) const
								{return(mLabel.GetString());}
	uint32_t				GetSelectedItemAs4CharCode(void) const;
	bool					HasSelection(void) const;
	enum {eMenuItemTagChanged = 99};
protected:
	EPopUpSize	mPopUpSize;
	char*		mAltStrStore;
	int16_t		mOffsetForShowMenu;
	bool		mIgnoreNextMouseUp;
	uint16_t	mLastMenuItemTag;

	void					SelectMenuItem(
								XMenuItem*				inMenuItem,
								bool					inUpdate = false);
};

#endif // XPopUpButton_h
