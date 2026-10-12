/*
*	XProgressBar.cpp, Copyright Jonathan Mackey 2026
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

#include "XProgressBar.h"
#include "XRootView.h"
#include "DisplayController.h"
#ifdef __MACH__
	#define map DisplayController::map
	#include <iostream>
#endif

/*********************************** XProgressBar ***********************************/
XProgressBar::XProgressBar(
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
	: XView(inX, inY, inWidth, inHeight, inTag, inNextView, nullptr,
		nullptr, true, inEnabled), mValue(inValue), mMaxValue(inMaxValue),
		mMinValue(inMinValue), mBGColor(inBGColor),
		mBarColor(inBarColor), mFrameColor(inFrameColor),
		mBarLeftOffset(0), mBarWidth(inWidth)
{
}

/********************************** DrawSelf **********************************/
void XProgressBar::DrawSelf(void)
{
	DrawBar();
}

/******************************** DrawBar *********************************/
void XProgressBar::DrawBar(void)
{
	DisplayController*	display = XRootView::GetInstance()->GetDisplay();
	if (display)
	{
		int16_t	x = 0;
		int16_t	y = 0;
		LocalToGlobal(x, y);

		int16_t		ctrY = y + (mHeight/2);
		uint16_t	color = mEnabled ? mBarColor :
						DisplayController::Calc565Color(mBarColor, 0, 184);
		display->SetFGColor(mFrameColor);
		display->SetBGColor(mBGColor);
		
		int16_t	barWidth = mBarWidth;
		int16_t	barLeft = x + mBarLeftOffset;
		
		display->DrawRoundedFrame(barLeft, ctrY-kBarEndRadius,
								  barWidth, (kBarEndRadius*2),
								  kBarEndRadius, kFrameThickness, mFrameColor);
		display->SetBGColor(mFrameColor);
		display->SetFGColor(color);

		/*
		*	If value isn't the minimum THEN
		*	draw the bar within the frame
		*/
		if (mValue > mMinValue)
		{
			uint8_t	octants = mValue < mMaxValue ? DisplayController::eWestHalf :
												DisplayController::eFullCircle;
			barWidth -= (kBarEndRadius*2);
			barLeft += kBarEndRadius;
			display->DrawInnerColorCircle(barLeft, ctrY, kBarRadius, 0,
									mFrameColor, octants,
									barWidth);	// barWidth is only used when
												// the value is 100%
			if (mValue > (mMinValue+1))
			{
				display->FillRect(barLeft, ctrY - (kBarThickness/2),
								map(mValue, mMinValue, mMaxValue, 1, barWidth),
								kBarThickness, color);
			}
		}
	}
}

/********************************** SetValue **********************************/
bool XProgressBar::SetValue(
	int32_t		inValue,
	bool		inUpdate)
{
	if (inValue < mMinValue)
	{
		inValue = mMinValue;
	} else if (inValue > mMaxValue)
	{
		inValue = mMaxValue;
	}
	bool	valueChanged = inValue != mValue;
	if (valueChanged)
	{
		mValue = inValue;
		if (inUpdate)
		{
			DrawBar();
			// Only notify of change when updating the UI
			HandleChange(this, eValueChanged);
		}
	}
	return(valueChanged);
}

