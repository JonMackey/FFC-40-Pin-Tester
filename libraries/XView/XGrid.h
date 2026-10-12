/*
*	XGrid.h, Copyright Jonathan Mackey 2026
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
#ifndef XGrid_h
#define XGrid_h

#include "XView.h"
#include "XFont.h"

class XGrid : public XView
{
public:
							XGrid(
								int16_t					inX,
								int16_t					inY,
								uint16_t				inRows,
								uint16_t				inColumns,
								uint16_t				inTag,
								XView*					inNextView,
								XFont::Font*			inFont,
								uint16_t				inCellWidth,
								uint16_t				inCellHeight,
								const uint16_t*			inColors,
								uint8_t*				inCellToColor = nullptr,
								const char**			inCellStrs = nullptr,
								uint16_t				inCellCount = 0,
								uint16_t				inNumberCells = 0xFFFF,	// 0xFFFF = don't number
								uint16_t				inBGColor =  XFont::eBlack,
								bool					inEnabled = true);
	virtual void			DrawSelf(void);
	virtual void			MouseDown(
								int16_t					inGlobalX,
								int16_t					inGlobalY);
	void					DrawCell(
								uint16_t				inCellIndex);
	void					DrawCells(
								uint16_t				inRow = 0,
								uint16_t				inColumn = 0,
								bool					inDrawAllCells = true);
	uint8_t*				GetCellToColor(void)	// Cell to color lookup
								{return(mCellToColor);}
protected:
	enum EGridDims
	{
		eCellCornerRadius = 6,
	//	eCellHeight = 34
	//	eCellWidth = 28
		eSpaceBetween = 5,	//	h,v, and border h,v space
		eBorderCornerRadius = eCellCornerRadius + eSpaceBetween
	};

	XFont::Font*			mFont;
	uint16_t				mRows;
	uint16_t				mColumns;
	uint16_t				mCellCount;
	uint16_t				mCellHeight;
	uint16_t				mCellWidth;
	uint16_t				mBGColor;
	const uint16_t*			mColors;
	uint8_t*				mCellToColor;
	const char**			mCellStrs;
	uint16_t				mNumberCells;

	XFont*					MakeFontCurrent(void);
};

#endif // XGrid_h
