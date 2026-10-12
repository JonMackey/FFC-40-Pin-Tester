/*
*	XPolygon.cpp, Copyright Jonathan Mackey 2026
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

#include "XPolygon.h"
#include "XRootView.h"
#include "DisplayController.h"
#ifdef __MACH__
	#define map DisplayController::map
	#include <iostream>
#endif

/*********************************** XPolygon ***********************************/
XPolygon::XPolygon(
	const int16_t*	inPolyPts,
	int16_t			inThickness,
	uint16_t		inTag,
	XView*			inNextView,
	uint16_t		inBGColor,
	uint16_t		inPolyColor,
	bool			inEnabled)
	: XView(0, 0, 0, 0, inTag, inNextView, nullptr,
		nullptr, true, inEnabled), mThickness(inThickness), mNumPts(0),
		mPolyPts(inPolyPts), mBGColor(inBGColor), mPolyColor(inPolyColor)
{
	if (inPolyPts && *inPolyPts)
	{
		// Determine the polygon bounds...
		const int16_t*	polyPtsPtr = mPolyPts;
		int16_t	minX = 0x7FFF;
		int16_t	maxX = 0;
		int16_t	minY = 0x7FFF;
		int16_t	maxY = 0;
		int16_t	thisCoord = *(polyPtsPtr++);
		int16_t	numPts = 0;
		for (; thisCoord; thisCoord = *(polyPtsPtr++))
		{
			numPts++;
			if (thisCoord < minX)
			{
				minX = thisCoord;
			}
			if (thisCoord > maxX)
			{
				maxX = thisCoord;
			}
			thisCoord = *(polyPtsPtr++);
			if (thisCoord < minY)
			{
				minY = thisCoord;
			}
			if (thisCoord > maxY)
			{
				maxY = thisCoord;
			}
		}
		mNumPts = numPts;
		if (numPts > 1)
		{
			/*
			*	Account for the fact that lines are drawn centered around the
			*	end points.	 mX, mY, mWidth, and mHeight are only used by
			*	this view to determine if the view needs to be drawn when
			*	another view above it is hidden.
			*/
			int16_t	thicknessAdj = inThickness/2;
			mX = minX - thicknessAdj;
			mY = minY - thicknessAdj;
			mWidth = (maxX - minX) + inThickness;
			mHeight = (maxY - minX) + inThickness;
		}
	}
}

/********************************** DrawSelf **********************************/
void XPolygon::DrawSelf(void)
{
	DisplayController*	display = XRootView::GetInstance()->GetDisplay();
	if (display && mNumPts > 1)
	{
		int16_t	x = -mX;
		int16_t	y = -mY;
		LocalToGlobal(x, y);

		display->SetFGColor(mPolyColor);
		display->SetBGColor(mBGColor);
		const int16_t*	polyPtsPtr = mPolyPts;
		int16_t	x0 = *(polyPtsPtr++) + x;
		int16_t	y0 = *(polyPtsPtr++) + y;
		for (uint16_t ptsLeft = mNumPts-1; ptsLeft; ptsLeft--)
		{
			int16_t	x1 = *(polyPtsPtr++) + x;
			int16_t	y1 = *(polyPtsPtr++) + y;
			display->DrawLine(x0, y0, x1, y1, mThickness);
			x0 = x1;
			y0 = y1;
		}
	}
}
