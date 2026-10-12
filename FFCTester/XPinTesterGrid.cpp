/*
*	XPinTesterGrid.cpp, Copyright Jonathan Mackey 2026
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

#include "XPinTesterGrid.h"
#include "XRootView.h"
#include "DisplayController.h"
#include "ValueFormatter.h"
#ifdef __MACH__
#include <iostream>
#endif

#ifndef DEBOUNCE_DELAY
#define DEBOUNCE_DELAY	20		// ms, for buttons
#endif

const uint16_t	XPinTesterGrid::kPinGridColors[] =
		{
			XFont::eGray,		// Pin can be ignored, except for bridge test.
			XFont::eGreen,		// Pin continuity has been verified, no bridges.
			0x7FDA,				// Pin verified, bridge expected by design.
			XFont::eRed,		// Pin continuity has not been verified.
			XFont::eYellow,		// Pin bridge detected.
			0x051F				// Expected bridge but is single (dark orange)
		};

/******************************* XPinTesterGrid *******************************/
XPinTesterGrid::XPinTesterGrid(
	int16_t			inX,
	int16_t			inY,
	uint16_t		inRows,
	uint16_t		inColumns,
	uint16_t		inTag,
	XView*			inNextView,
	XFont::Font*	inFont,
	uint16_t		inCellWidth,
	uint16_t		inCellHeight

)
	: XGrid(inX, inY, inRows, inColumns, inTag, inNextView, inFont,
	  inCellWidth, inCellHeight, kPinGridColors, mPinGridColorIdx,
	  nullptr, 0, 1), mPullState(99), mPinConfig(nullptr)
{
	memset(mPinGridColorIdx, eUnverified, sizeof(mPinGridColorIdx));
	mInvertPinOrder = true;
}

#ifndef __MACH__
/************************************ begin ***********************************/
void XPinTesterGrid::begin(
	const uint8_t*	inPortPinMap)
{
	mPortPinMap = inPortPinMap;
	memset(mPinGroupMask, 0, sizeof(mPinGroupMask));
	/*
	*	Generate the SPinGroups from the port pin map.
	*/
	for (uint32_t i = 0; i < eNumPins; i++)
	{
		uint8_t	pin = inPortPinMap[i];
		uint32_t	gpioIndex = pin >> 4;
		if (gpioIndex < eMaxPinGroups)
		{
			mPinGroupMask[gpioIndex] |= (_BV(pin & 0xF));
		}
	}
	
	/*
	*	After reset the IO port clocks (used for register read/write access)
	*	are disabled and the application software has to enable each port's
	*	clock before using them for IO.
	*/
	for (uint32_t gpioIndex = 0; gpioIndex < eMaxPinGroups; gpioIndex++)
	{
		if (mPinGroupMask[gpioIndex] != 0)
		{
			/*
			*	set_GPIO_Port_Clock is declared in PortNames.h.
			*	set_GPIO_Port_Clock contains a switch that executes the
			*	corresponding port's __HAL_RCC_GPIOx_CLK_ENABLE() macro.
			*/
			set_GPIO_Port_Clock(gpioIndex);
		}
	}
	
	SetPinsPullState(GPIO_PULLUP);
}

/****************************** SetPinsPullState ******************************/
void XPinTesterGrid::SetPinsPullState(
	uint32_t	inPullState)	// GPIO_NOPULL, GPIO_PULLUP or GPIO_PULLDOWN
{
	if (mPullState != inPullState &&
		inPullState <= GPIO_PULLDOWN)
	{
		GPIO_InitTypeDef gpioInit = {0, GPIO_MODE_INPUT, inPullState, 0, 0};
		mPullState = inPullState;
		for (uint32_t i = 0; i < eMaxPinGroups; i++)
		{
			if (mPinGroupMask[i] != 0)
			{
				gpioInit.Pin = mPinGroupMask[i];
				HAL_GPIO_Init(GPIOPort[i], &gpioInit);
			}
		}
	}
}

/*********************************** Update ***********************************/
void XPinTesterGrid::Update(void)
{
	/*
	*	All pins being tested are set to GPIO_PULLUP, so unless the pin is
	*	physically set to GND/LOW by the user, all pins should read HIGH.
	*
	*	Initially all grid cells are set to the unverified color red, or the
	*	unused color gray.
	*
	*	When the user grounds a pin, in some cases this will cause more than one
	*	pin to read as LOW.  This can happen by design when there are
	*	multiple GND & VCC pins, or when groups of pins are used to configure
	*	the PCB.  The other case may show that there is an unintended short
	*	between adjacent pins.  When this is detected the pin's cell color is
	*	set to the warning color yellow.  For all other cases, the pin(s) cell
	*	color is set to the verified color green.
	*/
	uint32_t	pinsState;
	bool		pinsChanged = false;
	for (uint32_t gpioIndex = 0; gpioIndex < eMaxPinGroups; gpioIndex++)
	{
		if (mPinGroupMask[gpioIndex] != 0)
		{
			pinsState = (~GPIOPort[gpioIndex]->IDR) & mPinGroupMask[gpioIndex];
		} else
		{
			pinsState = 0;
		}
		if (mPinsState[gpioIndex] != pinsState)
		{
			mPinsState[gpioIndex] = pinsState;
			pinsChanged = pinsState != 0;
		}
	}
	/*
	*	To deal with bounce, where the probe arcs causing a misread, the
	*	readings need to be the same for DEBOUNCE_DELAY period of time.
	*
	*	If the pins changed since the last reading THEN
	*	start the debounce delay period timer.
	*/
	if (pinsChanged)
	{
		mPinsStateDebouncePeriod.Set(DEBOUNCE_DELAY);
		mPinsStateDebouncePeriod.Start();
	/*
	*	Else if the debounce period timer is at or passed the delay period THEN
	*	there has been no change in the pins states during the debounce period.
	*/
	} else if (mPinsStateDebouncePeriod.Passed())
	{
		/*
		*	Cancel the timer so that Passed won't return true till the next
		*	pinsChanged is detected.
		*/
		mPinsStateDebouncePeriod.Set(0);

		/*
		*	mPinsState contains the current pin states of each gpio port
		*	used. If any pins changed, then align the pins currently high
		*	with the pins in the mCellToColor array (aka mPinGridColorIdx
		*	array), setting the color to either eVerified, eVerifiedBridge
		*	or eBridgeWarning.  eBridgeWarning is set when an adjacent pin
		*	is also high and the config isn't expecting a bridge for this
		*	pin. eVerifiedBridge is set when there is a bridge and the
		*	bridge is expected.
		*/
		uint32_t	pinRun = 0;
		for (uint32_t i = 0; i < eNumPins; i++)
		{
			uint8_t	pin = mPortPinMap[mInvertPinOrder ? (eNumPins-1) - i : i];
			uint32_t	gpioIndex = pin >> 4;
			if (gpioIndex < eMaxPinGroups)
			{
				pinsState = mPinsState[gpioIndex];
				/*
				*	If this pin is high...
				*/
				if (pinsState & (_BV(pin & 0xF)))
				{
				
					/*
					*	If this pin is bridged with an adjacent pin THEN
					*	set its status to eBridgeWarning or eVerifiedBridge
					*	eVerifiedBridge means a bridge is expected for this pin.
					*/
					if (pinRun)
					{
						SetPinStatus(i, mPinConfig[i] == eVerifiedBridge ?
											eVerifiedBridge : eBridgeWarning);
					/*
					*	Else if this pin hasn't been verified (and it's used) THEN
					*	set its status to eVerified.
					*/
					} else if (mPinGridColorIdx[i] == eUnverified)
					{
						SetPinStatus(i, mPinConfig[i] == eVerifiedBridge ?
											eBridgeExpectedWarning : eVerified);
					}
					pinRun++;
				} else if (pinRun)
				{
					/*
					*	If more than one pin is in the brige run THEN 
					*	set the first pin in the run as being bridged along
					*	with the rest of the pins in the run that have already
					*	been set above. 
					*/
					if (pinRun > 1)
					{
						uint32_t	pinIdx = i - pinRun;
						SetPinStatus(pinIdx, mPinConfig[pinIdx] == eVerifiedBridge ?
											eVerifiedBridge : eBridgeWarning);
					}
					pinRun = 0;
				}
			}
		}
		
		/*
		*	If a pin run occured that includes the last pin on the connector THEN
		*	set the first pin in the run as being bridged along with the rest of
		*	the pins in the run.
		*/
		if (pinRun > 1)
		{
			uint32_t	pinIdx = 40 - pinRun;
			SetPinStatus(pinIdx, mPinConfig[pinIdx] == eVerifiedBridge ?
								eVerifiedBridge : eBridgeWarning);
		}
	}
}

/******************************** SetPinStatus ********************************/
void XPinTesterGrid::SetPinStatus(
	uint16_t	inCellIndex,
	EPinStatus	inStatus,
	bool		inUpdate)
{
	/*
	*	If the pin status changed THEN
	*	update the status.
	*/
	if (mPinGridColorIdx[inCellIndex] != (uint8_t)inStatus)
	{
		mPinGridColorIdx[inCellIndex] = (uint8_t)inStatus;
		if (inUpdate)
		{
			DrawCell(inCellIndex);
		}
	}
}

#else
/*********************************** Update ***********************************/
void XPinTesterGrid::Update(void)
{
}

#endif

/******************************** LoadPinConfig *******************************/
void XPinTesterGrid::LoadPinConfig(
	const uint8_t*	inPinConfig)
{
	mPinConfig = inPinConfig;
	Clear(false);
}

/************************************ Clear ***********************************/
void XPinTesterGrid::Clear(
	bool	inUpdate)
{
	if (mPinConfig)
	{
		for (uint32_t i = 0; i < eNumPins; i++)
		{
			mPinGridColorIdx[i] = mPinConfig[i] ? eUnverified : eUnused;
		}
	} else
	{
		memset(mPinGridColorIdx, eUnverified, sizeof(mPinGridColorIdx));
	}
	if (inUpdate)
	{
		DrawCells();
	}
}

/******************************* InvertPinOrder *******************************/
bool XPinTesterGrid::InvertPinOrder(
	bool	inPin40isPin1,
	bool	inUpdate)
{
	bool	pinOrderChanged = mInvertPinOrder != inPin40isPin1;
	if (pinOrderChanged)
	{
		mInvertPinOrder = inPin40isPin1;
		/*
		*	Reverse/swap the status of the pins currently displayed.
		*/
		{
			
			uint32_t ii = (eNumPins-1);
			for (uint32_t i = 0; i < (eNumPins/2); i++, ii--)
			{
				uint8_t	val = mPinGridColorIdx[i];
				mPinGridColorIdx[i] = mPinGridColorIdx[ii];
				mPinGridColorIdx[ii] = val;
			}
		}
		if (inUpdate)
		{
			DrawCells();
		}
	}
	return(pinOrderChanged);
}

