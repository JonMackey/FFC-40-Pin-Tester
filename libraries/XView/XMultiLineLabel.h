/*
*	XMultiLineLabel.h, Copyright Jonathan Mackey 2026
*	Base class for a text label.
*	Similar to XLabel only text will wrap rather than be truncated.
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
#ifndef XMultiLineLabel_h
#define XMultiLineLabel_h

#include "XView.h"
#include "XFont.h"

class XMultiLineLabel : public XView
{
public:
							XMultiLineLabel(
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
								bool					inEnabled = true);
	virtual void			DrawSelf(void);
	XFont*					MakeFontCurrent(void);
	inline XFont::Font*		GetFont(void)
								{return(mFont);}
	void					SetFont(
								XFont::Font*			inFont)
								{mFont = inFont;}
	inline uint16_t			GetTextColor(void) const
								{return(mTextColor);}
	void					SetTextColor(
								uint16_t				inTextColor)
								{mTextColor = inTextColor;}
	inline uint16_t			GetBGColor(void) const
								{return(mBGColor);}
	void					SetBGColor(
								uint16_t				inBGColor)
								{mBGColor = inBGColor;}
	inline const char*		GetString(void) const
								{return(mString);}
	bool					HasString(void) const
								{return(mString != 0);}
	void					SetString(
								const char*				inString)
								{mString = inString;}
							/*
							*	Erases, sets and draws the new string.
							*/
	void					ReplaceString(
								const char*				inString);
protected:
	const char*				mString;
	XFont::Font*			mFont;
	uint16_t				mBGColor;
	uint16_t				mTextColor;

};

#endif // XMultiLineLabel_h
