/*
*	STM32UnixRTC.h, Copyright Jonathan Mackey 2022
*
*	Class to manage the date and time for an STM32.
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
#ifndef STM32UnixRTC_h
#define STM32UnixRTC_h

#include "UnixTime.h"

class STM32UnixRTC : public UnixTime
{
public:
#ifndef __MACH__
	static void				RTCInit(void);
	static void				SyncRTCToTime(void);
	static void				SecondsCB(
								void*					inUserData);
#endif
};

#endif // STM32UnixRTC_h
