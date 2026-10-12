/*
*	Config.h, Copyright Jonathan Mackey 2026
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
#ifndef Config_h
#define Config_h

#include <inttypes.h>
#include "PlatformDefs.h"

// This is coded for a STM32F429Vx

#define BAUD_RATE		19200
#define DEBOUNCE_DELAY	20		// ms, for buttons

#define DISPLAY_CONTROLLER	TFT_ILI9488
namespace Config
{
	const pin_t		kPA0Pin				= PA0;	// Unused
	const pin_t		kPA1Pin				= PA1;	// Unused
	const pin_t		kTx2Pin				= PA2;
	const pin_t		kRx2Pin				= PA3;
	const pin_t		kPA4LEDPin			= PA4;
	const pin_t		kPA5LEDPin			= PA5;
	const pin_t		kPA6LEDPin			= PA6;
	const pin_t		kPA7LEDPin			= PA7;
	const pin_t		kPA8LEDPin			= PA8;
	const pin_t		kPA9LEDPin			= PA9;
	const pin_t		kPA10LEDPin			= PA10;
	const pin_t		kUSB_DNPin			= PA11;
	const pin_t		kUSB_DPPin			= PA12;
	const pin_t		kSWDIOPin			= PA13;
	const pin_t		kSWCLKPin			= PA14;
	const pin_t		kSDDetectPin		= PA15;	// Ext Int 15

	const pin_t		kFFC_P37Pin			= PB0;
	const pin_t		kFFC_P36Pin			= PB1;
	const pin_t		kBoot1Pin			= PB2;	// Externally pulled low
	const pin_t		kSCK1Pin			= PB3;
	const pin_t		kMISO1Pin			= PB4;
	const pin_t		kMOSI1Pin			= PB5;
	const pin_t		kSCL1Pin			= PB6;
	const pin_t		kDispCSPin			= PB7;
	const pin_t		kDispResetPin		= PB8;
	const pin_t		kSDA1Pin			= PB9;
	const pin_t		kFFC_P26Pin			= PB10;
	const pin_t		kFFC_P25Pin			= PB11;
	const pin_t		kFFC_P24Pin			= PB12;
	const pin_t		kFFC_P23Pin			= PB13;
	const pin_t		kFFC_P22Pin			= PB14;
	const pin_t		kFFC_P21Pin			= PB15;

	const pin_t		kClearBtnPin		= PC0;	// Ext Int 0
	const pin_t		kPC1Pin				= PC1;	// Unused
	const pin_t		kPC2Pin				= PC2;	// Unused
	const pin_t		kFFC_P40Pin			= PC3;
	const pin_t		kFFC_P39Pin			= PC4;
	const pin_t		kFFC_P38Pin			= PC5;
	const pin_t		kFFC_P12Pin			= PC6;
	const pin_t		kFFC_P11Pin			= PC7;
	const pin_t		kFFC_P10Pin			= PC8;
	const pin_t		kFFC_P09Pin			= PC9;
	const pin_t		kSDSelectPin		= PC10;
	const pin_t		kBacklightPin		= PC11;
	const pin_t		kDispDCPin			= PC12;
	const pin_t		kPC13Pin			= PC13;	// Unused
	const pin_t		kOsc32InPin			= PC14;	// RTC OSC
	const pin_t		kOsc32OutPin		= PC15;	// RTC OSC

	const pin_t		kFFC_P01Pin			= PD0;
	const pin_t		kFFC_P02Pin			= PD1;
	const pin_t		kFFC_P03Pin			= PD2;
	const pin_t		kFFC_P04Pin			= PD3;
	const pin_t		kFFC_P05Pin			= PD4;
	const pin_t		kFFC_P06Pin			= PD5;
	const pin_t		kFFC_P07Pin			= PD6;
	const pin_t		kFFC_P08Pin			= PD7;
	const pin_t		kFFC_P20Pin			= PD8;
	const pin_t		kFFC_P19Pin			= PD9;
	const pin_t		kFFC_P18Pin			= PD10;
	const pin_t		kFFC_P17Pin			= PD11;
	const pin_t		kFFC_P16Pin			= PD12;
	const pin_t		kFFC_P15Pin			= PD13;
	const pin_t		kFFC_P14Pin			= PD14;
	const pin_t		kFFC_P13Pin			= PD15;

	const pin_t		kTouchCSPin			= PE0;
	const pin_t		kTouchIRQPin		= PE1;	// Ext Int 1
	const pin_t		kEnterBtnPin		= PE2;	// Ext Int 2
	const pin_t		kCancelBtnPin		= PE3;	// Ext Int 3
	const pin_t		kPE4Pin				= PE4;	// Unused
	const pin_t		kPE5Pin				= PE5;	// Unused
	const pin_t		kPE6Pin				= PE6;	// Unused
	const pin_t		kFFC_P35Pin			= PE7;
	const pin_t		kFFC_P34Pin			= PE8;
	const pin_t		kFFC_P33Pin			= PE9;
	const pin_t		kFFC_P32Pin			= PE10;
	const pin_t		kFFC_P31Pin			= PE11;
	const pin_t		kFFC_P30Pin			= PE12;
	const pin_t		kFFC_P29Pin			= PE13;
	const pin_t		kFFC_P28Pin			= PE14;
	const pin_t		kFFC_P27Pin			= PE15;

	const pin_t		kOscInPin			= PH0;
	const pin_t		kOscOutPin			= PH1;

	const uint8_t	kDisplayRotation	= 3;	// 0, 1, 2, 3 = 0, 90, 180, 270

	const uint32_t	kClearBtnMask		= _BV(0); //digitalPinToBitMask(PC0);
	const uint32_t	kEnterBtnMask 		= _BV(2); //digitalPinToBitMask(PE2);
	const uint32_t	kCancelBtnMask		= _BV(3); //digitalPinToBitMask(PE3);
//	const uint32_t	kSDDetectMask 		= _BV(15); //digitalPinToBitMask(PA15);
//	const uint32_t	kTouchIRQMask 		= _BV(1); //digitalPinToBitMask(PE1);

	const uint32_t	kPINBtnMask = (kEnterBtnMask | kCancelBtnMask);	// PEx

	const uint8_t	kTextInset			= 3; // Makes room for drawing the selection frame
	const uint8_t	kTextVOffset		= 6; // Makes room for drawing the selection frame
	// To make room for the selection frame the actual font height in the font
	// file is reduced.  The actual height is kFontHeight.
	const uint8_t	kFontHeight			= 43;

#if 1
	// Touchscreen min/max for 4" ILI9488 display
	const uint16_t	kDisplayWidth		= 320;
	const uint16_t	kDisplayHeight		= 480;
	const bool		kInvertTouchX		= false;
	const bool		kInvertTouchY		= false;
#else
	// Touchscreen min/max for 3.5" ILI9488 display
	const uint16_t	kDisplayWidth		= 320;
	const uint16_t	kDisplayHeight		= 480;
	const bool		kInvertTouchX		= true;
	const bool		kInvertTouchY		= true;
#endif

	/*
	*	The kPortPinMap below is an encapsulation of all of the kFFC_PxxPins
	*	listed above. The kPortPinMap maps the FFC pin with the corresponding
	*	mcu pin.  Each entry contains the gpio and pin indexes.
	*
	*	The Px_x enumerations are defined in PinNames.h
	*	The high nibble for each is the gpio index, and the low nibble is the
	*	pin index, 0 to 15.
	*
	*	In GPIOPort array is declared in PortNames.h.  GPIOPort is an array of
	*	GPIO_TypeDef pointers corresponding 1:1 with the gpio index in the high
	*	nibble (after shifting.)
	*/
	const uint8_t	kPortPinMap[] = 
	{
		PD_0,  PD_1,  PD_2,  PD_3,  PD_4,  PD_5,  PD_6,  PD_7,
		PC_9,  PC_8,  PC_7,  PC_6,  PD_15, PD_14, PD_13, PD_12,
		PD_11, PD_10, PD_9,  PD_8,  PB_15, PB_14, PB_13, PB_12,
		PB_11, PB_10, PE_15, PE_14, PE_13, PE_12, PE_11, PE_10,
		PE_9,  PE_8,  PE_7,  PB_1,  PB_0,  PC_5,  PC_4,  PC_3
	};
	
	const uint8_t kAT24CDeviceAddr = 0x50;	// Serial EEPROM I2C address
	const uint8_t kAT24CDeviceCapacity = 8;	// Value at end of AT24Cxxx xxx/8

	/*
	*	The preferences occupy the first N bytes of the AT24C64 EEPROM
	*	The AT24C64 EEPROM has 8KB of storage.
	*/
	const uint16_t	kTPreferencesAddr		= 0;		// EEPROM Page 0
	// struct STPreferences is defined in KRSettings.h

	struct SGeneralPreferences
	{
		uint8_t		unused[2];
		uint8_t		clockFormat;	// The time format (0=24, 1=12)
		uint8_t		unused2;
		uint16_t	tsMinMax[4];	// Touch screen alignment values
		int16_t		tsZCalibration[9];
	}; // __attribute__ ((packed)); (was getting compiler warnings)
	
	const uint16_t	kCurrentPinConfigNameAddr		= 1*32;		// EEPROM Page 1 & 2, 64 bytes
	const uint16_t	kUIPrefsAddr		= 3*32;		// EEPROM Page 3, 32 bytes
	struct SUIPrefs
	{
		uint16_t	initialized;		// Set to 1234 when initialized
		bool		swapPinOrder;		// Invert pin order
	};
	
	// From 128 till the AT24C64 end is reserved for pin configurations.
	const uint16_t	kPinConfigsAddr		= 4*32;			// EEPROM Page 4
	const uint32_t	kPinConfigsSize 	= 0x2000-kPinConfigsAddr;
}

#endif // Config_h

