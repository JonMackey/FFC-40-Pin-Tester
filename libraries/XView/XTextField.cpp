/*
*	XTextField.cpp, Copyright Jonathan Mackey 2026
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

#include "XTextField.h"
#include "XKeyboard.h"
#include "DisplayController.h"

#ifdef __MACH__
#define _BV(bit) (1 << (bit))
#include <iostream>
#endif

const uint16_t	kFocusFrameColor = 0xFD90; // light blue
const uint16_t	kCaretColor = 0xFBC0; // light blue

/********************************** XTextField *********************************/
XTextField::XTextField(
	int16_t			inX,
	int16_t			inY,
	uint16_t		inWidth,
	uint16_t		inTag,
	XView*			inNextView,
	XFont::Font*	inFont,
	uint16_t		inFGColor,
	uint16_t		inBGColor,
	uint16_t*		inTextBuffer,
	uint16_t		inTextBufferLen,
	uint16_t		inKeyboardOptions,
	bool			inHasFrame,
	bool			inDrawFocus)
	: XControl(inX, inY, inWidth, 0, inTag, inNextView),
	  mTextBuffer(inTextBuffer), mTextMaxLen(inTextBufferLen),
	  mTextLen(0), mCaretLoc(0),
	  mFont(inFont),  mBGColor(inBGColor), mFGColor(inFGColor),
	  mCaretVisible(false), mKeyboardOptions(inKeyboardOptions),
	  mHasFocus(false), mHasFrame(inHasFrame || inDrawFocus), mDrawFocus(inDrawFocus)
{
	XFont*	xFont = MakeFontCurrent();
	if (xFont)
	{
		mHeight = xFont->GetFontHeader().height + 8;
		if (mWidth == 0)
		{
			// If only numbers then use the width of '0', else use 'W'
			xFont->LoadGlyph((mKeyboardOptions & XKeyboard::eUIntegersOnly) != 0 ? '0' : 'W');
			mWidth = xFont->Glyph().advanceX*mTextMaxLen;
			mWidth += (mHasFrame ? 12 : 6);
			//mWidth = 12 + (xFont->GetFontHeader().width*mTextMaxLen);
		}
	}
}

/****************************** MakeFontCurrent *******************************/
XFont* XTextField::MakeFontCurrent(void) const
{
	return(mFont->MakeCurrent());
}

/********************************** DrawSelf **********************************/
void XTextField::DrawSelf(void)
{
	XFont*	xFont = MakeFontCurrent();
	if (xFont)
	{
		int16_t	x = 0;
		int16_t	y = 0;	
		DisplayController*	display = xFont->GetDisplay();
		LocalToGlobal(x, y);
		if (mHasFrame)
		{
			/*
			*	If the text field has focus AND
			*	is drawing focus THEN
			*	draw the focus frame
			*/
			if (mHasFocus && mDrawFocus)
			{
				const uint16_t	hiTh = 3;
				display->SetFGColor(kFocusFrameColor);
				display->SetBGColor(mBGColor);
				display->DrawRoundedFrame(x, y, mWidth, mHeight, hiTh, hiTh, mFGColor, false);
			} else
			{
				display->DrawFrame(x, y, mWidth, mHeight, mBGColor, 2);
				display->DrawFrame(x+2, y+2, mWidth-4, mHeight-4, XFont::eGray, 1);
				display->FillRect(x+3, y+3, mWidth-6, mHeight-6, mFGColor);
			}
		} else
		{
			display->FillRect(x, y+3, mWidth, mHeight-6, mFGColor);
		}
		DrawText(xFont, 0);
	}
}

/********************************** DrawText **********************************/
void XTextField::DrawText(
	XFont*		inXFont,
	uint16_t	inFromCharIdx) const
{
	int16_t	x = 0;
	int16_t	y = 0;	
	DisplayController*	display = inXFont->GetDisplay();
	LocalToGlobal(x, y);

	if (mTextBuffer)
	{
		uint16_t	charcode;
		const uint16_t* txBufPtr = mTextBuffer;
		
		/*
		*	Skip any characters not being drawn
		*/
		for (uint32_t i = 0; i < inFromCharIdx; i++)
		{
			charcode = *(txBufPtr++);
			inXFont->LoadGlyph(charcode);
			x += inXFont->Glyph().advanceX;
		}
		
		SetTextColors(inXFont);
		// x offset = 3 for frame, 3 for caret
		display->MoveTo(y+4, x+(mHasFrame ? 6 : 3));
		for (charcode = *(txBufPtr++);
				charcode; charcode = *(txBufPtr++))
		{
			inXFont->DrawCharcode(charcode);
		}
	}
}

/******************************** SetTextColors *******************************/
void XTextField::SetTextColors(
	XFont*		inXFont) const
{
	uint16_t	textColor = mEnabled ? mBGColor :
				DisplayController::Calc565Color(mBGColor, 0, 184);
	inXFont->SetTextColor(textColor);
	inXFont->SetBGTextColor(mFGColor);
}

/******************************** ToggleCaret *********************************/
void XTextField::ToggleCaret(void)
{
	if (mEnabled && mVisible && mCaretVisible)
	{
		mCaretIndex++;
		UpdateCaret();
	}
}

/********************************** ShowCaret *********************************/
void XTextField::ShowCaret(void)
{
	mCaretVisible = true;
	mCaretIndex = 0;
}

/********************************** ShowCaret *********************************/
void XTextField::ShowCaret(
	uint16_t	inLocation)
{
	ShowCaret();
	if (inLocation > mTextLen) inLocation = mTextLen;
	mCaretLoc = inLocation;
}

/********************************** HideCaret *********************************/
void XTextField::HideCaret(void)
{
	if (mCaretVisible)
	{
		mCaretVisible = false;
		mCaretIndex = 0;
		UpdateCaret();
	}
}

/******************************** UpdateCaret *********************************/
/*
*	Draws or erases the caret based on mCaretIndex.
*/
void XTextField::UpdateCaret(void) const
{
	XFont*	xFont = MakeFontCurrent();
	if (mTextBuffer && xFont)
	{
		DisplayController*	display = xFont->GetDisplay();
		int16_t	x = mHasFrame ? 6 : 3;
		int16_t	y = 4;	

		LocalToGlobal(x, y);

		uint16_t	charCodeIdx = 0;
		uint16_t	leftCharcode = 0;
		uint16_t	rightCharcode = *mTextBuffer;

		while(rightCharcode)
		{
			if (charCodeIdx != mCaretLoc)
			{
				leftCharcode = rightCharcode;
				xFont->LoadGlyph(leftCharcode);
				x += xFont->Glyph().advanceX;
				charCodeIdx++;
				rightCharcode = mTextBuffer[charCodeIdx];
				continue;
			}
			break;
		}
		
		/*
		*	If the caret should be drawn...
		*/
		if (mCaretIndex & 1)
		{
			display->FillRect(x-1, y, 2, mHeight-8, kCaretColor);
			//display->FillRect(x-1, y, 2, mHeight-8, kCaretColor);
		/*
		*	Else the caret should be erased.  Because the caret draws over one
		*	or more glyphs, the glyphs need to redrawn.
		*/
		} else
		{
			/*
			*	If the string isn't empty
			*/
			if (mTextLen)
			{
				SetTextColors(xFont);
				/*
				*	If there's a glyph to the left of the caret THEN
				*	draw the left hand charcode to to erase the left half of
				*	the caret.
				*/
				if (leftCharcode)
				{
					display->MoveTo(y, x-xFont->Glyph().advanceX);
					xFont->DrawCharcode(leftCharcode);
				/*
				*	Else the caret is before the start of the string
				*/
				} else
				{
					display->FillRect(x-1, y, 1, mHeight-8, mFGColor);
					//display->FillRect(x-1, y, 1, mHeight-8, mFGColor);
				}

				/*
				*	If there's a glyph to the right of the caret THEN
				*	draw the right hand charcode to to erase the right half of
				*	the caret.
				*/
				if (rightCharcode)
				{
					xFont->DrawCharcode(rightCharcode);
				/*
				*	Else the caret is at the end of the string,
				*	erase the right half of the caret.
				*/
				} else
				{
					display->FillRect(x, y, 1, mHeight-8, mFGColor);
					//display->FillRect(x, y, 1, mHeight-8, mFGColor);
				}
			/*
			*	Else the string is empty, erase the caret.
			*/
			} else
			{
				display->FillRect(x-1, y, 2, mHeight-8, mFGColor);
				//display->FillRect(x-1, y, 2, mHeight-8, mFGColor);
			}
		}
		/*
		*	Odd bug.  If another fill isn't performed, the previous fill doesn't
		*	complete until the next display fill.  It doesn't matter if it's
		*	a 3-bit optimized color or a full 565 color.  The fix below performs
		*	a dummy fill at the end of the text box.  Note that this bug doesn't
		*	happen when drawing text but that may be because when a glyph is
		*	drawn the space between is filled, essentally performing the same
		*	fix.
		*/
		{
			x = mWidth - (mHasFrame ? 6 : 3);
			y = 4;
			LocalToGlobal(x, y);
			display->FillRect(x, y, 1, mHeight-8, mFGColor);
		}
	}
}

/********************************** CanDelete *********************************/
bool XTextField::CanDelete(void) const
{
	return(mCaretLoc > 0 && mCaretVisible);
}

/********************************* DeleteChar *********************************/
void XTextField::DeleteChar(void)
{
	if (CanDelete())
	{
		mCaretVisible = false;
		mCaretIndex = 0;
		mCaretLoc--;
		// Shift the string left, including the null terminator
		memmove(&mTextBuffer[mCaretLoc], &mTextBuffer[mCaretLoc+1], (mTextLen - mCaretLoc)*sizeof(uint16_t));
		mTextLen--;
		XFont*	xFont = MakeFontCurrent();
		if (xFont)
		{
			DrawText(xFont, mCaretLoc);
			EraseTillFieldEnd(xFont);
		}
		ShowCaret();
	}
}

/********************************* InsertChar *********************************/
void XTextField::InsertChar(
	uint16_t	inCharcode)
{
	XFont*	xFont = MakeFontCurrent();
	if (xFont)
	{
		HideCaret();
		/*
		*	If this text field has a numbers only option set THEN
		*	apply special handling for the minus sign and decimal point.
		*/
		if ((mKeyboardOptions & XKeyboard::eNumbersOnly) != 0)
		{
			/*
			*	If the inCharcode being inserted is a minus sign THEN
			*	only allow one minus sign as the first character.
			*/
			if (inCharcode == '-')
			{
				/*
				*	If there is no minus sign THEN
				*	make sure it's inserted as the first character.
				*/
				if (mTextBuffer[0] != '-')
				{
					mCaretLoc = 0;
				/*
				*	Else, cancel insert, there already is a minus sign.
				*/
				} else
				{
					inCharcode = 0;
				}
			/*
			*	Else if the inCharcode being inserted is a decimal point THEN
			*	only allow one.
			*/
			} else if (inCharcode == '.')
			{
				/*
				*	See if there already is a decimal point
				*/
				for (uint16_t loc = 0; loc < mTextLen; loc++)
				{
					if (mTextBuffer[loc] != '.')
					{
						continue;
					}
					/*
					*	If there already is a decimal point at mCaretLoc THEN
					*	cancel insert.
					*/
					if (loc == mCaretLoc)
					{
						inCharcode = 0;
					/*
					*	Else, remove the existing decimal point.
					*/
					} else
					{
						memmove(&mTextBuffer[loc], &mTextBuffer[loc+1], (mTextLen - loc)*sizeof(uint16_t));
						mTextLen--;
						/*
						*	If the decimal point removed was to the left of the
						*	new decimal point THEN
						*	account for its removal.
						*/
						if (mCaretLoc > loc)
						{
							mCaretLoc--;
							DrawText(xFont, loc);
						}
					}
					break;
				}
			}
		}
		if (inCharcode)
		{
			mTextLen++;
			bool	textTruncated = false;
			if (mTextLen > mTextMaxLen)
			{
				mTextLen--;
				if (mCaretLoc == mTextMaxLen)
				{
					ShowCaret();
					return;
				}
				mTextBuffer[mTextLen-1] = 0;
				textTruncated = true;
			}
			// Shift the string right, including the null terminator
			memmove(&mTextBuffer[mCaretLoc+1], &mTextBuffer[mCaretLoc], (mTextLen - mCaretLoc)*sizeof(uint16_t));
			mTextBuffer[mCaretLoc] = inCharcode;
			DrawText(xFont, mCaretLoc);
			if (textTruncated)
			{
				EraseTillFieldEnd(xFont);
			}
			mCaretLoc++;
		}
		ShowCaret();
	}
}

/****************************** EraseTillFieldEnd *****************************/
void XTextField::EraseTillFieldEnd(
	XFont*	inXFont) const
{
	DisplayController*	display = inXFont->GetDisplay();
	int16_t	fillWidth = mWidth;
	if (mHasFrame)
	{
		fillWidth -= 3;
	}
	int16_t	y = 0;	
	LocalToGlobal(fillWidth, y);
	fillWidth -= display->GetColumn();
	if (fillWidth > 0)
	{
		display->FillBlock(inXFont->FontRows(), fillWidth, inXFont->GetBGTextColor());
	}
}

/******************************** SetUTF16Str *********************************/
void XTextField::SetUTF16Str(
	const uint16_t*	inUTF16Str,
	bool			inUpdate)
{
	if (inUTF16Str && mTextBuffer)
	{
		bool	caretWasVisible = mCaretVisible;
		if (mCaretVisible)
		{
			HideCaret();
		}
		uint16_t*		txBufPtr = mTextBuffer;
		const uint16_t*	strPtr = inUTF16Str;
		uint16_t		characterCount = 0;
		
		for (uint16_t charcode = *(strPtr++);
				charcode && characterCount < mTextMaxLen;
					charcode = *(strPtr++))
		{
			if (charcode >= ' ')
			{
				characterCount++;
				*(txBufPtr++) = charcode;
			}
		}
		*txBufPtr = 0;
		mTextLen = mCaretLoc = characterCount;
		if (inUpdate || caretWasVisible)
		{
			DrawSelf();
			if (caretWasVisible)
			{
				ShowCaret();
			}
		}
	}
}

/********************************* GetUTF8Str *********************************/
const char* XTextField::GetUTF8Str(
	char*	outUTF8Str) const
{
	return(XFont::UTF16To8Str(mTextBuffer, outUTF8Str));
}

/********************************* SetUTF8Str *********************************/
void XTextField::SetUTF8Str(
	const char*	inUTF8Str,
	bool		inUpdate)
{
	XFont*	xFont = MakeFontCurrent();
	if (xFont && mTextBuffer && inUTF8Str)
	{
		bool	caretWasVisible = mCaretVisible;
		if (mCaretVisible)
		{
			HideCaret();
		}
		const char*	strPtr = inUTF8Str;
		uint16_t*	txBufPtr = mTextBuffer;
		uint16_t	characterCount = 0;
		for (uint16_t charcode = XFont::NextChar(strPtr);
				charcode && characterCount < mTextMaxLen;
					charcode = XFont::NextChar(strPtr))
		{
			if (charcode >= ' ')
			{
				characterCount++;
				*(txBufPtr++) = charcode;
			}
		}
		*txBufPtr = 0;
		mTextLen = mCaretLoc = characterCount;
		if (inUpdate || caretWasVisible)
		{
			DrawSelf();
			if (caretWasVisible)
			{
				ShowCaret();
			}
		}
	}
}

/******************************** ClearTextStr ********************************/
void XTextField::ClearTextStr(
	bool	inUpdate)
{
	bool	caretWasVisible = mCaretVisible;
	if (mCaretVisible)
	{
		HideCaret();
	}
	mTextBuffer[0] = 0;
	mTextLen = mCaretLoc = 0;
	if (inUpdate || caretWasVisible)
	{
		DrawSelf();
		if (caretWasVisible)
		{
			ShowCaret();
		}
	}
}

#ifndef Swap32
#define Swap32(data) ((((data ^ (data >> 16 | (data << 16))) & 0xFF00FFFF) >> 8) ^ (data >> 8 | data << 24))
#endif
/******************************** Get4CharCode ********************************/
/*
*	This routine assumes the mTextBuffer contains only 8-bit characters.
*
*	Note that the mTextBuffer may contain less than four characters.
*	Any missing characters are interpreted as nulls.
*	e.g. the text "G" would be converted as 'G', 0, 0, 0.  The resulting value
*	would be 0x47 00 00 00 or 0x47000000.
*/
uint32_t XTextField::Get4CharCode(void) const
{
	uint32_t	value = 0;
	if (mTextLen > 0 &&
		mTextLen <= 4)
	{
		char			valueStr[4] = {0};
		char*			strPtr = &valueStr[3];
		const uint16_t*	txBufPtr = mTextBuffer;
		
		for (uint16_t charcode = *(txBufPtr++); charcode; charcode = *(txBufPtr++))
		{
			*(strPtr--) = (uint8_t)charcode;
		}
		
		value = *(const uint32_t*)valueStr;
	}
	return(value);
}

/******************************** Set4CharCode ********************************/
void XTextField::Set4CharCode(
	uint32_t	in4CharCode,
	bool		inUpdate)
{
	char	valueStr[5];
	*(uint32_t*)valueStr = Swap32(in4CharCode);
	valueStr[4] = 0;
	SetUTF8Str(valueStr, inUpdate);
}

/****************************** RelinquishFocus *******************************/
/*
*	Returns true if focus was relinquished.
*/
bool XTextField::RelinquishFocus(void)
{
	mHasFocus = false;
	HideCaret();
	DrawSelf();
	return(true);
}

/********************************* MouseDown **********************************/
void XTextField::MouseDown(
	int16_t	inGlobalX,
	int16_t	inGlobalY)
{
	XKeyboard*	keyboard = XKeyboard::GetInstance();
	bool	focusChanged = false;
	if (!mHasFocus)
	{
		mHasFocus = keyboard->ShowFor(this);
		focusChanged = mHasFocus;
	}
	if (mHasFocus)
	{
		XFont*	xFont = MakeFontCurrent();
		if (mTextBuffer && xFont)
		{
			int16_t	mouseX = inGlobalX;
			int16_t	mouseY = inGlobalY;
			GlobalToLocal(mouseX, mouseY);
			int16_t	frameInset = mHasFrame ? 3 : 0;
			
			/*
			*	If the click is inside the frame...
			*/
			if (mouseY >= 4 &&
				mouseY < (mHeight-4) &&
				mouseX >= frameInset &&
				mouseX < (mWidth-frameInset))
			{
				int16_t	x = mHasFrame ? 6 : 3;
				int16_t	advanceX;
				int16_t	xPlusHalfAdv;
				uint16_t	charIdx = 0;
				for (uint16_t charcode = mTextBuffer[charIdx];
						charcode; charcode = mTextBuffer[charIdx])
				{
					xFont->LoadGlyph(charcode);
					advanceX = xFont->Glyph().advanceX;
					xPlusHalfAdv = x + (advanceX/2);
					if (mouseX > xPlusHalfAdv)
					{
						x += advanceX;
						charIdx++;
						continue;
					}
					break;
				}
				if (!mCaretVisible)
				{
					ShowCaret(charIdx);
					keyboard->UpdateDeleteButton(true);
				} else if (charIdx != mCaretLoc)
				{
					HideCaret();
					ShowCaret(charIdx);
					keyboard->UpdateDeleteButton(true);
				}
			}
		}
		if (focusChanged)
		{
			DrawSelf();
		}
	}
}

/********************************** MouseUp ***********************************/
void XTextField::MouseUp(
	int16_t	inGlobalX,
	int16_t	inGlobalY)
{
	{
		HandleChange(this, XControl::eOff);
	}
}
