/*
*	XMenuItemProvider.h, Copyright Jonathan Mackey 2026
*
*	XMenuItemProvider is an abstract class that provides menu item strings on
*	demand.  For instance there is an XSDMenuItemProvider that will provide a
*	filtered list of file names on an SD card's root directory.
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
#ifndef XMenuItemProvider_h
#define XMenuItemProvider_h

#include "PlatformDefs.h"

class XMenu;
class XMenuItem;
class XFont;

class XMenuItemProvider
{
public:
							XMenuItemProvider(void);
	void					SetMenu(
								XMenu*					inMenu)
								{mMenu = inMenu;}
	virtual uint16_t		GetWidestItemWidth(
								XFont*					inXFont) = 0;
	bool					UpdateMenuItems(
								XMenuItem*				inItemSelected = nullptr);
protected:
	XMenu*		mMenu;
	XMenuItem*	mUnusedMenuItems;
	uint16_t	mItemCount;			// Number of item strings supplied by the provider.
	uint16_t	mItemIncrement;		// Increment per up/down arrow press.
	uint16_t	mFirstItemIndex;	// First item string displayed in menu
	uint16_t	mWidestItemWidth;
	static const char	kUpArrowStr[];
	static const char	kDownArrowStr[];

	virtual const char*		GetNthItemStr(
								uint16_t				inItemIndex) = 0;
	void					UpdateMenuItemList(void);
};
#endif // XMenuItemProvider_h
