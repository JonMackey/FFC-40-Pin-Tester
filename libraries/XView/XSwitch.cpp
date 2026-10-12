/*
*	XSwitch.cpp, Copyright Jonathan Mackey 2026
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

#include "XSwitch.h"
#include "DisplayController.h"
#include "XRootView.h"

static const uint16_t	kSwitchColor = 0xFFFF;
static const uint16_t	kOnSwitchBGColor = 0xEBA6;
static const uint16_t	kOnDisSwitchBGColor = 0xF5F3;
static const uint16_t	kOffSwitchBGColor = 0xE6FB;
static const uint16_t	kOffDisSwitchBGColor = 0xEF5C;
static const int16_t	kInnerSwitchGap = 2;
static const int16_t	kInnerSwitchRad = 9;
static const int16_t	kInnerSwitchHeight = 20;
static const int16_t	kInnerSwitchWidth = 32;
static const int16_t	kSwitchRad = 10;
static const int16_t	kSwitchHeight = 24;
static const int16_t	kSwitchWidth = 54;

/******************************** XSwitch *********************************/
XSwitch::XSwitch(
	int16_t			inX,
	int16_t			inY,
	uint16_t		inTag,
	XView*			inNextView,
	uint16_t		inFGColor,
	uint16_t		inBGColor)
	: XControl(inX, inY, kSwitchWidth, kSwitchHeight, inTag, inNextView),
		mFGColor(inFGColor), mBGColor(inBGColor)
{
}

/********************************** DrawSelf **********************************/
void XSwitch::DrawSelf(void)
{
	if (mVisible)
	{
		DisplayController*	display = XRootView::GetInstance()->GetDisplay();
		uint16_t	fieldColor;
		int16_t	x = 0;
		int16_t	y = 0;
		LocalToGlobal(x, y);
		if (mEnabled)
		{
			fieldColor = mState ? kOnSwitchBGColor : kOffSwitchBGColor;
		} else
		{
			fieldColor = mState ? kOnDisSwitchBGColor : kOffDisSwitchBGColor;
		}
		display->SetFGColor(fieldColor);
		display->SetBGColor(mBGColor);
		display->DrawRoundedFrame(x, y, kSwitchWidth, kSwitchHeight, 
									kSwitchRad, 1, fieldColor);
		display->SetFGColor(kSwitchColor);
		display->SetBGColor(fieldColor);
		int16_t	xPos = x + (mState == eOff ? kInnerSwitchGap : mWidth - kInnerSwitchWidth -kInnerSwitchGap);
		display->DrawRoundedFrame(xPos, y+kInnerSwitchGap, kInnerSwitchWidth, kInnerSwitchHeight, 
									kInnerSwitchRad, 1, kSwitchColor);
	}
}

/********************************* MouseDown **********************************/
void XSwitch::MouseDown(
	int16_t	inGlobalX,
	int16_t	inGlobalY)
{
	SetState(mState == eOn ? eOff : eOn);
}
