/*
*	XPT2046ZCalibration.cpp, Copyright Jonathan Mackey 2026
*	Class to align the display with the touch screen.
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

#include "XPT2046ZCalibration.h"
#include "DisplayController.h"
#include "XPT2046.h"
#ifdef __MACH__
#include <iostream>
#endif
#include "XRootView.h"
#include "XDialogBox.h"

const uint16_t	XPT2046ZCalibration::kInset = 20;

/************************** XPT2046ZCalibration **************************/
XPT2046ZCalibration::XPT2046ZCalibration(
	uint16_t		inTag,
	const char*		inSaveButtonText,
	const char*		inCancelButtonText,
	XFont::Font*	inFont,
	XView*			inNextView,
	const char*		inTopInst,
	const char*		inBottomInst)
	: XView(0, 0, 0, 0, inTag, inNextView, nullptr, nullptr, false),
	  mSaveButton(0,0,0,0,inTag+XDialogBox::eOKTagOffset, nullptr, inSaveButtonText, inFont),
	  mCancelButton(0,0,0,0,inTag+XDialogBox::eCancelTagOffset, &mSaveButton, inCancelButtonText, inFont),
	  mFont(inFont), mTopInst(inTopInst), mBottomInst(inBottomInst),
	  mDebouncePeriod(500)
{
	SetSubViews(&mCancelButton);
}

/************************************ Start ***********************************/
#ifndef __MACH__
void XPT2046ZCalibration::Start(
	XPT2046*				inTouchScreen,
	XViewChangedDelegate*	inViewChangedDelegate)
#else
void XPT2046ZCalibration::Start(
	XViewChangedDelegate*	inViewChangedDelegate)
#endif
{
#ifndef __MACH__
	mTouchScreen = inTouchScreen;
	mTouchScreen->PauseZCalibration();
#endif
	mViewChangedDelegate = inViewChangedDelegate;
	mStepH = eStart;
	mStepV = eTop;
	mCurrentX = kInset;
	mCurrentY = kInset;
	mPrevX = mPrevAdjX = 1;
	mPrevY = mPrevAdjY = 1;
	mDataRow = 0;
	mStepIndex = 0;
	mOKToSave = false;
	mSaveButton.Enable(false, false);
	
	mSavedModalView = XRootView::GetInstance()->ModalView();
	XRootView::GetInstance()->SetModalView(this);
	XFont*	xFont = MakeFontCurrent();
	if (xFont)
	{
		DisplayController*	display = xFont->GetDisplay();
		mWidth = display->GetColumns();
		mHeight = display->GetRows();
		
		if (mSaveButton.Width() > mCancelButton.Width())
		{
			mCancelButton.SetWidth(mSaveButton.Width());
		} else
		{
			mSaveButton.SetWidth(mCancelButton.Width());
		}
		mSaveButton.SetOrigin(mWidth - mSaveButton.Width() - 40,
							mHeight - mSaveButton.Height() - 20);
		mCancelButton.SetOrigin(mSaveButton.X() - mCancelButton.Width() - 10,
										mSaveButton.Y());
	}
	mVisible = true;			// Show();
	Draw(0, 0, 0x7FFF, 0x7FFF);	// Show();
	mDebouncePeriod.Start();
}

/******************************** HandleChange ********************************/
void XPT2046ZCalibration::HandleChange(
	XView*		inChangedView,
	uint16_t	inAction)
{
	if (inAction == XControl::eOff &&
		(inChangedView == &mSaveButton || inChangedView == &mCancelButton))
	{
		if (inChangedView == &mSaveButton)
		{
			// Adjust the mean values by subtracting them from 1000
			for (uint32_t i = 0; i < 9; i++)
			{
				mZCalibration[i][2] = 1000 - mZCalibration[i][2];
			}
			// Note that this does not set the touchscreen calibration here.
			// The main loop should check the exit state by calling OKToSave,
			// then call mTouchScreen->SetZCalibration with the Z calibration
			// array returned by OKToSave().
#ifndef __MACH__
		} else
		{
			mTouchScreen->ResumeZCalibration();
#endif
		}
		XRootView::GetInstance()->SetModalView(mSavedModalView);
		mSavedModalView = nullptr;
		Hide();
		if (mViewChangedDelegate)
		{
			mViewChangedDelegate->HandleViewChange(inChangedView, inAction);
		}
	} else
	{
		XView::HandleChange(inChangedView, inAction);
	}
}

/****************************** MakeFontCurrent *******************************/
XFont* XPT2046ZCalibration::MakeFontCurrent(void)
{
	XFont* xFont = nullptr;
	if (mFont)
	{
		xFont = mFont->MakeCurrent();
	}
	return(xFont);
}

/********************************** DrawSelf **********************************/
void XPT2046ZCalibration::DrawSelf(void)
{
	XFont*	xFont = MakeFontCurrent();
	if (xFont)
	{
		DisplayController*	display = xFont->GetDisplay();
		if (mStepH == eStart)
		{
			uint16_t	height;
			uint16_t	width;
			display->SetBGColor(XFont::eBlack);
			xFont->SetTextColor(XFont::eWhite);
			xFont->SetBGTextColor(XFont::eBlack);
			display->Fill();	// Erase display
			// Draw the mInstructions centered on the display
			xFont->MeasureStr(mTopInst, height, width);
			display->MoveTo(35, (display->GetColumns() - width)/2);
			xFont->DrawStr(mTopInst);
			
			xFont->MeasureStr(mBottomInst, height, width);
			display->MoveTo(mHeight - height - 55, (display->GetColumns() - width)/2);
			xFont->DrawStr(mBottomInst);

			// Draw a 30 pixel wide solid red circle at the top left half of
			// the display.
			display->SetFGColor(XFont::eRed);
			display->DrawCircle(30, mHeight/4, 15, 0);

			mStepH = eLeft;
		} 

		
		if (!mOKToSave)
		{
			uint16_t	rows = display->GetRows();
			uint16_t	columns = display->GetColumns();
			uint16_t	newX = 0;
			uint16_t	newY = 0;
			switch (mStepH)
			{
				case eLeft:
					newX = kInset;
					break;
				case eCenter:
					newX = columns/2;
					break;
				case eRight:
					newX = columns-kInset;
					break;
			}
			switch (mStepV)
			{
				case eTop:
					newY = kInset;
					break;
				case eCenter:
					newY = rows/2;
					break;
				case eBottom:
					newY = rows-kInset;
					break;
			}
			// Erase the location where the new circle and white dot will be drawn
			display->FillRect(newX-10, newY-10, kInset, kInset, XFont::eBlack);
			// Draw the new white circle around a white dot
			display->SetFGColor(XFont::eWhite);
			display->DrawCircle(newX, newY, 10, 2);
			// Draw the new white dot
			display->FillRect(newX-1, newY-1, 2, 2, XFont::eWhite);
			// Perform a dummy draw to flush the display command queue. 
			display->FillRect(1, 100, 2, 2, XFont::eBlack);
			mCurrentX = newX;
			mCurrentY = newY;
			mStepIndex++;
			if (mStepIndex == 9)
			{
				mStepIndex = 0;
			}
		}
	}
}

#define XY_SLOP	30
/********************************* MouseDown **********************************/
void XPT2046ZCalibration::MouseDown(
	int16_t	inGlobalX,
	int16_t	inGlobalY)
{
	XFont*	xFont = MakeFontCurrent();
	/*
	*	The goal is to not respond to every mouse down, especially when you're
	*	trying to precisely touch the tiny white dot.  Pressing slowly/precisely
	*	will generate multiple hits.  The mDebouncePeriod limits this to one hit
	*	every mDebouncePeriod.  This should give you enough time to ease the
	*	pressure off the touch screen without having to process multiple hits.
	*
	*	If the font is current AND
	*	five laps around the display isn't complete AND
	*	the press is not a multiple hit AND
	*	the press is within the displayed white circle THEN
	*	process the mouse down.
	*/
	if (xFont && !mOKToSave && mDebouncePeriod.Passed() &&
		inGlobalX >= mCurrentX-XY_SLOP && inGlobalX < mCurrentX+XY_SLOP &&
		inGlobalY >= mCurrentY-XY_SLOP && inGlobalY < mCurrentY+XY_SLOP)
	{
		bool		updateLapCount = mStepH == eStart;
		uint16_t	x,y,z;
		DisplayController*	display = xFont->GetDisplay();
#ifndef __MACH__
		Serial.printf("OK %hd >= %hd < %hd,  %hd >= %hd < %hd\n",
			mCurrentX-XY_SLOP, inGlobalX, mCurrentX+XY_SLOP,
			mCurrentY-XY_SLOP, inGlobalY, mCurrentY+XY_SLOP);
#endif

#ifndef __MACH__
		/*
		*	Update the calibration by adding the current z value touched.
		*/
		mTouchScreen->Read(x, y, z);
#else
		x = inGlobalX;
		y = inGlobalY;
		z = 500 + inGlobalX + inGlobalY;
#endif
		{
			uint16_t	count = mDataRow;
			XPT2046::InsertValue(z, 5, mZCalibration[mStepIndex], count);
		}
		mStepH++;
		if (mStepH == eNumSteps)
		{
			mStepH = eLeft;
			mStepV++;
			if (mStepV == eNumSteps)
			{
				mStepV = eTop;
				mDataRow++;
				updateLapCount = true;
				mOKToSave = mDataRow == 5;
				if (mOKToSave)
				{
					mSaveButton.Enable();
				}
			}
		}
		{
			// Erase the current circle and white dot
			display->SetFGColor(XFont::eBlack);
			display->DrawCircle(mCurrentX, mCurrentY, 10, 2);
			display->FillRect(mPrevX-1, mPrevY-1, 2, 2, XFont::eBlack);
			mPrevX = mCurrentX;
			mPrevY = mCurrentY;
			// Erase the previous calculated hit point (magenta dot)
			display->FillRect(mPrevAdjX-1, mPrevAdjY-1, 2, 2, XFont::eBlack);
			if (!mOKToSave)
			{
				// If the new calculated hit point is on the display THEN
				// Draw the new calculated hit point.
				if (x > 1 && x < display->GetColumns() &&
					y > 1 && y < display->GetRows())
				{
					display->FillRect(x-1, y-1, 2, 2, XFont::eMagenta);
					mPrevAdjX = x;
					mPrevAdjY = y;
				}
				xFont->SetBGTextColor(XFont::eRed);
			} else
			{
				display->FillRect(mPrevX-1, mPrevY-1, 2, 2, XFont::eBlack);
				display->SetFGColor(XFont::eGreen);
				display->DrawCircle(30, mHeight/4, 15, 0);
				xFont->SetBGTextColor(XFont::eGreen);
			}
			if (updateLapCount)
			{
				// Draw the lap count within the green or red circle.
				xFont->LoadGlyph('0');
				display->MoveTo((mHeight/4)-xFont->Glyph().y - (xFont->Glyph().rows/2),
								30 - (xFont->Glyph().columns)/2);
				xFont->SetTextColor(XFont::eWhite);
				xFont->DrawCharcode('0' + mDataRow);
			}
			// Perform a dummy draw to flush the display command queue. 
			display->FillRect(1, 100, 2, 2, XFont::eBlack);
		}
		DrawSelf();
		mDebouncePeriod.Start();
	} else if (xFont && !mOKToSave && mDebouncePeriod.Passed())
	{
#ifndef __MACH__
		Serial.printf("Expected %hd >= %hd < %hd,  %hd >= %hd < %hd\n",
			mCurrentX-XY_SLOP, inGlobalX, mCurrentX+XY_SLOP,
			mCurrentY-XY_SLOP, inGlobalY, mCurrentY+XY_SLOP);
#endif
	}
}

#if 0
/**************************** AddZCalibrationValue ****************************/
/*
*	Add a Z value while keeping the row values sorted.  It's assumed that for
*	the next call for mStepIndex, mDataRow will be mDataRow+1.
*
*	The combination of mDataRow & mStepIndex will be called only once.
*
*	mDataRow, mStepIndex,...
*	0, 0, 0, 1, 0, 2... 0, 8
*	1, 0, 1, 1, 1, 2... 1, 8
*	2, 0, 2, 1, 2, 2... 2, 8
*	3, 0, 3, 1, 3, 2... 3, 8
*	4, 0, 4, 1, 4, 2... 4, 8
*
*	After the five data rows are populated, mDataRow 2 contains the set of mean
*	values of all points.  The mean value is subtracted from 1000 as the Z
*	calibration value for each point.  In regular use, bilinear interpolation is
*	applied to each raw Z value read.
*/
void XPT2046ZCalibration::AddZCalibrationValue(
	int16_t		inZValue)
{
	if (mDataRow < 5 && mStepIndex < 9)
	{
		if (mDataRow)
		{
			int16_t leftIndex = 0;
			int16_t current = 0;
			int16_t rightIndex = mDataRow-1;	// Index of last value in array

			while (leftIndex <= rightIndex)
			{
				current = (leftIndex + rightIndex) / 2;
				
				int16_t	comparison = mZCalibration[current][mStepIndex] - inZValue;
				if (comparison == 0)
				{
					break;
				} else if (comparison > 0)
				{
					rightIndex = current - 1;
				} else
				{
					leftIndex = current + 1;
				}
			}

			/*
			*	Insert inZValue at leftIndex
			*/
			{
				/*
				*	If the values need to be shifted to make room for the value
				*	being inserted ...
				*/
				if (leftIndex < mDataRow)
				{
					for (uint32_t j = mDataRow; j > leftIndex; j--)
					{
						mZCalibration[j][mStepIndex] = mZCalibration[j-1][mStepIndex];
					}
				}
				mZCalibration[leftIndex][mStepIndex] = inZValue;
			}
		} else
		{
			mZCalibration[mDataRow][mStepIndex] = inZValue;
		}
	}
}
#endif

/********************************** OKToSave **********************************/
bool XPT2046ZCalibration::OKToSave(
	int16_t	outZCalibration[9]) const
{
	for (uint32_t i = 0; i < 9; i++)
	{
		outZCalibration[i] = mZCalibration[i][2];
	}
#ifdef __MACH__
	for (uint32_t row = 0; row < 9; row++)
	{
		fprintf(stderr, "[%d]\t", row);
		for (uint32_t col = 0; col < 5; col++)
		{
			fprintf(stderr, "%hd ", mZCalibration[row][col]);
		}
		fprintf(stderr, "\n");
	}
#else
	for (uint32_t row = 0; row < 9; row++)
	{
		Serial.printf("[%d]\t", row);
		for (uint32_t col = 0; col < 5; col++)
		{
			Serial.printf("%d ", (int32_t)mZCalibration[row][col]);
		}
		Serial.printf("\n");
	}
#endif
	return(mOKToSave);
}
