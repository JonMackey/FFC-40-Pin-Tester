/*
*	XColoredView.cpp, Copyright Jonathan Mackey 2023
*
*	Class that draws/fills the display background.
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

#include "XRootView.h"
#include "XColoredView.h"
#include "DisplayController.h"

/*********************************** XColoredView ***********************************/
XColoredView::XColoredView(
	int16_t		inX,
	int16_t		inY,
	uint16_t	inWidth,
	uint16_t	inHeight,
	uint16_t	inTag,
	XView*		inNextView,
	XView*		inSubViews,
	XView*		inSuperView,
	bool		inVisible,
	bool		inEnabled,
	uint16_t	inColor)
	: XView(inX, inY, inWidth, inHeight, inTag, inNextView, inSubViews,
		inSuperView, inVisible, inEnabled),
	  mColor(inColor)
{
}

//#define DebugDrawUpdates	1
#ifdef DebugDrawUpdates
static const uint16_t	kColor565[] = {0x31DF, 0x4665, 0xFBC0, 0xFFE0, 0xFA1F,
										0x07DF, 0x43D5, 0x9112, 0x049F, 0xFD60,
										0xCE79};
static uint16_t	sColorIndex = 0;
#endif
/************************************ Draw ************************************/
/*
*	inX and inY are local.
*/
void XColoredView::Draw(
	int16_t		inX,
	int16_t		inY,
	uint16_t	inWidth,
	uint16_t	inHeight)
{
	/*
	*	If this view is visible AND
	*	the area to be drawn intersects this view's bounds...
	*/
	if (mVisible &&
		!(mX + mWidth <= inX ||
		  mX >= inX + inWidth ||
		  mY + mHeight <= inY ||
		  mY >= inY + inHeight))
	{
		DisplayController*	display = XRootView::GetInstance()->GetDisplay();
		if (display)
		{
			
#ifndef DebugDrawUpdates
			display->FillRect(inX, inY, inWidth, inHeight, mColor);
#else
			display->FillRect(inX, inY, inWidth, inHeight, kColor565[sColorIndex]);
			sColorIndex = sColorIndex < 10 ? (sColorIndex+1) : 0;
#endif
			DrawSelf();
			if (mSubViews)
			{
				mSubViews->Draw(inX, inY, inWidth, inHeight);
			}
		}
	}
	if (mNextView)
	{
		mNextView->Draw(inX, inY, inWidth, inHeight);
	}


}


