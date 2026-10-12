/*
*	XTextField.h, Copyright Jonathan Mackey 2026
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
#ifndef XTextField_h
#define XTextField_h

#include "XControl.h"
#include "XFont.h"
class XKeyboard;

class XTextField : public XControl
{
public:
							XTextField(
								int16_t					inX,
								int16_t					inY,
								uint16_t				inWidth,
								uint16_t				inTag,
								XView*					inNextView,
								XFont::Font*			inFont,
								uint16_t				inFGColor = XFont::eWhite,
								uint16_t				inBGColor =  XFont::eBlack,
								uint16_t*				inTextBuffer = nullptr,
								uint16_t				inTextBufferLen = 0,
								uint16_t				inKeyboardOptions = 0,
								bool					inHasFrame = true,
								bool					inDrawFocus = false);
	virtual void			DrawSelf(void);
	virtual void			MouseDown(
								int16_t					inGlobalX,
								int16_t					inGlobalY);
	virtual void			MouseUp(
								int16_t					inGlobalX,
								int16_t					inGlobalY);
	void					ToggleCaret(void);
	void					UpdateCaret(void) const;
	void					ShowCaret(void);
	void					ShowCaret(
								uint16_t				inLocation);
	void					HideCaret(void);
	bool					CanDelete(void) const;
	void					DeleteChar(void);
	void					InsertChar(
								uint16_t				inCharcode);
							/*
							*	Returns the number of charcodes not uncluding
							*	the null terminator.
							*/
	uint16_t				CharcodeCount(void) const
								{return(mTextLen);}

	void					ClearTextStr(
								bool					inUpdate = false);
	const uint16_t*			GetUTF16Str(void) const
								{return(mTextBuffer);}
							/*
							*	Loads inUTF16Str into the text buffer.  The
							*	string will be terminated if it's longer than
							*	mTextMaxLen-1
							*/
	void					SetUTF16Str(
								const uint16_t*			inUTF16Str,
								bool					inUpdate = false);
							/*
							*	Converts the UTF-16 text buffer to UTF-8
							*	outStr must be long enough to hold the
							*	converted text.
							*/
	const char*				GetUTF8Str(
								char*					outUTF8Str) const;
							/*
							*	Converts outUTF8Str from UTF-8 to UTF-16.  The
							*	string will be truncated if it's converted
							*	length is longer than mTextMaxLen-1.
							*/
	void					SetUTF8Str(
								const char*				inUTF8Str,
								bool					inUpdate = false);
							/*
							*	Converts the first 4 characters into a 4 byte
							*	code.  This assumes the buffer only contains
							*	charcodes less than 0x100, otherwise the result
							*	will be the first 4 bytes of the internal UTF-16
							*	text buffer converted to UTF-8.
							*/
	uint32_t				Get4CharCode(void) const;
	void					Set4CharCode(
								uint32_t				in4CharCode,
								bool					inUpdate = false);
	void					SetKeyboard(
								XKeyboard*				inKeyboard);
	uint16_t				GetKeyboardOptions(void) const
								{return(mKeyboardOptions);}
	bool					HasFocus(void) const
								{return(mHasFocus);}
	bool					RelinquishFocus(void);
	virtual uint16_t		ID(void) const
								{return(XViewID::kXTextFieldID);}
	static bool				IsTextField(
								uint16_t				inID)
								{return(inID == XViewID::kXTextFieldID);}

	XFont*					MakeFontCurrent(void) const;
protected:
	XFont::Font*		mFont;
	uint16_t			mFGColor;
	uint16_t			mBGColor;
	uint16_t			mKeyboardOptions;
	uint16_t*			mTextBuffer;
	uint16_t			mTextMaxLen;
	uint16_t			mTextLen;
	uint16_t			mCaretLoc;		// Charcode index
	uint16_t			mCaretIndex;	// Used for toggling the caret
	bool				mCaretVisible;
	bool				mHasFocus;
	bool				mDrawFocus;
	bool				mHasFrame;		// Used to support subfields
				
	void					DrawText(
								XFont*					inXFont,
								uint16_t				inFromCharIdx) const;
	void					EraseTillFieldEnd(
								XFont*					inXFont) const;
	void					SetTextColors(
								XFont*					inXFont) const;
};
#endif // XTextField_h
