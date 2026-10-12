/*
*	TSettings.h, Copyright Jonathan Mackey 2026
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


#ifndef TSettings_h
#define TSettings_h

#include <inttypes.h>

#ifdef __MACH__
#include <stdio.h>
#define SdFile	FILE
#else
class SdFile;
#endif

namespace Config
{
	struct STSettings
	{
		uint8_t		hourFormat;
		uint16_t	currentView;
		uint16_t	tsMinMax[4];
		int16_t		tsZCalibration[9];
	};
}

class TSettings
{
public:
							TSettings(void);
	bool					ReadFile(
								const char*				inPath);
	bool					WriteFile(
								const char*				inPath,
								const Config::STSettings&		inSettings);
	const Config::STSettings&		Settings(void) const
								{return(mSettings);}
protected:
	SdFile*				mFile;
	Config::STSettings	mSettings;
	
	char					NextChar(void);
	uint8_t					FindKeyIndex(
								const char*				inKey);
	char					SkipWhitespace(
								char					inCurrChar);
	char					SkipWhitespaceAndHashComments(
								char					inCurrChar);
	char					SkipToNextLine(void);
	void					Int32ToString(
								int32_t					inValue,
								char*					outString);
//	char					ReadUInt32Number(
//								uint32_t&				outValue);
	char					ReadInt32Number(
								int32_t&				outValue);
	char					ReadStr(
								char					inDelimiter,
								uint8_t					inMaxStrLen,
								char*					outStr);
	void					WriteStr(
								const char*				inStr);
	void					WriteChar(
								char					inChar);
};

#endif /* TSettings_h */
