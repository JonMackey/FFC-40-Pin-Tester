/*
*	XKeyboard.h, Copyright Jonathan Mackey 2026
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
#ifndef XKeyboard_h
#define XKeyboard_h

#include "XView.h"
#include "XFont.h"
struct SLayoutButton
{
	uint16_t	left;
	uint16_t	width;
	enum EButtonFont
	{
		eNormalFont,
		eSmallFont
	};
	EButtonFont	font;
	bool		enabled;
};

/*
*	Callback passed to SKeyboardLayout::ForEachButton.  This callback is called
*	for each button.  The callback returns false if the search isn't complete.
*/
typedef	bool (*ForEachButtonCB)(uint16_t inCharcode, SLayoutButton& inButton);

struct SKeyboardLayout
{
	enum {eMaxLayoutRows = 4, eMaxButtonsPerRow = 10};
	uint32_t		rows;
	const uint16_t*	charcodes[eMaxLayoutRows];
	const uint8_t*	layoutRows[eMaxLayoutRows];
	const char**	multiCharStrs;
	bool			initialized;	// true if rowTop & buttonLeft are valid
	uint16_t		rowTop[eMaxLayoutRows];
	uint16_t		buttonsInRow[eMaxLayoutRows];
					// eMaxLayoutRows, 2 values per button, eMaxButtonsPerRow
	SLayoutButton	button[eMaxLayoutRows][eMaxButtonsPerRow];
	void					ForEachButton(
								ForEachButtonCB			inCallback);
	SLayoutButton*			EnableButton(
								uint16_t				inCharcode,
								bool					inEnable);
	SLayoutButton*			FindButton(
								uint16_t				inCharcode,
								uint16_t&				outRowIdx,
								uint16_t&				outButtonIdx);
};

#if 0
/*
*	XKeyboard is currently hardcoded to the standard 4 US layouts.  At some
*	point it might be useful to allow for custom layouts, including custom
*	drawing.
*/
struct SKeyboardLayoutGroup
{
	enum {eMaxLayouts = 4};
	uint32_t		numLayouts;
	SKeyboardLayout	layout[eMaxLayouts];
};
#endif

class XTextField;

class XKeyboard : public XView
{
public:
							XKeyboard(
								uint16_t				inTag,
								XView*					inNextView,
								XFont::Font*			inNormalFont,
								XFont::Font*			inSmallFont,
								uint16_t				inFGColor = XFont::eWhite,
								uint16_t				inBGColor =  XFont::eBlack);
	virtual void			DrawSelf(void);
	virtual XView*			HitTest(
								int16_t					inX,
								int16_t					inY);
	virtual void			MouseDown(
								int16_t					inGlobalX,
								int16_t					inGlobalY);
	void					ToggleCaret(void);

	void					LoadLayout(
								SKeyboardLayout*		inLayout);
	void					UpdateDeleteButton(
								bool					inDrawChange);
	virtual void			Hide(void);
	bool					ShowFor(
								XTextField*				inTextField);
	virtual uint16_t		ID(void) const
								{return(XViewID::kXKeyboardID);}
	static bool				IsKeyboard(
								uint16_t				inID)
								{return(inID == XViewID::kXKeyboardID);}
	static XKeyboard*		GetInstance(void)
								{return(sInstance);}
	enum EKeyboardDims
	{
		eSmallWidth,
		eMediumWidth1,
		eMediumWidth2,
		eWideWidth,
		eSpaceBarWidth1,
		eSpaceBarWidth2,
		eHeight,
		eVSpaceBetween,
		eHSpaceBetween,
		eVBorderSpace,
		eHBorderSpace,
		eBorderCornerRadius,
		eButtonCornerRadius,
		eNumDimIndexes
	};
	
	enum EKeyboardOptions
	{
		eAllCharcodes		= 0,
		eNumbersOnly		= 1,	// Allows '.' && '-'
		eIntegersOnly		= 3,	// Allows '-'
		eDisableDecPt		= 2,
		eUNumbersOnly		= 5,	// Allows '.'
		eUIntegersOnly		= 7,
		eDisableMinusSign	= 4,
		eUppercaseOnly		= 8,
		eLowercaseOnly		= 0x10,
		eAlphaOnly			= 0x20,
		e8BitOnly			= 0x40,
	};

protected:
	XFont::Font*		mNormalFont;
	XFont::Font*		mSmallFont;
	// At some point XTextField may be changed to a mixin class, XKeyboardClient
	// or something, so that some view other than XTextField can use a keyboard.
	XTextField*			mTextField;
	XView*				mSavedModalView;
	uint16_t			mFGColor;
	uint16_t			mBGColor;
	uint16_t			mScaledDim[eNumDimIndexes];
	uint16_t			mAppliedOptions;
	SKeyboardLayout*	mCurrentLayout;
	static XKeyboard*		sInstance;
	static SKeyboardLayout	sUppercaseLayout;
	static SKeyboardLayout	sLowercaseLayout;
	static SKeyboardLayout	sNumbersLayout;
	static SKeyboardLayout	sSpecialCharsLayout;

	XFont*					MakeFontCurrent(
								SLayoutButton::EButtonFont	inEButtonFont);
	void					DrawButtons(
								uint16_t				inRow = 0,
								uint16_t				inButtonIdx = 0,
								bool					inDrawAllButtons = true);
	bool					ApplyOptions(
								uint16_t				inOptionFlags);
	static bool				EnableAllChars(
								uint16_t				inCharcode,
								SLayoutButton&			inButton);
	static bool				DisableNonNumbers(
								uint16_t				inCharcode,
								SLayoutButton&			inButton);
	static bool				Disable16BitChars(
								uint16_t				inCharcode,
								SLayoutButton&			inButton);
};
#endif // XKeyboard_h
