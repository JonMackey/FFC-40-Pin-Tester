/*
*	XViewID.h, Copyright Jonathan Mackey 2026
*
*	This is a const list of unique IDs assigned to classes that need to be
*	identified at runtime without using RTTI
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
#ifndef XViewID_h
#define XViewID_h

#include <inttypes.h>

namespace XViewID
{
	const uint16_t	kXViewID			= 0;
	// IDs 1 through 10 are reserved for text fields.
	// At some point XDateValueField may be modified to support the keyboard.
	const uint16_t	kXTextFieldID		= 1;
	const uint16_t	kXKeyboardID		= 11;
}

#endif // XViewID_h
