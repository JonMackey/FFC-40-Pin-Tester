/*
*	TestMenuItemProvider.h, Copyright Jonathan Mackey 2026
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
#ifndef TestMenuItemProvider_h
#define TestMenuItemProvider_h

#include "XMenuItemProvider.h"

class TestMenuItemProvider : public XMenuItemProvider
{
public:
							TestMenuItemProvider(void);
	virtual uint16_t		GetWidestItemWidth(
								XFont*					inXFont);
protected:
	virtual const char*		GetNthItemStr(
								uint16_t				inIndex);
};
#endif // TestMenuItemProvider_h
