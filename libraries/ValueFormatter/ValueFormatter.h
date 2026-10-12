/*
*	ValueFormatter.h, Copyright Jonathan Mackey 2023
*	Helper functions that format integer strings.
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
#ifndef ValueFormatter_h
#define ValueFormatter_h

#include <inttypes.h>

typedef  uint8_t (*ValueFormatterPtr)(int32_t, char*);

struct ValueFormatter
{
	static uint8_t			Int32ToString(
								int32_t					inValue,
								char*					outString);
	/*
	*	inValue is of the form NNdd where dd is the decimal part of the value.
	*	This formatter rounds up the decimal part leaving only the integer.
	*	e.g. 3195 = 32, -3195 = -31, -3192 = -31
	*/
	static uint8_t			Decimal20ToString(
								int32_t					inValue,
								char*					outString);
	/*
	*	inValue is of the form NNdd where dd is the decimal part of the value.
	*	This formatter rounds up the decimal part to only one place after the
	*	decimal.
	*	The resulting string always has 1 place after the decimal point.
	*	e.g. 0 = 0.0
	*/
	static uint8_t			Decimal21ToString(
								int32_t					inValue,
								char*					outString);
	/*
	*	inValue is of the form NNdd where dd is the decimal part of the value.
	*	inValue is of the form -1234 = -12.34
	*	The resulting string always has 2 places after the decimal point.
	*	e.g. 0 = 0.00
	*/
	static uint8_t			Decimal22ToString(
								int32_t					inValue,
								char*					outString);
	/*
	*	inValue is of the form NNdd where dd is the decimal part of the value.
	*	inValue is of the form -1234 = -1.234
	*	The resulting string always has 3 places after the decimal point.
	*	e.g. 0 = 0.000
	*/
	static uint8_t			Decimal23ToString(
								int32_t					inValue,
								char*					outString);
	/*
	*	Converts the value to the nearest bytes, Kb, MB or GB size string.  For
	*	sizes greaterthan 1000, the number is decimal with one place after the
	*	decimal point.  See Decimal21ToString() above.
	*	
	*	inValue is treated as an unsigned 32 bit number.
	*/
	static uint8_t			SizeToString(
								int32_t					inValue,
								char*					outString);
	/*
	*	Converts the value to a 4 byte code. This assumes all 4 bytes are ASCII.
	*	e.g. 0x6D703300 would be converted to "mp3".
	*/
	static uint8_t			Uint32To4ByteCodeString(
								int32_t					inValue,
								char*					outString);
	/*
	*	Appends " dB" to inValue converted to a string.
	*/
	static uint8_t			Int32ToDBString(
								int32_t					inValue,
								char*					outString);
	enum EPressureUnit
	{
		eHectopascal,	// hPa
		eInchesOfWater	// "
	};			
	static EPressureUnit sPressureUnit;
	static const char kHectopascalSuffixStr[];
	static const char kInchesOfWaterSuffixStr[];
	
	static uint8_t			PressureToString(
								int32_t					inValue,
								char*					outString);
	static uint8_t			PressureToStringNoUnit(
								int32_t					inValue,
								char*					outString);
								
	enum ETemperatureUnit
	{
		eCelsius,	// C
		eFahrenheit	// F
	};			
	static ETemperatureUnit	sTemperatureUnit;
	static const char kCTempSuffixStr[];
	static const char kFTempSuffixStr[];

	static uint8_t			TemperatureToString(
								int32_t					inValue,
								char*					outString);
};

#endif // ValueFormatter_h
