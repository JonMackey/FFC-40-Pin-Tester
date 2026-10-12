/*
*	XLabel.h, Copyright Jonathan Mackey 2023-2026
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
#ifndef XLabel_h
#define XLabel_h

#include "XMultiLineLabel.h"
#include "XFont.h"

class XLabel : public XMultiLineLabel
{
public:
							XLabel(
								int16_t					inX,
								int16_t					inY,
								uint16_t				inWidth,
								uint16_t				inHeight,
								uint16_t				inTag,
								XView*					inNextView = nullptr,
								const char*				inString = nullptr,
								XFont::Font*			inFont = nullptr,
								XView*					inSuperView = nullptr,
								uint16_t				inTextColor = 0xFFFF,
								uint16_t				inBGColor = 0,
								XFont::ETextAlignment	inTextAlignment = XFont::eAlignLeft,
								bool					inEnabled = true);
	virtual void			DrawSelf(void);
	XFont::ETextAlignment	GetTextAlignment(void) const
								{return(mTextAlignment);}
	void					SetTextAlignment(
								XFont::ETextAlignment	inTextAlignment)
								{mTextAlignment = inTextAlignment;}
	void					SetEraseUnusedAreaAfterDraw(
								bool					inEraseUnusedAreaAfterDraw=true)
								{mEraseUnusedAreaAfterDraw = inEraseUnusedAreaAfterDraw;}
protected:
	XFont::ETextAlignment	mTextAlignment;
	bool					mEraseUnusedAreaAfterDraw;
};

#endif // XLabel_h
