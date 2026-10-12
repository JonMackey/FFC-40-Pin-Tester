/*
*	FFCTesterSTM32.h, Copyright Jonathan Mackey 2026
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
#ifndef FFCTesterSTM32_h
#define FFCTesterSTM32_h

#include "Config.h"
#include "TSettings.h"
#include "AT24C.h"
#include "AT24CDataStream.h"
#include "TFT_ILI9488.h"
#include "XPT2046.h"
#include "XDialogBox.h"
#include "MSPeriod.h"
#include "STM32UnixRTC.h"
#include "PinConfigs.h"

class TwoWire;

class FFCTesterSTM32 : public XViewChangedDelegate,
								public XValidatorDelegate
{
public:
							FFCTesterSTM32(void);
		
	virtual void			begin(void);
	virtual bool			Update(void);

	virtual void			HandleViewChange(
								XView*					inView,
								uint16_t				inAction);
	virtual bool			ValuesAreValid(
								XDialogBox*				inDialog);
protected:
	XView*			mHitView;
	TFT_ILI9488		mDisplay;
	XPT2046			mTouchScreen;
	AT24C			mPreferences;
	AT24CDataStream	mPinConfigsDataStream;
	PinConfigs		mPinConfigs;
	bool			mDefPinGridIsDirty;
	bool			mDisplaySleeping;
	bool			mButtonPressed;
	bool			mClearButtonPressed;
	MSPeriod		mButtonDebouncePeriod;
	bool			mSendDebugStrings;
	uint32_t		mButtonPinState;
	uint16_t		mX, mY;
	HardwareTimer	mCaretTimer;
	
protected:
	bool					NoModalDialogDisplayed(void) const;
	void					ShowMainView(void);
	void					ShowManagePinsView(void);
	void					ShowUtilitiesView(void);
	void					HandleConfigsLoaded(void);
	void					SavePinConfigsToSD(
								bool					inMsgToScreen);
	void					LoadPinConfigsFromSD(
								bool					inMerge,
								bool					inMsgToScreen);
	void					SaveTSettingsToSD(
								bool					inMsgToScreen = false);
	void					LoadTSettingsFromSD(
								bool					inMsgToScreen = false);
	void					UpdateCurrentView(void);
	void					WakeUp(void);
	void					GoToSleep(void);
	void					ButtonPressedISR(void);
	void					CheckButtons(void);
	void					ClearButtonPressedISR(void);
	void					CheckClearButton(void);
	static void				CaretTimerISR(void);
	void					LoadGeneralPrefsFromFlash(void);
	void					SaveZCalibrationValues(void);
	void					HandleCancelButtonPressed(void);
	void					HandleEnterButtonPressed(void);
	void					LoadSelectedConfig(void);
	void					RemoveSelectedConfig(void);
	void					SaveConfig(void);
	bool					PinsNotDirtyCanContinue(
								uint16_t				inReference);
	void					GetCurrentPinConfigName(
								char*					outName);
	void					SetCurrentPinConfigName(
								const char*				inName);
	void					SwapPinsCheckboxChanged(void);
	bool					GetPinOrderPref(void);
	void 					SetPinOrderPref(
								bool					inSwapPinOrder);
};

#endif // FFCTesterSTM32_h
