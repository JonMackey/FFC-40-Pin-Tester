/*
*	STM32UnixRTC.cpp, Copyright Jonathan Mackey 2022
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
#include "STM32UnixRTC.h"


#ifndef __MACH__
#include "Arduino.h"
#include "rtc.h"

//#if defined(STM32_CORE_VERSION) && (STM32_CORE_VERSION  > 0x01090000)
//	#include "rtc.h"
//#endif
// Check if RTC HAL enable in variants/board_name/stm32yzxx_hal_conf.h
#ifndef HAL_RTC_MODULE_ENABLED
	#error "RTC configuration is missing. Check flag HAL_RTC_MODULE_ENABLED in variants/board_name/stm32yzxx_hal_conf.h"
#endif
/*
*	The original STM32UnixRTC code maintained the 32 bit Unix time by directly
*	accessing the time stored in the 16 bit values CNTL and CNTH.  CNTL and CNTH
*	are only available in the F1xx family.  The remaining stm32 families
*	maintain a time structure that isn't compatible with a simple 32 bit value. 
*	For this reason the STM32UnixRTC code has been modified to use the stm32
*	time structure and converts to and from the Unix time as needed.
*/

/********************************** RTCInit ***********************************/
void STM32UnixRTC::RTCInit(void)
{
	// LSE = low-speed external 32.768 kHz crystal
	// RTC_init will only fully initialize if the RTC has never been initialized.
	// The RTC_init writes a marker to the backup memory to determine if it
	// needs to be initialized.  So it's safe to call on reset/startup.
	if (RTC_init(HOUR_FORMAT_24, MODE_BINARY_ONLY, LSE_CLOCK, false))
	{
		sTime = 0x6423FFF0;
		SyncRTCToTime();
	} else
	{
		SComponents	ct;
		uint8_t	year99;
		uint8_t	unusedWDay;
		RTC_GetDate(&year99, &ct.month, &ct.day, &unusedWDay);
		RTC_GetTime(&ct.hour, &ct.minute, &ct.second, NULL, NULL);
		ct.year = year99;
		sTime = FromComponents(ct);
	}

	attachSecondsIrqCallback(SecondsCB);
	ResetSleepTime();
}

/********************************** SyncTime **********************************/
void STM32UnixRTC::SyncRTCToTime(void)
{
	SComponents	ct;
	ToComponents(sTime, ct);
	// Note that subSeconds and period are ignored ,,,0, HOUR_AM
	RTC_SetTime(ct.hour, ct.minute, ct.second, 0, HOUR_AM);
	uint8_t wday = DayOfWeek(sTime);
#ifndef STM32F1xx
	// 0=SUN -> 7=SUN
	if (wday == 0)
	{
		wday = 7;
	}
#endif
	RTC_SetDate(ct.year-2000, ct.month, ct.day, wday);
}

/********************************* SecondsCB **********************************/
void STM32UnixRTC::SecondsCB(
	void*	inUserData)
{
	UnixTime::Tick();
}

#endif	// __MACH__

