/*
*	XPT2046ZCalibration.h, Copyright Jonathan Mackey 2026
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
#ifndef XPT2046ZCalibration_h
#define XPT2046ZCalibration_h

#include "XView.h"
#include "XPushButton.h"
#include "XFont.h"
#include "MSPeriod.h"

#ifndef __MACH__
class XPT2046;
#endif

class XPT2046ZCalibration : public XView
{
public:
							XPT2046ZCalibration(
								uint16_t				inTag,
								const char*				inSaveButtonText,
								const char*				inCancelButtonText,
								XFont::Font*			inFont,
								XView*					inNextView,
								const char*				inTopInst,
								const char*				inBottomInst);
	XFont*					MakeFontCurrent(void);
	virtual void			DrawSelf(void);
	virtual bool			HitSelf(
								int16_t					inLocalX,
								int16_t					inLocalY)
								{return(mVisible);}
	virtual void			MouseDown(
								int16_t					inGlobalX,
								int16_t					inGlobalY);
#ifdef __MACH__
	void					Start(
								XViewChangedDelegate*	inViewChangedDelegate);
#else
	void					Start(
								XPT2046*				inTouchScreen,
								XViewChangedDelegate*	inViewChangedDelegate);
#endif
	bool					OKToSave(
								int16_t					outZCalibration[9]) const;
	
protected:
	XFont::Font*		mFont;
#ifndef __MACH__
	XPT2046*			mTouchScreen;
#endif
	XViewChangedDelegate*	mViewChangedDelegate;
	XView*				mSavedModalView;
	XPushButton			mSaveButton;
	XPushButton			mCancelButton;
	MSPeriod			mDebouncePeriod;
	int16_t				mZCalibration[9][5];
	const char*			mTopInst;
	const char*			mBottomInst;
	uint16_t			mDataRow;	// 0 to 4
	uint16_t			mStepIndex;	// 0 to 8
	uint16_t			mStepH;
	uint16_t			mStepV;
	uint16_t			mCurrentX;
	uint16_t			mCurrentY;
	uint16_t			mPrevX;
	uint16_t			mPrevY;
	uint16_t			mPrevAdjX;
	uint16_t			mPrevAdjY;
	bool				mOKToSave;
	static const uint16_t	kInset;
	
	enum ESteps
	{
		eStart			= 0,
		eLeft			= 1,
		eTop			= 1,
		eCenter			= 2,
		eRight			= 3,
		eBottom			= 3,
		eNumSteps
	};

	virtual	void			HandleChange(
							XView*						inView,
								uint16_t				inAction = 0);
};
#endif // XPT2046ZCalibration_h
