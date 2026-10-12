/*
*	XVolumeControl.h, Copyright Jonathan Mackey 2026
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
#ifndef XVolumeControl_h
#define XVolumeControl_h

#include "XProgressBar.h"

class XVolumeControl : public XProgressBar
{
public:
							XVolumeControl(
								int16_t					inX,
								int16_t					inY,
								uint16_t				inWidth,
								uint16_t				inHeight,	// 45
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
	virtual void			MouseDown(
								int16_t					inGlobalX,
								int16_t					inGlobalY);
protected:
	const int16_t	kSpeakerBarGap = 10;
	const int16_t	kLeftSpeakerWidth = 10;
	const int16_t	kRightSpeakerWidth = 23;
	
	static void				DrawSpeaker(
								int16_t					inX,
								int16_t					inY,
								DisplayController*		inDisplay,
								uint16_t				inFillColor,
								bool					inWithLoudnessBars = false);

};

#endif // XVolumeControl_h
