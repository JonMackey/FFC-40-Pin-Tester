/*
*	XGrid.cpp, Copyright Jonathan Mackey 2026
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

#include "XGrid.h"
#include "XRootView.h"
#include "DisplayController.h"
#include "ValueFormatter.h"
#ifdef __MACH__
#include <iostream>
#endif

/*********************************** XGrid ***********************************/
/*
*	When inCellToColor is null, the inColors are 1:1, one color for each cell.
*	When inCellCount is zero, mCellCount = inRows*inColumns.
*/
XGrid::XGrid(
	int16_t			inX,
	int16_t			inY,
	uint16_t		inRows,
	uint16_t		inColumns,
	uint16_t		inTag,
	XView*			inNextView,
	XFont::Font*	inFont,
	uint16_t		inCellWidth,
	uint16_t		inCellHeight,
	const uint16_t*	inColors,
	uint8_t*		inCellToColor,
	const char**	inCellStrs,
	uint16_t		inCellCount,
	uint16_t		inNumberCells,
	uint16_t		inBGColor,
	bool			inEnabled
)
	: XView(inX, inY, 0, 0, inTag, inNextView, nullptr,
		nullptr, true, inEnabled),
		mRows(inRows), mColumns(inColumns), mCellCount(inCellCount),
		mCellWidth(inCellWidth), mCellHeight(inCellHeight), mFont(inFont),
		mColors(inColors), mCellToColor(inCellToColor), mBGColor(inBGColor),
		mCellStrs(inCellStrs), mNumberCells(inNumberCells)
{
	if (inCellCount == 0)
	{
		mCellCount = inRows*inColumns;
	}
	/*
	*	Calculate the view height and width based on the cell dimensions.
	*/
	mWidth = eSpaceBetween + ((inCellWidth+eSpaceBetween) * inColumns);
	mHeight = eSpaceBetween + ((inCellHeight+eSpaceBetween) * inRows);
}

/****************************** MakeFontCurrent *******************************/
XFont* XGrid::MakeFontCurrent(void)
{
	XFont* xFont = nullptr;
	if (mFont)
	{
		xFont = mFont->MakeCurrent();
	}
	return(xFont);
}


/********************************** DrawSelf **********************************/
void XGrid::DrawSelf(void)
{
	XFont*	xFont = MakeFontCurrent();
	if (xFont)
	{
		DisplayController*	display = xFont->GetDisplay();
		display->SetFGColor(XFont::eWhite);
		display->SetBGColor(mBGColor);
		int16_t	x = 0;
		int16_t	y = 0;
		// Note: if mY's global value is zero, FillRect and
		// DrawRoundedRect will fail.
		LocalToGlobal(x, y);

		display->DrawRoundedRect(x, y, mWidth, mHeight, eBorderCornerRadius);
		DrawCells();
	}
}

/******************************** DrawCells *********************************/
/*
*	The default is to draw all of the cells by setting inRow and inColumn to
*	zero, and inDrawAllCells to true.
*
*	To draw a single cell, set the cell's row and column, and inDrawAllCells to
*	false.
*/
void XGrid::DrawCells(
	uint16_t	inRow,
	uint16_t	inColumn,
	bool		inDrawAllCells)
{
	XFont*	xFont = MakeFontCurrent();
	uint32_t	rows = inDrawAllCells ? mRows : inRow +1;
	uint32_t	cellIdx = (inRow * mColumns) + inColumn;
	if (xFont &&
		cellIdx < mCellCount)
	{
		DisplayController*	display = xFont->GetDisplay();
		//display->SetFGColor(kDarkBlueColor);
		display->SetBGColor(XFont::eWhite);
		int16_t	x = 0;
		int16_t	y = 0;
		// Note: if mY's global value is zero, FillRect and
		// DrawRoundedRect will fail.
		LocalToGlobal(x, y);
		
		uint16_t	charTop = (mCellHeight-xFont->GetFontHeader().height)/2;
		
		display->SetFGColor(XFont::eWhite);
		xFont->SetBGTextColor(XFont::eWhite);
		for (uint32_t row = inRow; row < rows; row++)
		{
			uint32_t	lastCellInRow;
			uint16_t	cellLeft = x + eSpaceBetween;
			uint16_t	cellTop = y + (row * (mCellHeight+eSpaceBetween)) + eSpaceBetween;
			if (inDrawAllCells)
			{
				lastCellInRow = (row+1) * mColumns;
				if (lastCellInRow > mCellCount)
				{
					lastCellInRow = mCellCount;
				}
				cellLeft = x + eSpaceBetween;
			} else
			{
				lastCellInRow = cellIdx+1;
				cellLeft = x + (inColumn * (mCellWidth + eSpaceBetween)) + eSpaceBetween;
			}
										
			for (; cellIdx < lastCellInRow; cellIdx++, cellLeft+=(mCellWidth + eSpaceBetween))
			{
				uint16_t	cellColor;
				if (mColors)
				{
					if (mCellToColor)
					{
						cellColor = mColors[mCellToColor[cellIdx]];
					} else
					{
						cellColor = mColors[cellIdx];
					}
				} else
				{
					cellColor = XFont::eGray;
				}
				display->SetFGColor(cellColor);
				xFont->SetBGTextColor(cellColor);
				// Set the text color to black or white depending on the BG color.
				xFont->SetTextColorForBG();
				display->DrawRoundedRect(cellLeft, cellTop, mCellWidth, mCellHeight, eCellCornerRadius);
				if (mNumberCells != 0xFFFF)
				{
					char	cellNumStr[10];
					ValueFormatter::Int32ToString(cellIdx + mNumberCells, cellNumStr);
					xFont->DrawAligned(cellNumStr,
										cellLeft, cellTop+charTop, mCellWidth,
											XFont::eAlignCenter, false);
				} else if (mCellStrs)
				{
					xFont->DrawAligned(mCellStrs[cellIdx],
										cellLeft, cellTop+charTop, mCellWidth,
											XFont::eAlignCenter, false);
				}
			}
		}
	}
}

/********************************** DrawCell **********************************/
void XGrid::DrawCell(
	uint16_t	inCellIndex)
{
	DrawCells(inCellIndex/mColumns, inCellIndex%mColumns, false);
}

/********************************* MouseDown **********************************/
void XGrid::MouseDown(
	int16_t	inGlobalX,
	int16_t	inGlobalY)
{
	int16_t	x = inGlobalX;
	int16_t	y = inGlobalY;
	GlobalToLocal(x, y);
	
	int16_t	cellTop = eSpaceBetween;

	/*
	*	Determine the row
	*/
	for (uint32_t row = 0; row < mRows; row++, cellTop += (mCellHeight+eSpaceBetween))
	{
		if (y >= cellTop)
		{
			if (y <= cellTop+mCellHeight)
			{
				uint32_t	cellIdx = row * mColumns;
				uint32_t	lastCellIdx = cellIdx + mColumns;
				int16_t		cellLeft = eSpaceBetween;
				for (; cellIdx < lastCellIdx; cellIdx++)
				{
					if (x >= cellLeft)
					{
						if (x <= cellLeft+mCellWidth)
						{
							if (cellIdx < mCellCount)
							{
								/*
								*	Because the grid only supports mouse down,
								*	the only change can be a mouse down.  So the
								*	change action is the index of the cell hit
								*	rather than a unique action.
								*/
								HandleChange(this, cellIdx);
								//fprintf(stderr, "Cell hit = %u\n", cellIdx+1);
							}
							break;
						}
						cellLeft += (mCellWidth+eSpaceBetween);
					} else
					{
						break;
					}
				}
				break;
			}
		} else
		{
			break;
		}
	}
}

