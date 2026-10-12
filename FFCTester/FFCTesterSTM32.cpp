/*
*	FFCTesterSTM32.cpp, Copyright Jonathan Mackey 2026
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
#include <Arduino.h>
#include "FFCTesterSTM32.h"
#include "SdFat.h"
#include <Wire.h>
#include "AT24CDataStream.h"
#include "SerialUtils.h"

#if 1
XFont	xFont;
// 8-bit fonts (antialiased)
#define UI20ptFont	MyriadPro_Regular_20::font
#include "MyriadPro-Regular_20.h"
#define UI13ptFont	MyriadPro_Regular_13::font
#include "MyriadPro-Regular_13.h"
#endif
#include "TXViews.h"
static const char kTSettingsPath[] = "TSettings.txt";
static const char kPinConfigsPath[] = "PinConfigs.csv";
volatile bool		sCaretNeedsUpdate;

/***************************** FFCTesterSTM32 *****************************/
FFCTesterSTM32::FFCTesterSTM32(void)
  : mDisplay(Config::kDispDCPin, Config::kDispResetPin,
						Config::kDispCSPin, Config::kBacklightPin),
	mPreferences(Config::kAT24CDeviceAddr, Config::kAT24CDeviceCapacity),
	mPinConfigsDataStream(&mPreferences, (void *)Config::kPinConfigsAddr, Config::kPinConfigsSize),
    mTouchScreen(Config::kTouchCSPin, Config::kTouchIRQPin,
			Config::kDisplayHeight, Config::kDisplayWidth,
			0, 0, 0, 0, Config::kInvertTouchX, Config::kInvertTouchY),
	mButtonDebouncePeriod(DEBOUNCE_DELAY), mButtonPressed(false),
	mClearButtonPressed(false), mDefPinGridIsDirty(false)
{
}

/************************************ begin ***********************************/
void FFCTesterSTM32::begin(void)
{
	pinMode(Config::kCancelBtnPin, INPUT_PULLUP);
	pinMode(Config::kEnterBtnPin, INPUT_PULLUP);
	pinMode(Config::kClearBtnPin, INPUT_PULLUP);

	// Define button pin interrupt routine
	// The buttons are used to setup the touchscreen.
	//
	// Note that the doc for attachInterrupt recommends passing the pin through
	// digitalPinToInterrupt(), but all digitalPinToInterrupt does is validate
	// the passed pin using digitalPinToPinName(), and if it's not NC it
	// returns the same pin value passed.  attachInterrupt passes the pin to
	// digitalPinToPinName in the first line, so passing the pin through to 
	// digitalPinToInterrupt is pointless.
	attachInterrupt(Config::kCancelBtnPin,
		std::bind(&FFCTesterSTM32::ButtonPressedISR, this), FALLING);
	attachInterrupt(Config::kEnterBtnPin,
		std::bind(&FFCTesterSTM32::ButtonPressedISR, this), FALLING);
	attachInterrupt(Config::kClearBtnPin,
		std::bind(&FFCTesterSTM32::ClearButtonPressedISR, this), FALLING);

	pinTesterGrid.begin(Config::kPortPinMap);
	
	/*
	*	Configure the pins not associated with the FFC connector or any other
	*	class that configures its own pins.
	*/
	{
		// GPIO Init structure is {Pin, Mode, Pull, Speed, Alternate}
		GPIO_InitTypeDef gpioInit = {0, GPIO_MODE_INPUT, GPIO_PULLUP, 0, 0};

		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_GPIOC_CLK_ENABLE();
		__HAL_RCC_GPIOE_CLK_ENABLE();

		gpioInit.Pin = 0x0003;		// Unused: PA1, PA0
		HAL_GPIO_Init(GPIOA, &gpioInit);
		gpioInit.Pin = 0x2006;		// Unused: PC13, PC2, PC1
		HAL_GPIO_Init(GPIOC, &gpioInit);
		gpioInit.Pin = 0x0070;		// Unused: PE6, PE5, PE4
		HAL_GPIO_Init(GPIOE, &gpioInit);
	}

	/*
	*	The status LEDs
	*/
	{
		pinMode(Config::kPA4LEDPin, OUTPUT);
		pinMode(Config::kPA5LEDPin, OUTPUT);
		pinMode(Config::kPA6LEDPin, OUTPUT);
		pinMode(Config::kPA7LEDPin, OUTPUT);
		pinMode(Config::kPA8LEDPin, OUTPUT);
		pinMode(Config::kPA9LEDPin, OUTPUT);
		pinMode(Config::kPA10LEDPin, OUTPUT);
		digitalWrite(Config::kPA4LEDPin, LOW);
		digitalWrite(Config::kPA5LEDPin, LOW);
		digitalWrite(Config::kPA6LEDPin, LOW);
		digitalWrite(Config::kPA7LEDPin, LOW);
		digitalWrite(Config::kPA8LEDPin, LOW);
		digitalWrite(Config::kPA9LEDPin, LOW);
		digitalWrite(Config::kPA10LEDPin, LOW);
	}

	Wire.begin();
	SPI.begin();

	// Generate a 2Hz timer used to flash the insertion caret in a text field
	{
		mCaretTimer.setup(TIM4);
		mCaretTimer.setOverflow(2, HERTZ_FORMAT);
		mCaretTimer.attachInterrupt(&FFCTesterSTM32::CaretTimerISR);
		mCaretTimer.resume();
	}
		
	STM32UnixRTC::RTCInit();
	{
		RCC_OscInitTypeDef RCC_OscInitStruct = {0};
		RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
		uint32_t	pFLatency;
		HAL_RCC_GetOscConfig(&RCC_OscInitStruct);
		HAL_RCC_GetClockConfig(&RCC_ClkInitStruct, &pFLatency);
		
		Serial.printf("OscillatorType = ");
		if (RCC_OscInitStruct.OscillatorType & RCC_OSCILLATORTYPE_HSE)
		{
			Serial.printf("RCC_OSCILLATORTYPE_HSE");
		}
		if (RCC_OscInitStruct.OscillatorType & RCC_OSCILLATORTYPE_LSE)
		{
			Serial.printf("|RCC_OSCILLATORTYPE_LSE");
		}
		Serial.printf("\n");
		Serial.printf("RCC_OscInitStruct.LSEState = %s\n", RCC_OscInitStruct.LSEState == RCC_LSE_ON ? "RCC_LSE_ON":"RCC_LSE_OFF");
	}
	pinMode(Config::kSDDetectPin, INPUT_PULLUP);
	pinMode(Config::kSDSelectPin, OUTPUT);
	digitalWrite(Config::kSDSelectPin, HIGH);	// Deselect the SD card.
	{
		Wire.beginTransmission(Config::kAT24CDeviceAddr);
		bool	at24C64IsResp = Wire.endTransmission(true) == 0;
	
		/*
		*	Load the preferences and pin configurations
		*	If the EEPROM is responding...
		*/
		if (at24C64IsResp)
		{
			LoadGeneralPrefsFromFlash();
			mPinConfigs.begin(&mPinConfigsDataStream);
			configItemProvider.SetPinConfigs(&mPinConfigs);
			if (mPinConfigs.GetCount())
			{
				char configName[64];
				GetCurrentPinConfigName(configName);
				if (mPinConfigs.GoToNamedLink(configName))
				{
					mPinConfigs.GetCurrentConfig(sDefPinGridColorIdx);
					loadConfigPopUp.SetSelectedItemString(configName, false);
					pinSetNameTextField.SetUTF8Str(configName, false);
				}
			}
			{
				bool	swapPinOrder = GetPinOrderPref();
				pinTesterGrid.InvertPinOrder(swapPinOrder);
				swapPinsCheckbox.SetState(swapPinOrder ? XControl::eOn:XControl::eOff, false);
			}
			// sDefPinGridColorIdx is also the current pin configuration.
			// Only the pointer to sDefPinGridColorIdx is stored so it only
			// needs to be set once.
			pinTesterGrid.LoadPinConfig(sDefPinGridColorIdx);
		} else
		{
			Serial.printf("AT24C64 is not responding.\n");
		}
	}
	Serial.printf("MCU Frequency = %d\n", F_CPU);
	
	/*
	*	Setup the display...
	*/
	mTouchScreen.begin(Config::kDisplayRotation);
	mDisplay.begin(Config::kDisplayRotation);	// Init TFT

	
	rootView.SetSize(Config::kDisplayHeight, Config::kDisplayWidth);
	rootView.SetDisplay(&mDisplay);
	rootView.SetViewChangedDelegate(this);
	alertDialog.SetViewChangedDelegate(this);
	warningDialog.SetViewChangedDelegate(this);
	warningDialog.SetMinDialogSize();
	xFont.SetDisplay(&mDisplay, &UI20ptFont);	// To initialize mDisplay of xFont
	
	ShowMainView();
}

/*************************** NoModalDialogDisplayed ***************************/
bool FFCTesterSTM32::NoModalDialogDisplayed(void) const
{
	return(rootView.ModalView() == nullptr);
}

/*********************************** Update ***********************************/
/*
*	Called from loop()
*/
bool FFCTesterSTM32::Update(void)
{	
	if (mTouchScreen.PenStateChanged())
	{
		if (mTouchScreen.PenIsDown())
		{
			if (!mDisplaySleeping)
			{
				//Serial.printf("D\n");
				UnixTime::ResetSleepTime();
				uint16_t	z;
				bool isValid = mTouchScreen.Read(mX, mY, z);
				if (mTouchScreen.ZCalibrated() && z < 800)
				{
					Serial.printf("z = %hd\n", z);
				}/* else
				{
					Serial.printf("isValid %c, zCal = %c, x = %d, y = %d, z = %d\n", isValid ? 'T':'F', 
						mTouchScreen.ZCalibrated() ? 'T':'F', (int32_t)mX, (int32_t)mY, (int32_t)z);
				}
				*/
			
				/*
				*	If the pen is held down hard enough THEN
				*	process the click, otherwise ignore it.
				*/
				if (isValid &&
					(mTouchScreen.ZCalibrated() == false ||
						z >= 800))
				{
					mHitView = rootView.HitTest(mX, mY);
					if (mHitView)
					{
						mHitView->MouseDown(mX,mY);
					}
				}
			} else
			{
				WakeUp();
			}
		} else	// Else, pen is up
		{
			//Serial.printf("U\n");
			if (mHitView)
			{
				mHitView->MouseUp(mX, mY);
				mHitView = nullptr;
			}
		}
	}

	CheckButtons();	// Buttons are used to setup the touchscreen.
	CheckClearButton();
	pinTesterGrid.Update();
	
	if (XKeyboard::GetInstance()->IsVisible() &&
		sCaretNeedsUpdate)
	{
		sCaretNeedsUpdate = false;
		XKeyboard::GetInstance()->ToggleCaret();
	}
	
	/*
	*	If the display isn't sleeping...
	*/
	if (!mDisplaySleeping)
	{
		UpdateCurrentView();
		
		if (UnixTime::TimeToSleep())
		{
			if (NoModalDialogDisplayed())
			{
				GoToSleep();
			} else
			{
				UnixTime::ResetSleepTime();
			}
		}
	/*} else if (mMotorIsRunning)
	{
		WakeUp();*/
	}

	/*
	*	Serial commands
	*/
	if (Serial.available())
	{
		switch (Serial.read())
		{
			case '>':
				Serial.printf(">\n");
				// Set the time.  A hexadecimal ASCII UNIX time follows
						// Use >65920071 for all-fields-change test (15s delay)
				UnixTime::SetUnixTimeFromSerial();
				STM32UnixRTC::SyncRTCToTime();
				break;
			case 'e':	// Start touchscreen alignment
				Serial.flush();
				HandleEnterButtonPressed();
				break;
			case 'c':	// Cancel touchscreen alignment
				Serial.flush();
				HandleCancelButtonPressed();
				break;
			case 's':
				// This command attempts to save the current settings to
				// TSettings.txt on the SD root.
				// This will overwrite any existing TSettings.txt
				Serial.flush();
				SaveTSettingsToSD();
				break;
			case 'l':
				// This command attempts to load the file TSettings.txt from
				// the SD root.
				Serial.flush();
				Serial.printf("Loading Settings...\n");
				LoadTSettingsFromSD();
				//prefs.tsMinMax: 255, 3919, 223, 3779
				//prefs.tsZCalibration: 348, 24, 362, 762, -109, -116, 490, -259, -199,
				break;
			case 'S':
				// This command attempts to save the current pin settings to
				// PinConfigs.csv on the SD root.
				// This will overwrite any existing TSettings.txt
				SavePinConfigsToSD(false);
			case 'L':
				// This command attempts to load the current pin settings from
				// PinConfigs.csv on the SD root.
				// This will overwrite any existing pin configurations.
				LoadPinConfigsFromSD(false, false);
				break;
			case 'M':
				// This command attempts to merge the current pin settings from
				// PinConfigs.csv on the SD root with existing pin
				// configurations.  Existing configurations with the same name
				// will be updated to the configuration in PinConfigs.csv
				LoadPinConfigsFromSD(true, false);
				break;
		}
	}

	return(false);
}

/*********************************** WakeUp ***********************************/
/*
*	Wakup the display from sleep and/or keep it awake if not sleeping.
*/
void FFCTesterSTM32::WakeUp(void)
{
	if (mDisplaySleeping)
	{
		mDisplaySleeping = false;
		mDisplay.WakeUp();
		rootView.Draw(0, 0, 999, 999);
	}
	UnixTime::ResetSleepTime();
}

/********************************* GoToSleep **********************************/
/*
*	Puts the display to sleep.
*/
void FFCTesterSTM32::GoToSleep(void)
{
	if (!mDisplaySleeping)
	{
		mDisplay.Fill();
		mDisplay.Sleep();
		mDisplaySleeping = true;
	}
}

/******************************** CheckButtons ********************************/
/*
*	CheckButtons is used to setup the display orientation and the touch screen
*	alignment values. 
*/
void FFCTesterSTM32::CheckButtons(void)
{
	if (mButtonPressed)
	{
		uint32_t	btnPinsState = (~GPIOE->IDR) & Config::kPINBtnMask;
		if (btnPinsState)
		{
			/*
			*	If the state hasn't changed since last check
			*/
			if (mButtonPinState == btnPinsState)
			{
				/*
				*	Wakeup the display when any key is pressed.
				*/
				WakeUp();
				/*
				*	If a debounce period has passed
				*/
				if (mButtonDebouncePeriod.Passed())
				{
					UnixTime::ResetSleepTime();
					mButtonPressed = false;
					mButtonPinState = 0xFF;
					switch (btnPinsState)
					{
						case Config::kCancelBtnMask:	// Cancel button pressed
							HandleCancelButtonPressed();
							break;
						case Config::kEnterBtnMask:	// Enter button pressed
							HandleEnterButtonPressed();
							break;
						default:
							mButtonDebouncePeriod.Start();
							break;
					}
				}
			} else
			{
				mButtonPinState = btnPinsState;
				mButtonDebouncePeriod.Start();
			}
		}
	}

}

/****************************** CheckClearButton ******************************/
void FFCTesterSTM32::CheckClearButton(void)
{
	if (mClearButtonPressed)
	{
		uint32_t	btnPinsState = (~GPIOC->IDR) & Config::kClearBtnMask;
		if (btnPinsState)
		{
			/*
			*	If the state hasn't changed since last check
			*/
			if (mButtonPinState == btnPinsState)
			{
				/*
				*	Wakeup the display when any key is pressed.
				*/
				WakeUp();
				/*
				*	If a debounce period has passed
				*/
				if (mButtonDebouncePeriod.Passed())
				{
					UnixTime::ResetSleepTime();
					mClearButtonPressed = false;
					mButtonPinState = 0xFF;
					switch (btnPinsState)
					{
						case Config::kClearBtnMask:	// Clear button pressed
							pinTesterGrid.Clear(mainView.IsVisible());
							break;

						default:
							mButtonDebouncePeriod.Start();
							break;
					}
				}
			} else
			{
				mButtonPinState = btnPinsState;
				mButtonDebouncePeriod.Start();
			}
		}
	}

}

/****************************** UpdateCurrentView *****************************/
/*
*	Called from Update()
*/
void FFCTesterSTM32::UpdateCurrentView(void)
{
	/*
	*	The mainMenuBtn is located in top right corner of the display.
	*
	*	When one of the base views are made visible, they become the superview
	*	of the mainMenuBtn by calling XView::AttachView().
	*
	*	To get the tag of the current base view you just need to get the tag of
	*	the superview of the mainMenuBtn.
	*/
	switch(mainMenuBtn.SuperView()->Tag())
	{
		case eMainViewTag:
			if (NoModalDialogDisplayed() &&
				UnixTime::TimeChanged())
			{
				UnixTime::ResetTimeChanged();
				infoDateValueField.SetValue(UnixTime::Time());
			}
			break;
		case eUtilitiesViewTag:
			if (UnixTime::TimeChanged())
			{
				UnixTime::ResetTimeChanged();
				dateValueField.IncrementTime(NoModalDialogDisplayed());
			}
			break;
	}
}

/************************** HandleCancelButtonPressed *************************/
void FFCTesterSTM32::HandleCancelButtonPressed(void)
{
	if (touchScreenAlignment.IsVisible())
	{
		// Stop and cancel/quit (restore old alignment)
		Serial.printf(".TS  Align cancelled\n");
		mainView.SetVisible(true);
		touchScreenAlignment.Stop(true);
	}
}

/************************** HandleEnterButtonPressed **************************/
void FFCTesterSTM32::HandleEnterButtonPressed(void)
{
	if (touchScreenAlignment.IsVisible())
	{
		if (touchScreenAlignment.OKToSave())
		{
			Serial.printf(".TS alignment saved\n");
			// Stop alignment and save
			mainView.SetVisible(true);
			touchScreenAlignment.Stop(false);
			uint16_t	minMax[4];
			mTouchScreen.GetMinMax(minMax);
			mPreferences.Write(offsetof(Config::SGeneralPreferences, tsMinMax), sizeof(minMax), (const uint8_t*)minMax);
		} else
		{
			Serial.printf(".TS alignment not saved, not enough points entered\n");
		}
	} else if (NoModalDialogDisplayed())
	{
		mainView.SetVisible(false);
		touchScreenAlignment.Start(&mTouchScreen);
	}
}

/****************************** ButtonPressedISR ******************************/
/*
*	Called via an interrupt.  The button press is handled in CheckButtons()
*	after debouncing.
*/
void FFCTesterSTM32::ButtonPressedISR(void)
{
	mButtonPressed = true;
}
	
/****************************** ClearButtonPressedISR ******************************/
/*
*	Called via an interrupt.  The button press is handled in CheckClearButton()
*	after debouncing.
*/
void FFCTesterSTM32::ClearButtonPressedISR(void)
{
	mClearButtonPressed = true;
}
	
/******************************* ValuesAreValid *******************************/
/*
*	ValuesAreValid is a member of the XValidatorDelegate mixin class.
*	It's called when a dialog's OK button is pressed.
*	True is returned if the values are valid and the dialog can be dismissed,
*	otherwise the dialog either just stays visible (no action) or the dialog
*	displays an error explaining why the dialog can't be dismissed.
*/
bool FFCTesterSTM32::ValuesAreValid(
	XDialogBox*	inDialog)
{
	bool	valuesAreValid = false;
	if (inDialog)
	{
	/*	switch (inDialog->Tag())
		{
			case eCutKeyDialogTag:
			{
				SKeySpec::EErrorCode	errCode = pinsValueField.ValuesAreValid();
				valuesAreValid = errCode == SKeySpec::eNoErr;
				if (!valuesAreValid)
				{
					const char*	errStr = nullptr;
					switch((uint16_t)errCode)
					{
						case SKeySpec::eMACSErr:
							errStr = kMACSMessageStr;
							break;
						case SKeySpec::eDepthMaxErr:
							errStr = kDepthErrMessageStr;
							break;
						case SKeySpec::ePinIndexErr:
							errStr = kPinIndexMessageStr;
							break;
					}
					if (errStr)
					{
						warningDialog.DoMessage(errStr);
					}
				}
				break;
			}
			case eSetupKeyHolderDialogTag:
			{
				if (mController.isRunning() == false)
				{
					warningDialog.DoMessage(kSteppersBusyStr);
				}
				break;
			}
			
		}*/
	}
	return(valuesAreValid);
}

/****************************** HandleViewChange ******************************/
void FFCTesterSTM32::HandleViewChange(
	XView*		inView,
	uint16_t	inAction)
{
	if (inView)
	{
	#if 0
		Serial.printf("Change: Tag = %hd, Action = %hd\n", inView->Tag(), inAction);
	#endif
		switch (inView->Tag())
		{
			case eMainMenuTag:
				switch (inAction)
				{
					case eMainMenuItem:
						if (mainView.IsVisible() == false &&
							PinsNotDirtyCanContinue(eMainMenuItem))
						{
							ShowMainView();
						}
						break;
					case eManagePinsMenuItem:
						if (managePinsView.IsVisible() == false)
						{
							ShowManagePinsView();
						}
						break;
					case eUtilitiesMenuItem:
						if (utilitiesView.IsVisible() == false &&
							PinsNotDirtyCanContinue(eUtilitiesMenuItem))
						{
							ShowUtilitiesView();
						}
						break;
					case eAboutMenuItem:
						aboutBox.Show();
						break;
				}
				break;
			case eLoadConfigPopUpTag:
			{
				if (inAction == XPopUpButton::eMenuItemTagChanged)
				{
					if (mDefPinGridIsDirty)
					{
						alertDialog.DoMessage("The current pin set changes have not been saved.\n\n"
										  	"Press OK to load the selected set without saving.",
										 	 eLoadConfigPopUpTag);
					} else
					{
						LoadSelectedConfig();
					}
				}
				break;
			}
			case eDefPinsGridTag:
			{
				if (inAction < sizeof(sDefPinGridColorIdx))
				{
					uint8_t	status = sDefPinGridColorIdx[inAction]+1;
					if (status > 2)
					{
						status = 0;
					}
					mDefPinGridIsDirty = true;
					sDefPinGridColorIdx[inAction] = status;
					defPinsGrid.DrawCell(inAction);
				}
				
				break;
			}
			case eRemoveTagBtnTag:
			{
				if (inAction == XControl::eOff)
				{
					if (loadConfigPopUp.HasSelection())
					{
						alertDialog.DoMessage("Remove the selected pin set?\n\n"
										  		"Press OK to remove.",
										 	 	eRemoveTagBtnTag);
					} else
					{
						warningDialog.DoMessage("Nothing selected.");
					}
				}
				break;
			}
			case eSavePinSetBtnTag:
			{
				if (inAction == XControl::eOff)
				{
					SaveConfig();
				}
				break;
			}
			case eInfoPinsGridTag:
			{
				sInfoPinGridColorIdx++;
				if (sInfoPinGridColorIdx > 2)
				{
					sInfoPinGridColorIdx = 0;
				}
				infoPinsGrid.DrawCell(0);
				infoPinsLabel.ReplaceString(kCellInfoStr[sInfoPinGridColorIdx]);
				break;
			}
			case eClearTagBtnTag:
				if (inAction == XControl::eOff)
				{
					pinTesterGrid.Clear();
				}
				break;
			case eSwapPinsCheckboxTag:
				SwapPinsCheckboxChanged();
				break;
			case eSetTimeBtnTag:
			{
				if (inAction == XControl::eOff)
				{
					UnixTime::SetTime(dateValueField.Value());
					STM32UnixRTC::SyncRTCToTime();
				}
				break;
			}
			case eSaveConfigsBtnTag:
			{
				if (inAction == XControl::eOff)
				{
					SavePinConfigsToSD(true);
				}
				break;
			}
			case eMergeConfigsBtnTag:
			{
				if (inAction == XControl::eOff)
				{
					LoadPinConfigsFromSD(true, true);
				}
				break;
			}
			case eLoadConfigsBtnTag:
			{
				if (inAction == XControl::eOff)
				{
					LoadPinConfigsFromSD(false, true);
				}
				break;
			}
			case eAlertDialogTag+XDialogBox::eOKTagOffset:
				if (inAction == XControl::eOff)
				{
					switch(alertDialog.MessageTag())
					{
						case eMainMenuItem:
							ShowMainView();
							break;
						case eUtilitiesMenuItem:
							ShowUtilitiesView();
							break;
						case eLoadConfigPopUpTag:
							LoadSelectedConfig();
							break;
						case eRemoveTagBtnTag:
							RemoveSelectedConfig();
							break;
					}
				}
				break;
			case eAlertDialogTag+XDialogBox::eCancelTagOffset:
				if (inAction == XControl::eOff)
				{
					switch(alertDialog.MessageTag())
					{
						/*
						*	When cancelling, restore the previous selection.
						*/
						case eLoadConfigPopUpTag:
						//case eRemoveTagBtnTag:
						{
							char configName[64];
							GetCurrentPinConfigName(configName);
							if (mPinConfigs.GoToNamedLink(configName))
							{
								loadConfigPopUp.SetSelectedItemString(configName, true);
								pinSetNameTextField.SetUTF8Str(configName, true);
							}
						}
					}
				}
			case eTouchScreenCalibrationTag+XDialogBox::eOKTagOffset:
			{
				if (inAction == XControl::eOff)
				{
					SaveZCalibrationValues();
				}
				break;
			}
			case eAdjustZBtnTag:	// Show Z Calibration UI
				if (inAction == XControl::eOff && !touchScreenZCalibration.IsVisible())
				{
					touchScreenZCalibration.Start(&mTouchScreen, this);
				}
				break;
		}
	}
}

/******************************** ShowMainView ********************************/
void FFCTesterSTM32::ShowMainView(void)
{
	/*
	*	Because the views are the size of the screen, Hide isn't called because
	*	Show will redraw the entire screen.
	*/
	managePinsView.SetVisible(false);
	utilitiesView.SetVisible(false);
	pinTesterGrid.Clear(false);	// Reload, in case the config changed
	mainView.AttachView(&mainMenuBtn);
	mainView.Show();
}

/***************************** ShowManagePinsView *****************************/
void FFCTesterSTM32::ShowManagePinsView(void)
{
	/*
	*	Because the views are the size of the screen, Hide isn't called because
	*	Show will redraw the entire screen.
	*/
	mainView.SetVisible(false);
	utilitiesView.SetVisible(false);
	managePinsView.AttachView(&mainMenuBtn);
	managePinsView.Show();
}

/****************************** ShowUtilitiesView *****************************/
void FFCTesterSTM32::ShowUtilitiesView(void)
{
	/*
	*	Because the views are the size of the screen, Hide isn't called because
	*	Show will redraw the entire screen.
	*/
	mainView.SetVisible(false);
	managePinsView.SetVisible(false);
	utilitiesView.AttachView(&mainMenuBtn);
	dateValueField.SetValue(UnixTime::Time(), false);
	utilitiesView.Show();
}

/************************** PinsNotDirtyCanContinue ***************************/
bool FFCTesterSTM32::PinsNotDirtyCanContinue(
	uint16_t	inReference)
{
	bool	canContinue = managePinsView.IsVisible() == false ||
								mDefPinGridIsDirty == false;
	if (!canContinue)
	{
		alertDialog.DoMessage(
					"The current pin set changes have not been saved.\n\n"
					"Press OK to leave Manage Pins without saving.",
					 inReference);
	}
	return(canContinue);
}

/************************** GetCurrentPinConfigName ***************************/
void FFCTesterSTM32::GetCurrentPinConfigName(
	char*	outName)
{
	if (mPreferences.Read(Config::kCurrentPinConfigNameAddr, 64,
							(uint8_t*)outName) == 0)
	{
		outName[0] = 0;
	}
}

/************************** SetCurrentPinConfigName ***************************/
void FFCTesterSTM32::SetCurrentPinConfigName(
	const char*	inName)
{
	mPreferences.Write(Config::kCurrentPinConfigNameAddr,
				strlen(inName)+1, (const uint8_t*)inName);
}

/************************** SwapPinsCheckboxChanged ***************************/
void FFCTesterSTM32::SwapPinsCheckboxChanged(void)
{
	bool swapPinOrder = swapPinsCheckbox.GetState() == XControl::eOn;
	if (pinTesterGrid.InvertPinOrder(swapPinOrder, true))
	{
		SetPinOrderPref(swapPinOrder);
	}
}

/************************** GetPinOrderPref ***************************/
bool FFCTesterSTM32::GetPinOrderPref(void)
{
	Config::SUIPrefs	uiPrefs;
	bool		swapPinOrder = true;
	if (mPreferences.Read(Config::kUIPrefsAddr, sizeof(Config::SUIPrefs),
							(uint8_t*)&uiPrefs) == 0 &&
		uiPrefs.initialized == 1234)
	{
		swapPinOrder = uiPrefs.swapPinOrder;
	}
	return(swapPinOrder);
}

/************************** SetPinOrderPref ***************************/
void FFCTesterSTM32::SetPinOrderPref(
	bool	inSwapPinOrder)
{
	Config::SUIPrefs	uiPrefs;
	uiPrefs.initialized = 1234;
	uiPrefs.swapPinOrder = inSwapPinOrder;
	mPreferences.Write(Config::kUIPrefsAddr,
				 sizeof(Config::SUIPrefs), (const uint8_t*)&uiPrefs);
}

/***************************** LoadSelectedConfig *****************************/
void FFCTesterSTM32::LoadSelectedConfig(void)
{
	const char*	selItemStr = loadConfigPopUp.GetSelectedItemString();
	if (selItemStr)
	{
		pinSetNameTextField.SetUTF8Str(selItemStr, true);
		if (mPinConfigs.GoToNamedLink(selItemStr))
		{
			SetCurrentPinConfigName(selItemStr);
			mPinConfigs.GetCurrentConfig(sDefPinGridColorIdx);
			defPinsGrid.DrawCells();
			mDefPinGridIsDirty = false;
		}
	}
}

/**************************** RemoveSelectedConfig ****************************/
void FFCTesterSTM32::RemoveSelectedConfig(void)
{
	const char*	selItemStr = loadConfigPopUp.GetSelectedItemString();
	if (mPinConfigs.GoToNamedLink(selItemStr))
	{
		mPinConfigs.RemoveCurrent();
		if (mPinConfigs.GetCurrentIndex())
		{
			loadConfigPopUp.SetSelectedItemString(mPinConfigs.GetCurrentName(), true);
			LoadSelectedConfig();
		} else
		{
			pinSetNameTextField.SetUTF8Str("", true);
			loadConfigPopUp.ClearSelectedItemString(true);
			mDefPinGridIsDirty = false;
		}
	}
}

/********************************* SaveConfig *********************************/
void FFCTesterSTM32::SaveConfig(void)
{
	if (pinSetNameTextField.CharcodeCount())
	{
		char	saveName[XPCMenuItemProvider::eMaxStrLen];
		pinSetNameTextField.GetUTF8Str(saveName);
		if (mPinConfigs.GoToNamedLink(saveName))
		{
			mPinConfigs.SetCurrentConfig(sDefPinGridColorIdx);
		} else
		{
			mPinConfigs.AddConfig(saveName, sDefPinGridColorIdx);
			loadConfigPopUp.SetSelectedItemString(saveName, true);
			SetCurrentPinConfigName(saveName);
		}
		mDefPinGridIsDirty = false;
	} else
	{
		warningDialog.DoMessage("The name is empty.");
	}
}

/*************************** SaveZCalibrationValues ***************************/
void FFCTesterSTM32::SaveZCalibrationValues(void)
{
	int16_t	zCalibrationValues[9];
	if (touchScreenZCalibration.OKToSave(zCalibrationValues))
	{
		mTouchScreen.SetZCalibration(zCalibrationValues);
		mPreferences.Write(offsetof(Config::SGeneralPreferences, tsZCalibration),
				sizeof(zCalibrationValues), (const uint8_t*)zCalibrationValues);
	} else
	{
		mTouchScreen.ResumeZCalibration();
	}
}

/***************************** SaveTSettingsToSD *****************************/
void FFCTesterSTM32::SaveTSettingsToSD(
	bool	inMsgToScreen)
{
	if (digitalRead(Config::kSDDetectPin) == LOW)
	{
		TSettings	aaSettings;
		Config::STSettings	settings;
		settings.hourFormat = UnixTime::Format24Hour() ? 24:12,
		settings.currentView = eMainViewTag;
		mTouchScreen.GetMinMax(settings.tsMinMax);
		mTouchScreen.GetZCalibration(settings.tsZCalibration);
		
		SdFat sd;
		bool	success = sd.begin(Config::kSDSelectPin, SD_SCK_MHZ(4));
		if (success)
		{
			success = aaSettings.WriteFile(kTSettingsPath, settings);
		} else
		{
			sd.initErrorHalt();
		}
		if (inMsgToScreen)
		{
			warningDialog.DoMessage(success ? kSavedTSettingsStr : kSaveToSDFailedStr);
		} else
		{
			Serial.printf("%s\n", success ? kSavedTSettingsStr : kSaveToSDFailedStr);
		}
	} else
	{
		if (inMsgToScreen)
		{
			warningDialog.DoMessage(kNoSDCardFoundStr);
		} else
		{
			Serial.printf("%s\n", kNoSDCardFoundStr);
		}
	}
}

/**************************** LoadTSettingsFromSD *****************************/
void FFCTesterSTM32::LoadTSettingsFromSD(
	bool	inMsgToScreen)
{
	if (digitalRead(Config::kSDDetectPin) == LOW)
	{
		SdFat sd;
		TSettings	aaSettings;
		bool	success = sd.begin(Config::kSDSelectPin, SD_SCK_MHZ(4));
		if (success)
		{
			success = aaSettings.ReadFile(kTSettingsPath);
		} else
		{
			sd.initErrorHalt();
		}
		if (success)
		{
			const Config::STSettings&	settings = aaSettings.Settings();
			Config::SGeneralPreferences	prefs = {0};
			prefs.clockFormat = settings.hourFormat == 12;
			memcpy(prefs.tsMinMax, settings.tsMinMax, sizeof(prefs.tsMinMax));
			memcpy(prefs.tsZCalibration, settings.tsZCalibration, sizeof(prefs.tsZCalibration));
			mPreferences.Write(Config::kTPreferencesAddr,
				sizeof(Config::SGeneralPreferences), &prefs.unused[0]);
			/*
			*	Restart the board.
			*/
			HAL_NVIC_SystemReset();
		} else if (inMsgToScreen)
		{
			warningDialog.DoMessage(kLoadFromSDFailedStr);
		} else
		{
			Serial.printf("%s\n", kLoadFromSDFailedStr);
		}
	} else if (inMsgToScreen)
	{
		warningDialog.DoMessage(kNoSDCardFoundStr);
	} else
	{
		Serial.printf("%s\n", kNoSDCardFoundStr);
	}
}

/**************************** HandleConfigsLoaded *****************************/
/*
*	A new set of pin configurations has been loaded.
*/
void FFCTesterSTM32::HandleConfigsLoaded(void)
{
	char	configName[XPCMenuItemProvider::eMaxStrLen];
	GetCurrentPinConfigName(configName);
	if (mPinConfigs.GoToNamedLink(configName))
	{
		mPinConfigs.GetCurrentConfig(sDefPinGridColorIdx);
		loadConfigPopUp.SetSelectedItemString(configName, false);
		pinSetNameTextField.SetUTF8Str(configName, false);
	} else
	{
		if (mPinConfigs.GoToNthLink(0))
		{
			mPinConfigs.GetCurrentConfig(sDefPinGridColorIdx);
			loadConfigPopUp.SetSelectedItemString(mPinConfigs.GetCurrentName(), false);
			pinSetNameTextField.SetUTF8Str(mPinConfigs.GetCurrentName(), false);
			SetCurrentPinConfigName(mPinConfigs.GetCurrentName());
		}
	}
	mDefPinGridIsDirty = false;
}

/***************************** SavePinConfigsToSD *****************************/
void FFCTesterSTM32::SavePinConfigsToSD(
	bool	inMsgToScreen)
{
	const char*	resultStr = kNoSDCardFoundStr;
	if (digitalRead(Config::kSDDetectPin) == LOW)
	{
		SdFat sd;
		bool	success = sd.begin(Config::kSDSelectPin, SD_SCK_MHZ(4));
		if (success)
		{
			success = mPinConfigs.SaveToSD(kPinConfigsPath);
		} else
		{
			sd.initErrorHalt();
		}
		if (success)
		{
			resultStr = kPinConfigsSavedStr;
		} else
		{
			resultStr = kSaveToSDFailedStr;
		}
	}
	if (inMsgToScreen)
	{
		warningDialog.DoMessage(resultStr);
	} else
	{
		Serial.printf("%s\n", resultStr);
	}
}

/**************************** LoadPinConfigsFromSD ****************************/
void FFCTesterSTM32::LoadPinConfigsFromSD(
	bool	inMerge,
	bool	inMsgToScreen)
{
	const char*	resultStr = kNoSDCardFoundStr;
	if (digitalRead(Config::kSDDetectPin) == LOW)
	{
		SdFat sd;
		bool	success = sd.begin(Config::kSDSelectPin, SD_SCK_MHZ(4));
		if (success)
		{
			success = mPinConfigs.LoadFromSD(kPinConfigsPath, inMerge);
		} else
		{
			sd.initErrorHalt();
		}
		if (success)
		{
			resultStr = inMerge ? kPinConfigsMergedStr : kPinConfigsLoadedStr;
		} else
		{
			resultStr = kLoadFromSDFailedStr;
		}
		HandleConfigsLoaded();
	}
	if (inMsgToScreen)
	{
		warningDialog.DoMessage(resultStr);
	} else
	{
		Serial.printf("%s\n", resultStr);
	}
}

/************************** LoadGeneralPrefsFromFlash *************************/
void FFCTesterSTM32::LoadGeneralPrefsFromFlash(void)
{
	Config::SGeneralPreferences	prefs;
	bool	prefsRead = mPreferences.Read(Config::kTPreferencesAddr, sizeof(Config::SGeneralPreferences),
							&prefs.unused[0]) != 0;
	if (prefsRead)
	{
		Serial.printf("prefsRead\n");
		UnixTime::SetFormat24Hour(prefs.clockFormat == 0);

		/*
		*	Initialize the touchscreen alignment values if previously saved
		*/
		if (prefs.tsMinMax[0] != 0xFFFF)
		{
			mTouchScreen.SetMinMax(prefs.tsMinMax);
			Serial.printf("prefs.tsMinMax: %hd, %hd, %hd, %hd\n",
							prefs.tsMinMax[0], prefs.tsMinMax[1],
							prefs.tsMinMax[2], prefs.tsMinMax[3]);
		}
		/*
		*	Initialize the touchscreen Z calibration values if valid
		*/
		if ((uint16_t)prefs.tsZCalibration[0] != 0xFFFF)
		{
			mTouchScreen.SetZCalibration(prefs.tsZCalibration);
			Serial.printf("prefs.tsZCalibration:");
			for (uint32_t i = 0; i < 9; i++)
			{
				Serial.printf(" %hd,", prefs.tsZCalibration[i]);
			}
			Serial.printf("\n");
		}
	}
}

/******************************** CaretTimerISR *******************************/
/*
*	This gets called every 1/2 second.
*/
void FFCTesterSTM32::CaretTimerISR(void)
{
	sCaretNeedsUpdate = true;
}
	
