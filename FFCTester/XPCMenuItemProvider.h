/*
*	XPCMenuItemProvider.h, Copyright Jonathan Mackey 2026
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
#ifndef XPCMenuItemProvider_h
#define XPCMenuItemProvider_h

#include "XCachedMenuItemProvider.h"

class PinConfigs;

//typedef	bool (*FileFilterCB)(FsFile& inFile, char* inName);

class XPCMenuItemProvider : public XCachedMenuItemProvider
{
public:
							XPCMenuItemProvider(
								uint8_t*				inItemStrCache,
								uint16_t				inNumEntries);
	virtual uint16_t		GetWidestItemWidth(
								XFont*					inXFont);
	void					SetPinConfigs(
								PinConfigs*				inPinConfigs)
								{mPinConfigs = inPinConfigs;}
	
	enum { eMaxStrLen = 64 };
protected:
	PinConfigs*		mPinConfigs;

	virtual const char*		LoadNthItemStr(
								uint16_t				inIndex,
								char*					outFilename);
};
#endif // XPCMenuItemProvider_h
