/*
*	XPinTesterGrid.h, Copyright Jonathan Mackey 2026
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
#ifndef XPinTesterGrid_h
#define XPinTesterGrid_h

#include "XGrid.h"
#include "MSPeriod.h"

struct SPinGroup
{
	uint16_t		mask;
	uint8_t			pin[16];
};

class XPinTesterGrid : public XGrid
{
public:
							XPinTesterGrid(
								int16_t					inX,
								int16_t					inY,
								uint16_t				inRows,
								uint16_t				inColumns,
								uint16_t				inTag,
								XView*					inNextView,
								XFont::Font*			inFont,
								uint16_t				inCellWidth,
								uint16_t				inCellHeight);
	void					begin(
								const uint8_t*			inPortPinMap);
	bool					InvertPinOrder(
								bool					inPin40isPin1,
								bool					inUpdate = false);
	void					Clear(
								bool					inUpdate = true);
	void					Update(void);
	void					LoadPinConfig(
								const uint8_t*			inPinConfig);
protected:
	enum EPins
	{
		eNumPins = 40,
		eMaxPinGroups = 5
	};
	enum EPinStatus
	{
		eUnused,		// Pin can be ignored, except for bridge test.
		eVerified,		// Pin continuity has been verified, no bridges.
		eVerifiedBridge,// Pin verified, bridge expected by design.
		eUnverified,	// Pin continuity has not been verified.
		eBridgeWarning,	// Pin unexpected bridge detected.
		eBridgeExpectedWarning // Expected bridge but is single
	};
	bool					mInvertPinOrder;
	const uint8_t*			mPortPinMap;
	uint16_t				mPinGroupMask[eMaxPinGroups];
	uint8_t					mPinGridColorIdx[eNumPins];
	const uint8_t*			mPinConfig;
	uint32_t				mPullState;
	uint32_t				mPinsState[eMaxPinGroups];
	MSPeriod				mPinsStateDebouncePeriod;
	static const uint16_t	kPinGridColors[];
		
	void					SetPinStatus(
								uint16_t				inCellIndex,
								EPinStatus				inStatus,
								bool					inUpdate = true);
	void					SetPinsPullState(
								uint32_t				inPullState);
};

#endif // XPinTesterGrid_h
