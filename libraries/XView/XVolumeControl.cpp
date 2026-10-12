/*
*	XVolumeControl.cpp, Copyright Jonathan Mackey 2026
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

#include "XVolumeControl.h"
#include "XRootView.h"
#include "DisplayController.h"
#ifdef __MACH__
	#define map DisplayController::map
	#include <iostream>
#endif

/*********************************** XVolumeControl ***********************************/
XVolumeControl::XVolumeControl(
	int16_t					inX,
	int16_t					inY,
	uint16_t				inWidth,
	uint16_t				inHeight,
	uint16_t				inTag,
	XView*					inNextView,
	int32_t					inValue,
	int32_t					inMaxValue,
	int32_t					inMinValue,
	uint16_t				inBGColor,
	uint16_t				inBarColor,
	uint16_t				inFrameColor,
	bool					inEnabled)
	: XProgressBar(inX, inY, inWidth, inHeight, inTag, inNextView,
		inValue, inMaxValue, inMinValue, inBGColor,
		inBarColor, inFrameColor, inEnabled)
{
	mBarLeftOffset = (kLeftSpeakerWidth+kSpeakerBarGap);
	mBarWidth = mWidth - (kLeftSpeakerWidth + kRightSpeakerWidth
														+ (kSpeakerBarGap*2));
}

/********************************** DrawSelf **********************************/
void XVolumeControl::DrawSelf(void)
{
	DrawBar();
	DisplayController*	display = XRootView::GetInstance()->GetDisplay();
	if (display)
	{
		int16_t	x = 0;
		int16_t	y = 0;
		LocalToGlobal(x, y);

		int16_t		ctrY = y + (mHeight/2);
		display->SetFGColor(mFrameColor);
		display->SetBGColor(mBGColor);
		DrawSpeaker(x, ctrY, display, mFrameColor);
		DrawSpeaker(x+mWidth-kRightSpeakerWidth, ctrY, display,
														mFrameColor, true);
	}
}

/******************************** DrawSpeaker *********************************/
/*
*	Speaker dims:
*		Height = 18px
*		Width = 23px wide with loudness bars and 10px wide without.
*/
void XVolumeControl::DrawSpeaker(
	int16_t				inX,	// Left
	int16_t				inY,	// Center
	DisplayController*	inDisplay,
	uint16_t			inFillColor,
	bool				inWithLoudnessBars)
{
	inDisplay->SetFGColor(inFillColor);
	uint8_t tintVal = inDisplay->DrawInnerColorCircle(inX+2, inY-2, 2, 0, inFillColor, DisplayController::eWestHalf, 2, 4);
	uint16_t	tintedColor = inDisplay->Calc565Color(tintVal);
	inDisplay->SetFGColor(tintedColor);
	inDisplay->FillRect(inX, inY-2, 1, 4, tintedColor);	// Vertical line
	
	inDisplay->DrawLine(inX+6, inY-5, inX+9, inY-8, 1);
	inDisplay->DrawLine(inX+6, inY+4, inX+9, inY+7, 1);
	
	inDisplay->FillRect(inX+9, inY-7, 1, 14, tintedColor);	// Vertical line


	inDisplay->DrawLine(inX+2, inY-4, inX+5, inY-4, 1);
	inDisplay->DrawLine(inX+2, inY+3, inX+5, inY+3, 1);
	
	inDisplay->SetFGColor(inFillColor);
	//							 x,	 y,     wt, ht, xS, yS, wS,hS,nR
	inDisplay->FillSteppedRect(inX+8, inY-6, 1, 12, -1, 1, 0, -2, 3, inFillColor);
	inDisplay->FillRect(inX+1, inY-3, 5, 6, inFillColor);
	if (inWithLoudnessBars)
	{
		inX += 2;
		inDisplay->DrawInnerColorCircle(inX, inY, 5, 1, inFillColor, DisplayController::eEQuarter, 8);
		inDisplay->DrawInnerColorCircle(inX, inY, 9, 1, inFillColor, DisplayController::eEQuarter, 8);
		inDisplay->DrawInnerColorCircle(inX, inY, 13, 1, inFillColor, DisplayController::eEQuarter, 8);
	}
}

/********************************* MouseDown **********************************/
void XVolumeControl::MouseDown(
	int16_t	inGlobalX,
	int16_t	inGlobalY)
{
	int16_t	mouseX = inGlobalX;
	int16_t	mouseY = inGlobalY;
	GlobalToLocal(mouseX, mouseY);
	int16_t	halfHt = mHeight/2;
	
	/*
	*	If the click is inside the bar...
	*/
	if (mouseY >= (halfHt-kBarEndRadius) &&
		mouseY < (halfHt+kBarEndRadius) &&
		mouseX >= (kLeftSpeakerWidth+kSpeakerBarGap) &&
		mouseX < (mWidth-kRightSpeakerWidth-kSpeakerBarGap))
	{
		int16_t	barWidth = mWidth - kBarRadius;
		//if (mHasSpeakers)
		{
			mouseX -= (kLeftSpeakerWidth+kSpeakerBarGap);
			barWidth -= (kLeftSpeakerWidth + kRightSpeakerWidth
												+ (kSpeakerBarGap*2));
		}
		SetValue(map(mouseX, kBarRadius, barWidth, mMinValue, mMaxValue), true);
	} else// if (mHasSpeakers)
	{
		/*
		*	Decrease the volume if clicking on the left speaker icon
		*/
		if (mouseX < (kLeftSpeakerWidth+kSpeakerBarGap))
		{
			if (mValue > mMinValue)
			{
				SetValue(mValue-1, true);
			}
		/*
		*	Increase the volume if clicking on the right speaker icon
		*/
		} else if (mouseX > (mWidth-kRightSpeakerWidth-kSpeakerBarGap))
		{
			if (mValue < mMaxValue)
			{
				SetValue(mValue+1, true);
			}
		}
	}
}

