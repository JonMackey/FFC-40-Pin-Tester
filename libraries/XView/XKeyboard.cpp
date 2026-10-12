/*
*	XKeyboard.cpp, Copyright Jonathan Mackey 2026
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

#include "XKeyboard.h"
#include "XTextField.h"
#include "XRootView.h"
#include "DisplayController.h"

#ifdef __MACH__
#define _BV(bit) (1 << (bit))
#include <iostream>
#endif
/*
*	There is only one instance of XKeyboard.  In the XView hierarchy, XKeyboard
*	should be placed after the alert and warning dialogs, but above any dialog
*	that may need to use the keyboard.
*
*	XKeyboard draws somewhat like an XMenu in that it self positions relative to
*	the XTextField.  For this reason the position of the XTextField must provide
*	enough display space to draw the keyboard.
*
*	XKeyboard is only made visible by clicking within a text field.  The text
*	field requests that the keyboard show itself by calling
*	XKeyboard::ShowFor(). If the keyboard is already showing at the request of
*	another text field, that text field is asked if it will relinquish focus. 
*	ShowFor() returns true if focus was relinquished.  At that point the text
*	field may optionally draw a focus frame.
*
*	When the keyboard is visible, the keyboard will periodically call the
*	XTextField to toggle the caret.
*
*	The keyboard is hidden by explicitly pressing the Done keyboard button or
*	by clicking on another focusable item, such as a different XTextField.
*
*	To determine if another text field has been hit when a click is made outside
*	of the keyboard or focused text field frame, XKeyboard will hit test text
*	field items in the text field's superview.  For this to work, text field tag
*	values must have the XView::eTextFieldTagModifier bit set.
*/

const uint32_t	kUnscaledDim[] =
{
	75,		//	eSmallWidth		button width
	103,	//	eMediumWidth1	button width
	111,	//	eMediumWidth2	button width
	213,	//	eWideWidth		button width
	214,	//	eSpaceBarWidth1	button width
	446,	//	eSpaceBarWidth2	button width
	126,	//	eHeight			button height (all)
	37,		//	eVSpaceBetween	vertical space between buttons (all)
	19,		//	eHSpaceBetween	horizontal space between buttons in group
	71,		//	eVBorderSpace	space from border to first/last button edge
	36,		//	eHBorderSpace	space from border to first/last button edge
	75,		//	eBorderCornerRadius
	22		//	eButtonCornerRadius
};
const uint32_t	kUnscaled0Height = 48;

/*
*	The multi character string indexes are used for the 3 character keyboard
*	text.  All other text are single 16 bit charcodes.  There can be no more
*	than 32 indexes because charcodes 0 to 31 are reused as indexes.  When
*	defining the layout charcode, any charcode value lessthan 32 is assumed to
*	be an index to kMultiCharStrs[].
*/
enum EMultiCharStrs
{
	eABCStrIndex,
	e123StrIndex,
	eSpStrIndex,
	eDoneStrIndex
};

const uint16_t	kDeleteCharcode = 0x232B;
const uint16_t	kReturnCharcode = 0x21A9; 
const uint16_t	kShiftCharcode = 0x21E7;
const uint16_t	kUseEnterStrCharcode = 0x1F;	// A flag to override enter

const char*	kMultiCharStrs[] = {"ABC", "123", "#+=", "Done"};

/*
	75 -> 23.44
	103	-> 32.19
	111 -> 34.69
	213 -> 66.56
	446 -> 139.37
	126 -> 39.38
	37 -> 11.56
	19 -> 5.94
	71 -> 22.19
	36 -> 11.25
*/

/*
*	Uppercase Layout
*/
const uint8_t	kUppercaseLayoutRow1[] =
{
	10, XKeyboard::eSmallWidth, SLayoutButton::eNormalFont, XFont::eAlignLeft, 0			// QWERTYUIOP
};
const uint8_t	kUppercaseLayoutRow2[] =
{
	9, XKeyboard::eSmallWidth, SLayoutButton::eNormalFont, XFont::eAlignCenter, 0			// ASDFGHJKL
};
const uint8_t	kUppercaseLayoutRow3[] =
{
	1, XKeyboard::eMediumWidth1, SLayoutButton::eNormalFont, XFont::eAlignLeft,			// ⬆︎ (⇧)
		7, XKeyboard::eSmallWidth, SLayoutButton::eNormalFont, XFont::eAlignCenter,		// ZXCVBNM
		1, XKeyboard::eMediumWidth1, SLayoutButton::eNormalFont, XFont::eAlignRight, 0	// ⌫
};
const uint8_t	kUppercaseLayoutRow4[] =
{
	1, XKeyboard::eWideWidth, SLayoutButton::eNormalFont, XFont::eAlignLeft,			// 123
		1, XKeyboard::eSpaceBarWidth2, SLayoutButton::eNormalFont, XFont::eAlignCenter,	//
		1, XKeyboard::eWideWidth, SLayoutButton::eSmallFont, XFont::eAlignRight, 0		// [Done]
};
const uint16_t	kUppercaseCharcodesRow1[] =
{
	0x51, 0x57, 0x45, 0x52, 0x54, 0x59, 0x55, 0x49, 0x4F, 0x50	// QWERTYUIOP
};
const uint16_t	kUppercaseCharcodesRow2[] =
{
	0x41, 0x53, 0x44, 0x46, 0x47, 0x48, 0x4A, 0x4B, 0x4C		// ASDFGHJKL
};
const uint16_t	kUppercaseCharcodesRow3[] =
{
	0x21E7, 0x5A, 0x58, 0x43, 0x56, 0x42, 0x4E, 0x4D, 0x232B	// ⇧ZXCVBNM⌫
};
const uint16_t	kUppercaseCharcodesRow4[] =
{
	e123StrIndex, 0x20, eDoneStrIndex									// [123] [Done]
};
SKeyboardLayout	XKeyboard::sUppercaseLayout =
{
	SKeyboardLayout::eMaxLayoutRows,
	{kUppercaseCharcodesRow1, kUppercaseCharcodesRow2, kUppercaseCharcodesRow3, kUppercaseCharcodesRow4},
	{kUppercaseLayoutRow1, kUppercaseLayoutRow2, kUppercaseLayoutRow3, kUppercaseLayoutRow4},
	kMultiCharStrs
};

/*
*	Lowercase Layout
*/
const uint8_t	kLowercaseLayoutRow1[] =
{
	10, XKeyboard::eSmallWidth, SLayoutButton::eNormalFont, XFont::eAlignLeft, 0			// qwertyuiop
};
const uint8_t	kLowercaseLayoutRow2[] =
{
	9, XKeyboard::eSmallWidth, SLayoutButton::eNormalFont, XFont::eAlignCenter, 0			// asdfghjkl
};
const uint8_t	kLowercaseLayoutRow3[] =
{
	1, XKeyboard::eMediumWidth1, SLayoutButton::eNormalFont, XFont::eAlignLeft,			// ⇧
		7, XKeyboard::eSmallWidth, SLayoutButton::eNormalFont, XFont::eAlignCenter,		// zxcvbnm
		1, XKeyboard::eMediumWidth1, SLayoutButton::eNormalFont, XFont::eAlignRight, 0	// ⌫
};
const uint8_t	kLowercaseLayoutRow4[] =
{
	1, XKeyboard::eWideWidth, SLayoutButton::eNormalFont, XFont::eAlignLeft,			// 123
		1, XKeyboard::eSpaceBarWidth2, SLayoutButton::eNormalFont, XFont::eAlignCenter,	//
		1, XKeyboard::eWideWidth, SLayoutButton::eSmallFont, XFont::eAlignRight, 0		// [Done]
};
const uint16_t	kLowercaseCharcodesRow1[] =
{
	0x71, 0x77, 0x65, 0x72, 0x74, 0x79, 0x75, 0x69, 0x6F, 0x70	// qwertyuiop
};
const uint16_t	kLowercaseCharcodesRow2[] =
{
	0x61, 0x73, 0x64, 0x66, 0x67, 0x68, 0x6A, 0x6B, 0x6C		// asdfghjkl
};
const uint16_t	kLowercaseCharcodesRow3[] =
{
	0x21E7, 0x7A, 0x78, 0x63, 0x76, 0x62, 0x6E, 0x6D, 0x232B	// ⇧zxcvbnm⌫
};
const uint16_t	kLowercaseCharcodesRow4[] =
{
	e123StrIndex, 0x20, eDoneStrIndex									// [123] [Done]
};
SKeyboardLayout	XKeyboard::sLowercaseLayout =
{
	SKeyboardLayout::eMaxLayoutRows,
	{kLowercaseCharcodesRow1, kLowercaseCharcodesRow2, kLowercaseCharcodesRow3, kLowercaseCharcodesRow4},
	{kLowercaseLayoutRow1, kLowercaseLayoutRow2, kLowercaseLayoutRow3, kLowercaseLayoutRow4},
	kMultiCharStrs
};


/*
*	Numbers Layout
*/
const uint8_t	kNumbersLayoutRow1[] =
{
	10, XKeyboard::eSmallWidth, SLayoutButton::eNormalFont, XFont::eAlignLeft, 0		// 1234567890
};
const uint8_t	kNumbersLayoutRow2[] =
{
	10, XKeyboard::eSmallWidth, SLayoutButton::eNormalFont, XFont::eAlignLeft, 0		// -/:;()$&@“
};
const uint8_t	kNumbersLayoutRow3[] =
{
	1, XKeyboard::eMediumWidth1, SLayoutButton::eSmallFont, XFont::eAlignLeft,			// #+=
		5, XKeyboard::eMediumWidth2, SLayoutButton::eNormalFont, XFont::eAlignCenter,	// .,?!’
		1, XKeyboard::eMediumWidth1, SLayoutButton::eNormalFont, XFont::eAlignRight, 0	// ⌫
};
const uint8_t	kNumbersLayoutRow4[] =
{
	1, XKeyboard::eWideWidth, SLayoutButton::eNormalFont, XFont::eAlignLeft,			// ABC
		1, XKeyboard::eSpaceBarWidth2, SLayoutButton::eNormalFont, XFont::eAlignCenter,	//
		1, XKeyboard::eWideWidth, SLayoutButton::eSmallFont, XFont::eAlignRight, 0		// [Done]
};
const uint16_t	kNumbersCharcodesRow1[] =
{
	0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30	// 1234567890
};
const uint16_t	kNumbersCharcodesRow2[] =
{
	0x2D, 0x2F, 0x3A, 0x3B, 0x28, 0x29, 0x24, 0x26, 0x40, 0x201C// -/:;()$&@“
};
const uint16_t	kNumbersCharcodesRow3[] =
{
	eSpStrIndex, 0x2E, 0x2C, 0x3F, 0x21, 0x2019, 0x232B			// [#+=].,?!’⌫
};
const uint16_t	kNumbersCharcodesRow4[] =
{
	eABCStrIndex, 0x20, eDoneStrIndex									// [ABC] [Done]
};
SKeyboardLayout	XKeyboard::sNumbersLayout =
{
	SKeyboardLayout::eMaxLayoutRows,
	{kNumbersCharcodesRow1, kNumbersCharcodesRow2, kNumbersCharcodesRow3, kNumbersCharcodesRow4},
	{kNumbersLayoutRow1, kNumbersLayoutRow2, kNumbersLayoutRow3, kNumbersLayoutRow4},
	kMultiCharStrs
};

/*
*	Special Characters Layout
*/
const uint8_t	kSpecialCharsLayoutRow1[] =
{
	10, XKeyboard::eSmallWidth, SLayoutButton::eNormalFont, XFont::eAlignLeft, 0			// []{}#%^*+=
};
const uint8_t	kSpecialCharsLayoutRow2[] =
{
	10, XKeyboard::eSmallWidth, SLayoutButton::eNormalFont, XFont::eAlignLeft, 0			// _\|~<>€£¥•
};
const uint8_t	kSpecialCharsLayoutRow3[] =
{
	1, XKeyboard::eMediumWidth1, SLayoutButton::eSmallFont, XFont::eAlignLeft,			// 123
		5, XKeyboard::eMediumWidth2, SLayoutButton::eNormalFont, XFont::eAlignCenter,	// .,?!’
		1, XKeyboard::eMediumWidth1, SLayoutButton::eNormalFont, XFont::eAlignRight, 0	// ⌫
};
const uint8_t	kSpecialCharsLayoutRow4[] =
{
	1, XKeyboard::eWideWidth, SLayoutButton::eNormalFont, XFont::eAlignLeft,			// ABC
		1, XKeyboard::eSpaceBarWidth2, SLayoutButton::eNormalFont, XFont::eAlignCenter,	//
		1, XKeyboard::eWideWidth, SLayoutButton::eSmallFont, XFont::eAlignRight, 0		// [Done]
};
const uint16_t	kSpecialCharsCharcodesRow1[] =
{
	0x5B, 0x5D, 0x7B, 0x7D, 0x23, 0x25, 0x5E, 0x2A, 0x2B, 0x3D	// []{}#%^*+=
};
const uint16_t	kSpecialCharsCharcodesRow2[] =
{
	0x5F, 0x5C, 0x7C, 0x7E, 0x3C, 0x3E, 0x20AC, 0xA3, 0xA5, 0x2022	//  _\|~<>€£¥•
};
const uint16_t	kSpecialCharsCharcodesRow3[] =
{
	e123StrIndex, 0x2E, 0x2C, 0x3F, 0x21, 0x2019, 0x232B		// [123].,?!’⌫
};
const uint16_t	kSpecialCharsCharcodesRow4[] =
{
	eABCStrIndex, 0x20, eDoneStrIndex									// [ABC] [Done]
};
SKeyboardLayout	XKeyboard::sSpecialCharsLayout =
{
	SKeyboardLayout::eMaxLayoutRows,
	{kSpecialCharsCharcodesRow1, kSpecialCharsCharcodesRow2, kSpecialCharsCharcodesRow3, kSpecialCharsCharcodesRow4},
	{kSpecialCharsLayoutRow1, kSpecialCharsLayoutRow2, kSpecialCharsLayoutRow3, kSpecialCharsLayoutRow4},
	kMultiCharStrs
};

XKeyboard*		XKeyboard::sInstance;

/********************************** XKeyboard *********************************/
XKeyboard::XKeyboard(
	uint16_t		inTag,
	XView*			inNextView,
	XFont::Font*	inNormalFont,
	XFont::Font*	inSmallFont,
	uint16_t		inFGColor,
	uint16_t		inBGColor)
	: XView(0, 0, 0, 0, inTag, inNextView, nullptr, nullptr, false),
	  mNormalFont(inNormalFont), mSmallFont(inSmallFont), 
	  mBGColor(inBGColor), mFGColor(inFGColor), mCurrentLayout(nullptr),
	  mSavedModalView(nullptr), mAppliedOptions(eAllCharcodes)
{
	sInstance = this;
	XFont*	xFont = MakeFontCurrent(SLayoutButton::eNormalFont);
	if (xFont)
	{
		/*
		*	Get the scale.  All of the unscaled values are scaled based on the
		*	height of font glyph '0' and the measured kUnscaled0Height.
		*/
		xFont->LoadGlyph('0');
		float	scale = (float)xFont->Glyph().rows/kUnscaled0Height;
		for (uint32_t j = 0; j < eNumDimIndexes; j++)
		{
			mScaledDim[j] = round(kUnscaledDim[j] * scale);
		}

		mWidth = (mScaledDim[eHBorderSpace] * 2) + (mScaledDim[XKeyboard::eSmallWidth] * 10) + (mScaledDim[eHSpaceBetween] * 9);
		mHeight = (mScaledDim[eVBorderSpace] * 2) + (mScaledDim[eHeight] * 4) +(mScaledDim[eVSpaceBetween] * 3);
		LoadLayout(&sNumbersLayout);
		LoadLayout(&sLowercaseLayout);
		LoadLayout(&sSpecialCharsLayout);
		LoadLayout(&sUppercaseLayout);
	}
}

/********************************* LoadLayout *********************************/
void XKeyboard::LoadLayout(
	SKeyboardLayout*	inLayout)
{
	if (mCurrentLayout != inLayout)
	{
		mCurrentLayout = inLayout;
		if (inLayout && !inLayout->initialized)
		{
			inLayout->initialized = true;
			{
				uint16_t	currYTop = mScaledDim[eVBorderSpace];
				uint16_t*	rowTop = inLayout->rowTop;
				uint16_t	rowAdv = mScaledDim[eHeight] + mScaledDim[eVSpaceBetween];
				for (uint32_t i = 0; i < SKeyboardLayout::eMaxLayoutRows; i++)
				{
					rowTop[i] = currYTop;
					currYTop += rowAdv;
				}
			}
			{
				uint16_t	hSpaceBetween = mScaledDim[eHSpaceBetween];
				for (uint32_t row = 0; row < inLayout->rows; row++)
				{
					uint16_t	currXLeft = mScaledDim[eHBorderSpace];
					uint16_t	currXRight = mWidth - mScaledDim[eHBorderSpace];
					const uint8_t*	layoutRow = inLayout->layoutRows[row];
					SLayoutButton*	button = inLayout->button[row];
					
					for (uint32_t buttonsInGroup = *(layoutRow++);
							buttonsInGroup; buttonsInGroup = *(layoutRow++))
					{
						uint16_t	buttonWidth = mScaledDim[*(layoutRow++)];
						SLayoutButton::EButtonFont	buttonFont = (SLayoutButton::EButtonFont)*(layoutRow++);
						uint16_t	groupWidth = (buttonWidth * buttonsInGroup) +
													(hSpaceBetween * (buttonsInGroup-1));
						/*
						*	Calculate the left offset of the button group
						*/
						switch (*(layoutRow++))
						{
							//case XFont::eAlignLeft:
							//	break;
							case XFont::eAlignCenter:
							{
								/*
								*	Center alignment assumes the there is enough space for the button group.
								*	Center alignment blindly places the button group in the center of the row.
								*/
								currXLeft = (mWidth - groupWidth)/2;
								break;
							}
							case XFont::eAlignRight:
								currXRight -= groupWidth;
								currXLeft = currXRight;
								break;
						}
						
						/*
						*	Populate the left, width, and font for each button
						*	in the group.
						*/
						for (uint32_t bIdx = 0; bIdx < buttonsInGroup; bIdx++)
						{
							button->left = currXLeft;
							currXLeft += (buttonWidth + hSpaceBetween);
							button->width = buttonWidth;
							button->font = buttonFont;
							button->enabled = true;
							button++;
						}
						inLayout->buttonsInRow[row] += buttonsInGroup;
					}
				}
			}
		}
		UpdateDeleteButton(false);
	}
}

/****************************** MakeFontCurrent *******************************/
XFont* XKeyboard::MakeFontCurrent(
	SLayoutButton::EButtonFont	inEButtonFont)
{
	return((inEButtonFont == SLayoutButton::eNormalFont ? mNormalFont : mSmallFont)->MakeCurrent());
}

/********************************** DrawSelf **********************************/
void XKeyboard::DrawSelf(void)
{
	
	XFont*	xFont = MakeFontCurrent(SLayoutButton::eNormalFont);
	if (xFont && mCurrentLayout)
	{
		DisplayController*	display = xFont->GetDisplay();
		display->SetFGColor(mBGColor);
		int16_t	x = 0;
		int16_t	y = 0;
		// Note: if mY's global value is zero, FillRect and
		// DrawRoundedRect will fail.
		LocalToGlobal(x, y);
		display->DrawRoundedRect(x, y, mWidth, mHeight, mScaledDim[eBorderCornerRadius]);
		DrawButtons();
	}
}

/******************************** DrawButtons *********************************/
/*
*	The default is to draw all of the buttons in the mCurrentLayout by setting
*	inRow and inButtonIdx to zero, and inDrawAllButtons to true.
*
*	To draw a single button in the mCurrentLayout, set the button's row and
*	index, and inDrawAllButtons to false.
*/
void XKeyboard::DrawButtons(
	uint16_t	inRow,
	uint16_t	inButtonIdx,
	bool		inDrawAllButtons)
{
	XFont*	xFont = MakeFontCurrent(SLayoutButton::eNormalFont);
	if (xFont && mCurrentLayout)
	{
		DisplayController*	display = xFont->GetDisplay();
		//display->SetFGColor(kDarkBlueColor);
		display->SetBGColor(mBGColor);
		int16_t	x = 0;
		int16_t	y = 0;
		// Note: if mY's global value is zero, FillRect and
		// DrawRoundedRect will fail.
		LocalToGlobal(x, y);
		
		
		uint16_t	bHeight = mScaledDim[eHeight];
		uint16_t	charTop = (bHeight-xFont->GetFontHeader().height)/2;
		uint16_t	cornerRadius = mScaledDim[eButtonCornerRadius];
		uint32_t	rows = inDrawAllButtons ? mCurrentLayout->rows : inRow +1;
		display->SetFGColor(XFont::eWhite);
		xFont->SetBGTextColor(XFont::eWhite);
		SLayoutButton::EButtonFont buttonFont = SLayoutButton::eNormalFont;
		for (uint32_t row = inRow; row < rows; row++)
		{
			SLayoutButton*	button = mCurrentLayout->button[row];
			const uint16_t*	charcodes = mCurrentLayout->charcodes[row];
			uint32_t buttonsInRow;
			if (inDrawAllButtons)
			{
				buttonsInRow = mCurrentLayout->buttonsInRow[row];
			} else
			{
				buttonsInRow = inButtonIdx +1;
				button += inButtonIdx;
			}
										
			uint16_t	bTop = y + mCurrentLayout->rowTop[row];
			for (uint32_t btnIdx = inButtonIdx; btnIdx < buttonsInRow; btnIdx++, button++)
			{
				xFont->SetTextColor(button->enabled ? XFont::eBlack : XFont::eGray);
				uint16_t	bLeft = x+button->left;
				display->DrawRoundedRect(bLeft, bTop, button->width, bHeight, cornerRadius);
				uint16_t	charcode = charcodes[btnIdx];
				if (buttonFont != button->font)
				{
					buttonFont = button->font;
					xFont = MakeFontCurrent(buttonFont);
					if (xFont)
					{
						charTop = (bHeight-xFont->GetFontHeader().height)/2;
					} else
					{
						return;
					}
				}
								
				if (charcode >= 0x20)
				{
					if (xFont->LoadGlyph(charcode))
					{
						display->MoveTo(bTop+charTop, bLeft+ (button->width - xFont->Glyph().columns)/2);
						xFont->DrawCharcode(charcode);
					}
				} else
				{
					xFont->DrawAligned(mCurrentLayout->multiCharStrs[charcode],
									bLeft, bTop+charTop, button->width,
										XFont::eAlignCenter, false);
				}
			}
		}
	}
}

/******************************** ToggleCaret *********************************/
void XKeyboard::ToggleCaret(void)
{
	if (mTextField)
	{
		mTextField->ToggleCaret();
	}
}

/********************************** HitTest ***********************************/
/*
*	XKeyboard is modal, so this routine is called from the root where inX & inY
*	are global.
*
*	Returns the keyboard if hit, else it will return any text field hit on the
*	same subview chain as mTextField.  Will return nullptr if nothing hit.
*/
XView* XKeyboard::HitTest(
	int16_t	inX,
	int16_t	inY)
{
	int16_t	localX = inX - mX;
	int16_t	localY = inY - mY;

	XView* hitView = HitSelf(localX, localY) ? this : nullptr;
	
	if (hitView == nullptr && mTextField)
	{
		XView*	superView = mTextField->SuperView();
		if (superView)
		{
			localX = inX;
			localY = inY;
			superView->GlobalToLocal(localX, localY);
			
			for (XView* thisView = superView->SubViews(); thisView != nullptr;
					thisView = thisView->NextView())
			{
				if (thisView->HitSelf(localX-thisView->X(), localY-thisView->Y()) == false)
				{
					continue;
				}
				if (XTextField::IsTextField(thisView->ID()) &&
					thisView->IsVisible())
				{
					hitView = thisView;
				}
				break;
			}
		}
	}
	return(hitView);
}

/********************************* MouseDown **********************************/
void XKeyboard::MouseDown(
	int16_t	inGlobalX,
	int16_t	inGlobalY)
{
	if (mCurrentLayout)
	{
		int16_t	x = inGlobalX - mX;
		int16_t	y = inGlobalY - mY;

		/*
		*	Determine the row
		*/
		uint16_t	bHeight = mScaledDim[eHeight];
		for (uint32_t row = 0; row < mCurrentLayout->rows; row++)
		{
			uint16_t	bTop = mCurrentLayout->rowTop[row];
			if (y >= bTop)
			{
				if (y <= bTop+bHeight)
				{
					SLayoutButton*	button = mCurrentLayout->button[row];
					uint32_t buttonsInRow = mCurrentLayout->buttonsInRow[row];
					for (uint32_t btnIdx = 0; btnIdx < buttonsInRow; btnIdx++, button++)
					{
						if (x >= button->left)
						{
							if (x <= button->left+button->width)
							{
								if (button->enabled)
								{
									uint16_t charcode = mCurrentLayout->charcodes[row][btnIdx];
									switch(charcode)
									{
										case kShiftCharcode:
											LoadLayout(mCurrentLayout == &sLowercaseLayout ? &sUppercaseLayout : &sLowercaseLayout);
											DrawSelf();
											break;
										case eABCStrIndex:
											LoadLayout(&sLowercaseLayout);
											DrawSelf();
											break;
										case e123StrIndex:
											LoadLayout(&sNumbersLayout);
											DrawSelf();
											break;
										case eSpStrIndex:
											LoadLayout(&sSpecialCharsLayout);
											DrawSelf();
											break;
										case eDoneStrIndex:
											Hide();
											break;
										case kDeleteCharcode:
											if (mTextField)
											{
												mTextField->DeleteChar();
												UpdateDeleteButton(true);
											}
											break;
										default:
											if (mTextField && charcode >= ' ')
											{
												mTextField->InsertChar(charcode);
												UpdateDeleteButton(true);
											}
											break;
									}
								}
								break;
							}
						} else
						{
							break;
						}
					}
					break;
				}
			} else
			{
				break;
			}
		}
	}
}

/***************************** UpdateDeleteButton *****************************/
void XKeyboard::UpdateDeleteButton(
	bool	inDrawChange)
{
	if (mTextField && mCurrentLayout)
	{
		uint16_t	row, btnIdx;
		SLayoutButton* button = mCurrentLayout->FindButton(kDeleteCharcode, row, btnIdx);
		bool	canDelete = mTextField->CanDelete();
		if (button && button->enabled != canDelete)
		{
			button->enabled = canDelete;
			if (inDrawChange)
			{
				DrawButtons(row, btnIdx, false);
			}
		}
	}
}

/************************************ Hide ************************************/
void XKeyboard::Hide(void)
{
	if (mTextField &&
		mTextField->RelinquishFocus())
	{
		mTextField = nullptr;
		XRootView::GetInstance()->SetModalView(mSavedModalView);
		XView::Hide();
	}	
}

/********************************** ShowFor ***********************************/
bool XKeyboard::ShowFor(
	XTextField*	inTextField)
{
	XRootView*	rootView = XRootView::GetInstance();
	bool		success = true;
	/*
	*	If the keyboard is already a client of another text field THEN
	*	see if that text field will reliquish focus.
	*/
	if (mTextField)
	{
		success = mTextField->RelinquishFocus();
	}	
	if (success)
	{
		int16_t	tfGlobalX = 0;
		int16_t	tfGlobalY = 0;
		uint16_t	tfWidth = inTextField->Width();
		uint16_t	tfHeight = inTextField->Height();
#ifdef __MACH__
		uint16_t	dispWidth = 480;
		uint16_t	dispHeight = 320;
#else
		DisplayController*	display = rootView->GetDisplay();
		uint16_t	dispWidth = display->GetColumns();
		uint16_t	dispHeight = display->GetRows();
#endif	
		mTextField = inTextField;
		inTextField->LocalToGlobal(tfGlobalX, tfGlobalY);
		/*
		*	If the keyboard isn't already visible OR
		*	the keyboard overlaps the text field THEN
		*	move the keyboard by setting its origin
		*/
		if (!mVisible ||
			!(mX+mWidth <= tfGlobalX || 
			mX >= tfGlobalX+tfWidth || 
			mY+mHeight <= tfGlobalY || 
			mY >= tfGlobalY+tfHeight))
		{
			const uint16_t	kGap = 15;
			XView::Hide();
			int16_t	x = (dispWidth - mWidth)/2;	// Center horizontally
			int16_t y = tfGlobalY - mHeight - kGap;
			/*
			*	If there isn't enough space to the above of the text field THEN
			*	see if there's enough space to the below of the text field.
			*/
			if (y < 0)
			{
				y = tfGlobalY + tfHeight + kGap;
				/*
				*	If there isn't enough space to below the text field THEN
				*	see if there's enough space to the left of the text field.
				*/
				if (y + mHeight > dispHeight)
				{
					x = tfGlobalX - mWidth - kGap;
					y = (dispHeight - mHeight)/2; // Center vertically
					/*
					*	If there isn't enough space to the left of the text field THEN
					*	see if there's enough space to the right of the text field.
					*/
					if (x < 0)
					{
						x = tfGlobalX + tfWidth + kGap;
						/*
						*	If there isn't enough space to the right of the text field THEN
						*	there isn't enough space for the keyboard without
						*	overlapping the text field.  So punt and place it in
						*	center of the display, the text field needs to be moved.
						*/
						if (x + mWidth > dispWidth)
						{
							x = (dispWidth - mWidth)/2;
						}
					}
				}
			}
			mX = x;
			mY = y;
		}
		
		/*
		*	If the options changed AND
		*	the keyboard is already visible THEN
		*	redraw all of the buttons
		*/
		{
			SKeyboardLayout*	prevLayout = mCurrentLayout;
			if (ApplyOptions(inTextField->GetKeyboardOptions()) &&
				mVisible)
			{
				if (prevLayout != mCurrentLayout)
				{
					DrawSelf();
				} else
				{
					DrawButtons();
				}
			}
		}
		/*
		*	If the keyboard isn't the modal view THEN
		*	set the keyboard as the modal view.
		*/
		if (rootView->ModalView() != this)
		{
			mSavedModalView = rootView->ModalView();
			rootView->SetModalView(this);
		}
		/*
		*	Update the delete button and redraw it only if the keyboard is
		*	already visible.
		*/
		UpdateDeleteButton(mVisible);
		Show();
	}
	return(success);
}

/******************************** ApplyOptions ********************************/
bool XKeyboard::ApplyOptions(
	uint16_t	inOptionFlags)
{
	bool	optionsChanged = inOptionFlags != mAppliedOptions;
	if (optionsChanged)
	{
		mAppliedOptions = inOptionFlags;
		sUppercaseLayout.ForEachButton(EnableAllChars);
		sLowercaseLayout.ForEachButton(EnableAllChars);
		sNumbersLayout.ForEachButton(EnableAllChars);
		sSpecialCharsLayout.ForEachButton(EnableAllChars);
		
		// Apply non-exlusive options
		if (inOptionFlags & e8BitOnly)
		{
			sNumbersLayout.ForEachButton(Disable16BitChars);
			sSpecialCharsLayout.ForEachButton(Disable16BitChars);
		}
		
		// Apply Exclusive options
		if (inOptionFlags & eNumbersOnly)
		{
			sNumbersLayout.ForEachButton(DisableNonNumbers);
			if (inOptionFlags & eDisableDecPt)
			{
				sNumbersLayout.EnableButton('.', false);
			}
			if (inOptionFlags & eDisableMinusSign)
			{
				sNumbersLayout.EnableButton('-', false);
			}
			LoadLayout(&sNumbersLayout);
		} else if (inOptionFlags == eUppercaseOnly)
		{
			sUppercaseLayout.EnableButton(kShiftCharcode, false);
			sUppercaseLayout.EnableButton(e123StrIndex, false);
			LoadLayout(&sUppercaseLayout);
		} else if (inOptionFlags == eLowercaseOnly)
		{
			sLowercaseLayout.EnableButton(kShiftCharcode, false);
			sLowercaseLayout.EnableButton(e123StrIndex, false);
			LoadLayout(&sLowercaseLayout);
		} else if (inOptionFlags == eAlphaOnly)
		{
			sLowercaseLayout.EnableButton(e123StrIndex, false);
			sUppercaseLayout.EnableButton(e123StrIndex, false);
			LoadLayout(&sUppercaseLayout);
		} else
		{
			LoadLayout(&sUppercaseLayout);
		}
	}
	return(optionsChanged);
}

/******************************* EnableAllChars *******************************/
bool XKeyboard::EnableAllChars(
	uint16_t		inCharcode,
	SLayoutButton&	inButton)
{
	inButton.enabled = true;
	return(true);
}

/****************************** DisableNonNumbers *****************************/
/*
*	Allows 0 to 9, '.' '-'
*/
bool XKeyboard::DisableNonNumbers(
	uint16_t		inCharcode,
	SLayoutButton&	inButton)
{
	if (inButton.enabled &&
		(inCharcode < '0' || inCharcode > '9') &&
		inCharcode != '-' &&
		inCharcode != '.' &&
		inCharcode != eDoneStrIndex)
	{
		inButton.enabled = false;
	}
	
	return(true);
}

/****************************** Disable16BitChars *****************************/
bool XKeyboard::Disable16BitChars(
	uint16_t		inCharcode,
	SLayoutButton&	inButton)
{
	if (inButton.enabled &&
		inCharcode >= 0x100 &&
		inCharcode != kReturnCharcode &&
		inCharcode != kShiftCharcode)
	{
		inButton.enabled = false;
	}
	return(true);
}


/******************************** ForEachButton *******************************/
void SKeyboardLayout::ForEachButton(
	ForEachButtonCB	inCallback)
{	
	if (initialized && inCallback)
	{
		for (uint16_t row = 0; row < rows; row++)
		{
			const uint16_t*	lytCharcodes = charcodes[row];
			uint16_t		lytButtonsInRow = buttonsInRow[row];
			SLayoutButton*	lytButtons = button[row];
			for (uint16_t btnIdx = 0; btnIdx < lytButtonsInRow; btnIdx++)
			{
				if (inCallback(lytCharcodes[btnIdx], lytButtons[btnIdx]))
				{
					continue;
				}
				return;
			}
		}
	}
}

/******************************** EnableButton ********************************/
SLayoutButton* SKeyboardLayout::EnableButton(
	uint16_t	inCharcode,
	bool		inEnable)
{
	uint16_t		rowIdx, buttonIdx;
	SLayoutButton*	button = FindButton(inCharcode, rowIdx, buttonIdx);
	if (button)
	{
		button->enabled = inEnable;
	}
	return(button);
}

/********************************* FindButton *********************************/
SLayoutButton* SKeyboardLayout::FindButton(
	uint16_t	inCharcode,
	uint16_t&	outRowIdx,
	uint16_t&	outButtonIdx)
{	
	if (initialized)
	{
		for (uint16_t row = 0; row < rows; row++)
		{
			const uint16_t*	lytCharcodes = charcodes[row];
			uint16_t lytButtonsInRow = buttonsInRow[row];
			for (uint16_t btnIdx = 0; btnIdx < lytButtonsInRow; btnIdx++)
			{
				if (inCharcode == lytCharcodes[btnIdx])
				{
					outRowIdx = row;
					outButtonIdx = btnIdx;
					return(&button[row][btnIdx]);
				}
			}
		}
	}
	return(nullptr);
}

