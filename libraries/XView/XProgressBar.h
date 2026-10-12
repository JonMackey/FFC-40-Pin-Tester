/*
*	XProgressBar.h, Copyright Jonathan Mackey 2026
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
#ifndef XProgressBar_h
#define XProgressBar_h

#include "XView.h"

class DisplayController;

class XProgressBar : public XView
{
public:
							XProgressBar(
								int16_t					inX,
								int16_t					inY,
								uint16_t				inWidth,
								uint16_t				inHeight,
								uint16_t				inTag,
								XView*					inNextView = nullptr,
								int32_t					inValue = 0,
								int32_t					inMaxValue = 100,
								int32_t					inMinValue = 0,
								uint16_t				inBGColor = 0,
								uint16_t				inBarColor = 0xEBA6,
								uint16_t				inFrameColor = 0xCE79,
								bool					inEnabled = true);
	virtual void			DrawSelf(void);
	uint32_t				GetValue(void) const
								{return(mValue);}
	bool					SetValue(
								int32_t					inValue,
								bool					inUpdate = false);
	enum
	{
		eValueChanged = 1
	};
protected:
	uint16_t		mBarColor;
	uint16_t		mFrameColor;
	uint16_t		mBGColor;
	int32_t			mValue;
	int32_t			mMinValue;
	int32_t			mMaxValue;
	uint16_t		mBarLeftOffset;	// To support subclass
	uint16_t		mBarWidth;		// To support subclass
	const int16_t	kFrameThickness = 1;
	const int16_t	kBarEndRadius = 5;
	const int16_t	kBarRadius = 4;
	const int16_t	kBarThickness = 8;
	
	void					DrawBar(void);

};

#endif // XProgressBar_h
