/*
*	TSettings.cpp, Copyright Jonathan Mackey 2026
*
*	This is a minimal parser for key=value files of the same form as
*	boards.txt and platform.txt accept this class interprets the values as
*	either strings or numbers with support for notted values and hex values.
*	Only the select set of keys defined below are read.  Any other keys are
*	skipped.
*
*	To keep the code size small, very little error checking performed.
*	It's assumed that the config file was created by TSettings::WriteFile
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
#include "TSettings.h"
#ifndef __MACH__
#include <Arduino.h>
#include "SdFat.h"
#include "sdios.h"
#else
#include <string>
#define _BV(bit) (1 << (bit))
#endif
#include "UnixTime.h"

/*
*	List of keys
*/
const char kCurrentViewKeyStr[] = "currentView";
const char kHourFormat24KeyStr[] = "hourFormat";
const char kTsXMaxKeyStr[] = "tsXMax";
const char kTsXMinKeyStr[] = "tsXMin";
const char kTsYMaxKeyStr[] = "tsYMax";
const char kTsYMinKeyStr[] = "tsYMin";
const char kTsZCal0KeyStr[] = "tsZCal0";
const char kTsZCal1KeyStr[] = "tsZCal1";
const char kTsZCal2KeyStr[] = "tsZCal2";
const char kTsZCal3KeyStr[] = "tsZCal3";
const char kTsZCal4KeyStr[] = "tsZCal4";
const char kTsZCal5KeyStr[] = "tsZCal5";
const char kTsZCal6KeyStr[] = "tsZCal6";
const char kTsZCal7KeyStr[] = "tsZCal7";
const char kTsZCal8KeyStr[] = "tsZCal8";


const char* const kSettingsKeys[] =
{	// Sorted alphabetically
	kCurrentViewKeyStr,
	kHourFormat24KeyStr,
	kTsXMaxKeyStr,
	kTsXMinKeyStr,
	kTsYMaxKeyStr,
	kTsYMinKeyStr,
	kTsZCal0KeyStr,
	kTsZCal1KeyStr,
	kTsZCal2KeyStr,
	kTsZCal3KeyStr,
	kTsZCal4KeyStr,
	kTsZCal5KeyStr,
	kTsZCal6KeyStr,
	kTsZCal7KeyStr,
	kTsZCal8KeyStr
};

enum EKeyIndexes
{
	eInvalidKeyIndex,
	// Must align with kSettingsKeys
	eCurrentView,
	eHourFormat24,
	eTsXMax,
	eTsXMin,
	eTsYMax,
	eTsYMin,
	eTsZCal0,
	eTsZCal1,
	eTsZCal2,
	eTsZCal3,
	eTsZCal4,
	eTsZCal5,
	eTsZCal6,
	eTsZCal7,
	eTsZCal8,
	eKeyCount
};

static const uint32_t kNumericKeysMask =
						_BV(eCurrentView) |
						_BV(eHourFormat24) |
						_BV(eTsXMax) |
						_BV(eTsXMin) |
						_BV(eTsYMax) |
						_BV(eTsYMin) |
						_BV(eTsZCal0) |
						_BV(eTsZCal1) |
						_BV(eTsZCal2) |
						_BV(eTsZCal3) |
						_BV(eTsZCal4) |
						_BV(eTsZCal5) |
						_BV(eTsZCal6) |
						_BV(eTsZCal7) |
						_BV(eTsZCal8);

/********************************* TSettings **********************************/
TSettings::TSettings(void)
: mFile(nullptr)
{
}

/********************************** ReadFile **********************************/
/*
*	It's assumed SdFat.begin was successfully called prior to calling this
*	routine.
*
*	This routine returns true if the file contains all of the key values.
*/
bool TSettings::ReadFile(
	const char*	inPath)
{
	mSettings = {0};
	uint8_t	requiredKeyValues = 0;
#ifndef __MACH__
	SdFile file;
	bool	fileOpened = file.open(inPath, O_RDONLY);
	mFile = &file;
#else
	mFile = fopen(inPath, "r+");
	bool	fileOpened = mFile != nullptr;
#endif
	if (fileOpened)
	{
		char	thisChar = NextChar();
		while (thisChar)
		{
			thisChar = SkipWhitespaceAndHashComments(thisChar);
			if (thisChar)
			{
				char keyStr[32];
				keyStr[0] = thisChar;
				thisChar = ReadStr('=', 31, &keyStr[1]);
				while (thisChar)
				{
					uint8_t keyIndex = FindKeyIndex(keyStr);
					if (keyIndex)
					{
						requiredKeyValues++;
						if (kNumericKeysMask & (_BV(keyIndex)))
						{
							int32_t	value;
							thisChar = ReadInt32Number(value);
							switch (keyIndex)
							{
								case eCurrentView:
									mSettings.currentView = value;
									break;
								case eHourFormat24:
									mSettings.hourFormat = value;
									break;
								case eTsXMax:
									mSettings.tsMinMax[1] = value;
									break;
								case eTsXMin:
									mSettings.tsMinMax[0] = value;
									break;
								case eTsYMax:
									mSettings.tsMinMax[3] = value;
									break;
								case eTsYMin:
									mSettings.tsMinMax[2] = value;
									break;
								case eTsZCal0:
								case eTsZCal1:
								case eTsZCal2:
								case eTsZCal3:
								case eTsZCal4:
								case eTsZCal5:
								case eTsZCal6:
								case eTsZCal7:
								case eTsZCal8:
									mSettings.tsZCalibration[keyIndex-eTsZCal0] = value;
									break;

							}
/*						} else
						{
							char valueStr[10];
							thisChar = ReadStr('\n', sizeof(valueStr), valueStr);
							bool	isTrue = valueStr[0] == 't';
							switch (keyIndex)
							{
								case eBinMotorEnabled:
									mSettings.binMotorEnabled = isTrue;
									break;
								case eDisplayPressure:
									mSettings.displayPressure = isTrue;
									break;
								case ePressureUnit:
									mSettings.pressureUnit_hPa = isTrue;
									break;
								case eTemperatureUnit:
									mSettings.temperatureUnitCelsius = isTrue;
									break;
							}
*/						}
					} else
					{
						thisChar = SkipToNextLine();
					}
					break;
				}
			}
		}
	#ifndef __MACH__
		mFile->close();
	#else
		fclose(mFile);
	#endif
		mFile = nullptr;
	}
	return(requiredKeyValues == eTsZCal8);
}

/********************************** WriteFile *********************************/
bool TSettings::WriteFile(
	const char*					inPath,
	const Config::STSettings&	inSettings)
{
#ifndef __MACH__
	SdFile file;
	SdFile::dateTimeCallback(UnixTime::SDFatDateTimeCB);
	bool	fileOpened = file.open(inPath, O_WRONLY | O_TRUNC | O_CREAT);
	mFile = &file;
#else
	mFile = fopen(inPath, "w");
	bool	fileOpened = mFile != nullptr;
#endif
	if (fileOpened)
	{
		
		for (uint8_t keyIndex = 1; keyIndex < eKeyCount; keyIndex++)
		{
			
			WriteStr(kSettingsKeys[keyIndex-1]);
			WriteChar('=');
			if (kNumericKeysMask & (_BV(keyIndex)))
			{
				int32_t	value = 0;
				switch (keyIndex)
				{
					case eCurrentView:
						value = inSettings.currentView;
						break;
					case eHourFormat24:
						value = inSettings.hourFormat;
						break;
					case eTsXMax:
						value = inSettings.tsMinMax[1];
						break;
					case eTsXMin:
						value = inSettings.tsMinMax[0];
						break;
					case eTsYMax:
						value = inSettings.tsMinMax[3];
						break;
					case eTsYMin:
						value = inSettings.tsMinMax[2];
						break;
					case eTsZCal0:
					case eTsZCal1:
					case eTsZCal2:
					case eTsZCal3:
					case eTsZCal4:
					case eTsZCal5:
					case eTsZCal6:
					case eTsZCal7:
					case eTsZCal8:
						value = inSettings.tsZCalibration[keyIndex-eTsZCal0];
						break;
				}
				char valueStr[15];
				Int32ToString(value, valueStr);
				WriteStr(valueStr);
/*			} else
			{
				bool	isTrue = false;;
				switch (keyIndex)
				{
					case eBinMotorEnabled:
						isTrue = inSettings.binMotorEnabled;
						break;
					case eDisplayPressure:
						isTrue = inSettings.displayPressure;
						break;
					case ePressureUnit:
						isTrue = inSettings.pressureUnit_hPa;
						break;
					case eTemperatureUnit:
						isTrue = inSettings.temperatureUnitCelsius;
						break;
				}
				WriteStr(isTrue ? "true":"false");
*/			}
			WriteChar('\n');
		}
	#ifndef __MACH__
		mFile->close();
	#else
		fclose(mFile);
	#endif
	}
	return(fileOpened);
}

/********************************** NextChar **********************************/
char TSettings::NextChar(void)
{
	char	thisChar;
#ifdef __MACH__
	thisChar = getc(mFile);
	if (thisChar == -1)
	{
		thisChar = 0;
	}
#else
	if (mFile->read(&thisChar,1) != 1)
	{
		thisChar = 0;
	}
#endif
	return(thisChar);
}

/******************************** FindKeyIndex ********************************/
/*
*	Returns the index of inKey within the array kSettingsKeys + 1.
*	Returns 0 if inKey is not found.
*/
uint8_t TSettings::FindKeyIndex(
	const char*	inKey)
{
	uint8_t leftIndex = 0;
	uint8_t rightIndex = (sizeof(kSettingsKeys)/sizeof(char*)) -1;
	while (leftIndex <= rightIndex)
	{
		uint8_t current = (leftIndex + rightIndex) / 2;
		const char* currentPtr = kSettingsKeys[current];
		int cmpResult = strcmp(inKey, currentPtr);
		if (cmpResult == 0)
		{
			return(current+1);	// Add 1, 0 is reserved for "not found"
		} else if (cmpResult <= 0)
		{
			rightIndex = current - 1;
		} else
		{
			leftIndex = current + 1;
		}
	}
	return(0);
}

/******************************* SkipWhitespace *******************************/
char TSettings::SkipWhitespace(
	char	inCurrChar)
{
	char	thisChar = inCurrChar;
	for (; thisChar != 0; thisChar = NextChar())
	{
		if (isspace(thisChar))
		{
			continue;
		}
		break;
	}
	return(thisChar);
}

/*********************** SkipWhitespaceAndHashComments ************************/
char TSettings::SkipWhitespaceAndHashComments(
	char	inCurrChar)
{
	char	thisChar = inCurrChar;
	while (thisChar)
	{
		if (isspace(thisChar))
		{
			thisChar = NextChar();
			continue;
		} else if (thisChar == '#')
		{
			thisChar = SkipToNextLine();
			continue;
		}
		break;
	}
	return(thisChar);
}

/******************************* SkipToNextLine *******************************/
/*
*	Returns the character following the newline (if any).
*	Does not support Windows CRLF line endings.
*/
char TSettings::SkipToNextLine(void)
{
	char thisChar = NextChar();
	for (; thisChar; thisChar = NextChar())
	{
		if (thisChar != '\n')
		{
			continue;
		}
		thisChar = NextChar();
		break;
	}
	return(thisChar);
}

/******************************** Int32ToString *******************************/
void TSettings::Int32ToString(
	int32_t	inValue,
	char*	outString)
{
	if (inValue < 0)
	{
		*(outString++) = '-';
		inValue = -inValue;
	}
	for (int32_t num = inValue; num/=10; outString++);
	outString[1] = 0;
	do
	{
		*(outString--) = (inValue % 10) + '0';
		inValue /= 10;
	} while (inValue);
}

/****************************** ReadInt32Number *******************************/
/*
*	Returns the first non-whitespace character following the number.
*	If no digits were found, inNulValue is returned in outValue.
*/
char TSettings::ReadInt32Number(
//	int32_t		inNulValue,
	int32_t&	outValue)
{
	int32_t	value = 0;
	bool	isNeg = false;
//	bool	digitsConsumed = false;
	char	thisChar = SkipWhitespace(NextChar());
	if (thisChar)
	{
		if (thisChar == '-')
		{
			isNeg = true;
			thisChar = NextChar();
		}
		for (; isdigit(thisChar); thisChar = NextChar())
		{
//			digitsConsumed = true;
			value = (value * 10) + (thisChar - '0');
		}
		if (isNeg)
		{
			value = -value;
		}
	}
	thisChar = SkipWhitespace(thisChar);
	
	outValue = value;
//	outValue = digitsConsumed > 0 ? value : inNulValue;
	return(thisChar);
}

#if 0
/****************************** ReadUInt32Number ******************************/
char TSettings::ReadUInt32Number(
	uint32_t&	outValue)
{
	bool		bitwiseNot = false;
	bool 		isHex = false;
	uint32_t	value = 0;
	char		thisChar = SkipWhitespaceAndHashComments(NextChar());
	if (thisChar)
	{
		bitwiseNot = thisChar == '~';
		/*
		*	If notted THEN
		*	get the next char after the not.
		*/
		if (bitwiseNot)
		{
			thisChar = SkipWhitespaceAndHashComments(NextChar());
		}
		if (thisChar == '0')
		{
			thisChar = NextChar();	// Get the character following the leading zero.
			isHex = thisChar == 'x';
			/*
			*	If this is a hex prefix THEN
			*	get the character following it.
			*/
			if (isHex)
			{
				thisChar = NextChar();
			}
		}
		// Now just consume characters till a non valid numeric char is hit.
		if (isHex)
		{
			for (; isxdigit(thisChar); thisChar = NextChar())
			{
				thisChar -= '0';
				if (thisChar > 9)
				{
					thisChar -= 7;
					if (thisChar > 15)
					{
						thisChar -= 32;
					}
				}
				value = (value << 4) + thisChar;
			}
		} else
		{
			for (; isdigit(thisChar); thisChar = NextChar())
			{
				value = (value * 10) + (thisChar - '0');
			}
		}
		thisChar = SkipWhitespaceAndHashComments(thisChar);
	}
	if (bitwiseNot)
	{
		value = ~value;
	}
	outValue = value;
	return(thisChar);
}
#endif
/********************************** ReadStr ***********************************/
/*
*	Reads the string till the delimiter is hit.
*	Once the string is full (inMaxStrLen), characters are ignored till the
*	delimiter is hit.
*	Hash (#) comments are not allowed because the strings aren't quoted.
*/
char TSettings::ReadStr(
	char	inDelimiter,
	uint8_t	inMaxStrLen,
	char*	outStr)
{
	char		thisChar;
	char*		endOfStrPtr = &outStr[inMaxStrLen -1];

	while((thisChar = NextChar()) != 0)
	{
		if (thisChar != inDelimiter)
		{
			/*
			*	If outStr isn't full THEN
			*	append thisChar to it.
			*/
			if (outStr < endOfStrPtr)
			{
				*(outStr++) = thisChar;
			} // else discard the character, the outStr is full
			continue;
		}
		break;
	}
	*outStr = 0;	// Terminate the string
	return(thisChar);
}

/********************************** WriteStr **********************************/
void TSettings::WriteStr(
	const char*	inStr)
{
#ifdef __MACH__
	fwrite(inStr, strlen(inStr), 1, mFile);
#else
	mFile->write(inStr ,strlen(inStr));
#endif
}

/********************************** WriteChar *********************************/
void TSettings::WriteChar(
	char	inChar)
{
#ifdef __MACH__
	putc(inChar, mFile);
#else
	mFile->write(&inChar ,1);
#endif
}

