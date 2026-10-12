/*
*	AAXViews.h, Copyright Jonathan Mackey 2025
*	UI and constant strings.
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
#ifndef AAXViews_h
#define AAXViews_h

#include "ST77XXToXPT2046Alignment.h"
#include "XPT2046ZCalibration.h"
#include "ValueFormatter.h"
#include "XAlert.h"
#include "XAnimatedFontIcon.h"
#include "XCheckboxButton.h"
#include "XColoredView.h"
#include "XDateValueField.h"
#include "XDialogBox.h"
#include "XMenuButton.h"
#include "XMenuItem.h"
#include "XNumberValueField.h"
#include "XPopUpButton.h"
#include "XPushButton.h"
#include "XRadioButton.h"
#include "XRootView.h"
#include "XStepper.h"
#include "XKeyboard.h"
#include "XTextField.h"
#include "XPCMenuItemProvider.h"
#include "XSwitch.h"
#include "XGrid.h"
#include "XPinTesterGrid.h"


static const char kVerticalEllipsisStr[] = ".";

static const char kSaveSettingsStr[] = "Settings to/from SD:";
static const char kSaveToSDFailedStr[] = "Save to SD failed.";
static const char kLoadFromSDFailedStr[] = "Load from SD failed.";
static const char kNoSDCardFoundStr[] = "No SD card found.";
static const char kSavedTSettingsStr[] = "Saved to file TSettings.txt";
static const char kLoadTSettingsStr[] =	"Press OK to load file\n"
											"TSettings.txt.  The board\n"
											"will restart if successful.";
static const char kPinConfigsSavedStr[] = "PinConfigs.csv saved to SD.";
static const char kPinConfigsMergedStr[] = "PinConfigs.csv merged from SD.";
static const char kPinConfigsLoadedStr[] = "PinConfigs.csv loaded from SD.";
static const char kUnableToReadPrefsStr[] = "Unable to read preferences";

// Touch screen alignment instructions.
static const char	kAlignmentInstMsg[] =
									"Precisely touch the tiny white dot\n"
									"within each circle. A magenta dot\n"
									"will reflect the alignment accuracy.\n"
									"The red circle at the top of the display\n"
									"will change to green when enough points\n"
									"have been entered to save. When satisfied\n"
									"with the alignment, press Enter to save.\n"
									"Press Cancel to quit alignment at any time.";

// Touch screen Z calibration instructions.
static const char	kTopCalibInstMsg[] =
									"Precisely touch the tiny white dot within\n"
									"each circle while trying to apply the\n"
									"same pressure for each touch. You need to\n"
									"loop around the grid 5 times in order to\n"
									"get a valid set of samples.";
									
static const char	kBottomCalibInstMsg[] =
									"Once a valid set of samples is aquired the\n"
									"red circle will change to green and the\n"
									"Save button will be enabled.  At this point\n"
									"you either press Cancel or Save.";

enum EViewTag
{
	eMainViewTag = 300,
	eInfoDateValueFieldTag,
	eConfigLabelTag,
	eConfigNameLabelTag,
	eClearTagBtnTag,
	ePinTesterGridTag,
	eSwapPinsCheckboxTag,

	eManagePinsViewTag = 200,
	eManagePinsLabelTag,
	eLoadConfigPopUpTag,
	eRemoveTagBtnTag,
	eDefPinsGridTag,
	eLoadConfigMenuTag,
	ePinSetLabelTag,
	ePinSetNameTextFieldTag,
	eSavePinSetBtnTag,
	eInfoPinsGridTag,
	eInfoPinsLabelTag,

	eUtilitiesViewTag = 400,
	eUtilitiesLabelTag,
	eSetTimeLabelTag,
	eDateValueFieldTag,
	eDateValueStepperTag,
	eSetTimeBtnTag,
	eAdjustZBtnTag,
	eAdjustZLabelTag,
	eSaveConfigsBtnTag,
	eSaveConfigsLabelTag,
	eMergeConfigsBtnTag,
	eLoadConfigsBtnTag,
	eLoadConfigsLabelTag,
	
	eMainMenuBtnTag = 900,
	eMainMenuTag,

	eAlertDialogTag = 600,
	eWarningDialogTag = 700,
	eKeyboardTag = 800,
	eTouchScreenAlignmentTag = 1200,

	eTouchScreenCalibrationTag = 1300,
	eCalibrationDoneBtnTag,
	eCancelCalibrationBtnTag,

	eAboutBoxTag = 1000,
	eSoftwareNameLabelTag,
	eVersionLabelTag,
	eCopyrightLabelTag
};

static const uint16_t	kTGap = 10;
static const uint16_t	kSpaceBetween = 10;
static const uint16_t	kLabelYAdj = 4;
static const uint16_t	kRowHeight = 36;
static const uint16_t	kSRowHeight = 29;	// Text only row height
static const uint16_t	kDialogBGColor = 0xF77D;

/*
*	The touch screen alignment view is displayed when the Enter button is
*	pressed.  This button is only attached/needed when alignment of the touch
*	screen is performed.  The result of the alignment is written to the EEPROM.
*/
ST77XXToXPT2046Alignment touchScreenAlignment(eTouchScreenAlignmentTag,
				&UI20ptFont, nullptr,
				kAlignmentInstMsg);

XPT2046ZCalibration	touchScreenZCalibration(eTouchScreenCalibrationTag,
				"Save", "Cancel",
				&UI20ptFont, &touchScreenAlignment,
				kTopCalibInstMsg, kBottomCalibInstMsg);			

XAlert	alertDialog(eAlertDialogTag, &touchScreenZCalibration,
				"OK", "Cancel", nullptr,
				&UI20ptFont,
				nullptr, kDialogBGColor, kDialogBGColor);

XAlert	warningDialog(eWarningDialogTag, &alertDialog,
				"OK", nullptr, nullptr,
				&UI20ptFont,
				nullptr, kDialogBGColor, kDialogBGColor);

XKeyboard		keyboard(eKeyboardTag, &warningDialog,
				&UI20ptFont, &UI13ptFont, XFont::eWhite, kDialogBGColor);
				
// About box
XLabel		softwareNameLabel(0, 0, 280, 26,
				eSoftwareNameLabelTag, nullptr, "FFC Tester",
				&UI20ptFont, nullptr,
				XFont::eBlack, kDialogBGColor, XFont::eAlignCenter);
XLabel		versionLabel(0, kRowHeight, 280, 26,
				eVersionLabelTag, &softwareNameLabel, "STM32 version 1.0",
				&UI20ptFont, nullptr,
				XFont::eBlack, kDialogBGColor, XFont::eAlignCenter);
XLabel		copyrightLabel(0, kRowHeight*2, 280, 26,
				eCopyrightLabelTag, &versionLabel, "Copyright Jonathan Mackey 2026",
				&UI20ptFont, nullptr,
				XFont::eBlack, kDialogBGColor, XFont::eAlignCenter);
XDialogBox	aboutBox(&copyrightLabel,
				eAboutBoxTag, &keyboard,
				"Close", nullptr, nullptr,
				&UI20ptFont,
				nullptr, kDialogBGColor);
				
//	Main menu
enum EMenuItems
{
	eMainMenuItem = 1,
	eManagePinsMenuItem,
	eUtilitiesMenuItem,
	eAboutMenuItem
};

XMenuItem	aboutMenuItem(eAboutMenuItem, "About");
XMenuItem	utilitiesMenuItem(eUtilitiesMenuItem, "Utilities", &aboutMenuItem);
XMenuItem	managePinsMenuItem(eManagePinsMenuItem, "Manage Pins", &utilitiesMenuItem);
XMenuItem	mainMenuItem(eMainMenuItem, "Main", &managePinsMenuItem);
XMenu		mainMenu(eMainMenuTag, &UI20ptFont, &mainMenuItem);
XMenuButton mainMenuBtn(480-32, 2, 27, 0,
				eMainMenuBtnTag, &mainMenu, nullptr,
				kVerticalEllipsisStr, &UI20ptFont);

#ifdef __MACH__
namespace Config
{
	const pin_t		kSDDetectPin		= 0;
	const pin_t		kSDSelectPin		= 1;
}
#endif

// Select pin configuration menu
static uint8_t	sPCMenuItemCache[XItemStrCacheSize(5, XPCMenuItemProvider::eMaxStrLen)];

XPCMenuItemProvider	configItemProvider(sPCMenuItemCache, 5);

XMenuItem	configMenuItems[5];
XMenu		loadConfigMenu(eLoadConfigMenuTag, &UI20ptFont, configMenuItems, 5,
							0, &configItemProvider);




// Utilities view
XLabel			utilitiesLabel(18, kTGap+kLabelYAdj, 110, 26,
				eUtilitiesLabelTag, nullptr, "Utilities",
				&UI20ptFont, nullptr,
				XFont::eWhite, XFont::eBlack, XFont::eAlignLeft);
				
XLabel			setTimeLabel(46, kTGap+kLabelYAdj+(kRowHeight*1), 79, 26,
				eSetTimeLabelTag, &utilitiesLabel, "Set Time:",
				&UI20ptFont, nullptr,
				XFont::eWhite, XFont::eBlack, XFont::eAlignRight);
XDateValueField	dateValueField(0, 0, 0,	// << x & y gets set by the stepper.
				eDateValueFieldTag, &setTimeLabel,
				&UI20ptFont);
XStepper		dateValueStepper(345, kTGap+(kRowHeight*1), 0, 0,
				eDateValueStepperTag, &dateValueField);
XPushButton		setTimeBtn(374, kTGap+(kRowHeight*1), 90, 0,
				eSetTimeBtnTag, &dateValueStepper, "Set",
				&UI20ptFont);
				
XPushButton		adjustZBtn(374, kTGap+(kRowHeight*2), 90, 0,
				eAdjustZBtnTag, &setTimeBtn, "Adjust",
				&UI20ptFont);
XLabel			adjustZLabel(151, kTGap+kLabelYAdj+(kRowHeight*2), 214, 26,
				eAdjustZLabelTag, &adjustZBtn, "Adjust pen force settings:",
				&UI20ptFont, nullptr,
				XFont::eWhite, XFont::eBlack, XFont::eAlignRight);
XPushButton		saveConfigsBtn(374, kTGap+(kRowHeight*4), 90, 0,
				eSaveConfigsBtnTag, &adjustZLabel, "Save",
				&UI20ptFont);
XLabel			saveConfigsLabel(96, kTGap+kLabelYAdj+(kRowHeight*4), 269, 26,
				eSaveConfigsLabelTag, &saveConfigsBtn, "Save configs to \"PinConfigs.csv\"",
				&UI20ptFont, nullptr,
				XFont::eWhite, XFont::eBlack, XFont::eAlignRight);
XPushButton		mergeConfigsBtn(274, kTGap+(kRowHeight*5), 90, 0,
				eMergeConfigsBtnTag, &saveConfigsLabel, "Merge",
				&UI20ptFont);
XPushButton		loadConfigsBtn(374, kTGap+(kRowHeight*5), 90, 0,
				eLoadConfigsBtnTag, &mergeConfigsBtn, "Load",
				&UI20ptFont);
XLabel			loadConfigsLabel(36, kTGap+kLabelYAdj+(kRowHeight*5), 229, 26,
				eLoadConfigsLabelTag, &loadConfigsBtn, "Load from \"PinConfigs.csv\"",
				&UI20ptFont, nullptr,
				XFont::eWhite, XFont::eBlack, XFont::eAlignRight);
				
XColoredView	utilitiesView(0, 0, 480, 320,
				eUtilitiesViewTag, &aboutBox, &loadConfigsLabel,
				nullptr, false);
				
static uint16_t sChangeTagToBuf[5];
static uint16_t sAddTagBuf[5];
// Manage Pins view
XLabel			managePinsLabel(18, kTGap+kLabelYAdj, 110, 26,
				eManagePinsLabelTag, nullptr, "Manage Pins",
				&UI20ptFont, nullptr,
				XFont::eWhite, XFont::eBlack, XFont::eAlignLeft);

static char sLoadConfigName[XPCMenuItemProvider::eMaxStrLen];
XPopUpButton	loadConfigPopUp(15, kTGap+(kRowHeight*1), 340, 0,
				eLoadConfigPopUpTag, &loadConfigMenu, &managePinsLabel,
				&UI20ptFont, XPopUpButton::eLargePopUpSize,
				XFont::eWhite, kDialogBGColor, sLoadConfigName);
XPushButton		removeTagBtn(374, kTGap+(kRowHeight*1), 90, 0,
				eRemoveTagBtnTag, &loadConfigPopUp, "Remove",
				&UI20ptFont);

static const uint16_t	kDefPinGridColors[] =
		{
			0xE6FB,	//	(0)	unused
			0xEBA6,	//	(1)	used
			0xFD60	//	(2)	used, bridge expected
		};
static uint8_t	sDefPinGridColorIdx[40] = {0};

XGrid			defPinsGrid(17, kTGap+(kRowHeight*2), 4, 10,
				eDefPinsGridTag, &removeTagBtn, &UI13ptFont,
				28, 28,	// cell w, h
				kDefPinGridColors, sDefPinGridColorIdx, nullptr, 0 , 1);

XLabel			pinSetLabel(15, kTGap+kLabelYAdj+(kRowHeight*6), 62, 26,
				ePinSetLabelTag, &defPinsGrid, "Pin Set:",
				&UI20ptFont, nullptr,
				XFont::eWhite, XFont::eBlack, XFont::eAlignLeft);
static uint16_t sPinSetNameBuf[64];
XTextField		pinSetNameTextField(82, kTGap+(kRowHeight*6), 275,
				ePinSetNameTextFieldTag, &pinSetLabel,
				&UI20ptFont, XFont::eWhite, XFont::eBlack,
				sPinSetNameBuf, 63, XKeyboard::e8BitOnly, true, true);
XPushButton		savePinSetBtn(374, kTGap+(kRowHeight*6), 90, 0,
				eSavePinSetBtnTag, &pinSetNameTextField, "Save",
				&UI20ptFont);

static uint8_t	sInfoPinGridColorIdx = {0};
static const char*	kCellInfoStr[] =
						{"= Unused", "= Used", "= Used + Bridged"};
XGrid			infoPinsGrid(17, kTGap+(kRowHeight*7), 1, 1,
				eInfoPinsGridTag, &savePinSetBtn, &UI13ptFont,
				20, 20,	// cell w, h
				kDefPinGridColors, &sInfoPinGridColorIdx);
XLabel			infoPinsLabel(52, kTGap+kLabelYAdj+(kRowHeight*7), 145, 26,
				eInfoPinsLabelTag, &infoPinsGrid, kCellInfoStr[0],
				&UI20ptFont, nullptr,
				XFont::eWhite, XFont::eBlack, XFont::eAlignLeft);
				
XColoredView	managePinsView(0, 0, 480, 320,
				eManagePinsViewTag, &utilitiesView, &infoPinsLabel,
				nullptr, false);
// Main view
XDateValueField	infoDateValueField(136,3,0,
				eInfoDateValueFieldTag, &mainMenuBtn,
				&UI20ptFont);
				
XPushButton		clearTagBtn(374, kTGap+(kRowHeight*1), 90, 0,
				eClearTagBtnTag, &infoDateValueField, "Clear",
				&UI20ptFont);
XPinTesterGrid	pinTesterGrid(17, kTGap+(kRowHeight*1), 4, 10,
				ePinTesterGridTag, &clearTagBtn, &UI13ptFont,
				28, 28);	// cell w, h
XCheckboxButton	swapPinsCheckbox(15, kTGap+(kRowHeight*5), 260, 26,
				eSwapPinsCheckboxTag, &pinTesterGrid, "Mirror pins (1 is 40, 40 is 1)",
				&UI20ptFont);

XLabel			configLabel(15, kTGap+kLabelYAdj+(kRowHeight*6), 61, 26,
				eConfigLabelTag, &swapPinsCheckbox, "Pin set:",
				&UI20ptFont, nullptr,
				XFont::eWhite, XFont::eBlack, XFont::eAlignRight);
XLabel			configNameLabel(81, kTGap+kLabelYAdj+(kRowHeight*6), 300, 26,
				eConfigNameLabelTag, &configLabel, sLoadConfigName,
				&UI20ptFont, nullptr,
				XFont::eWhite, XFont::eBlack, XFont::eAlignLeft);

				

XColoredView	mainView(0, 0, 480, 320,
				eMainViewTag, &managePinsView, &configNameLabel,
				nullptr, false);

XRootView	rootView(&mainView);
#endif // AAXViews_h
