/*
*	XSDMenuItemProvider.cpp, Copyright Jonathan Mackey 2026
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
#include "XSDMenuItemProvider.h"
#include "XFont.h"
#ifdef __MACH__
#include <stdio.h>
#include <string>
#else
#include "SdFat.h"
#endif
#include "FileUtils.h"

#ifdef __MACH__
class FsFile
{
public:
							FsFile(
								const char*				inName = nullptr)
								: mName(inName) {}
	void					SetName(
								const char*				inName)
								{mName = inName;}
	bool					isHidden(void)
								{return(false);}
	bool					getName(
								char*					outName,
								uint32_t				inMaxChars)
								{strncpy(outName, mName, inMaxChars);
								 return(true);}
protected:
	const char*	mName;
};

#if 1
const char*	kTestFilenames[] =
{
	"DustCollectorFull.wav",	// 0
	"FilterNeedsCleaning.wav",	// 1
	"SomeoneAtFrontdoor.WAV",	// 2
	"SomeoneAtBackdoor.mp3",	// 3
	//"ExtTest.txt",
	"YourLightsAreOn.wav",		// 4
	"Power on.wav",				// 5
	"Power on.mp3",				// 6
	nullptr
};
#else
const char*	kTestFilenames[] =
{
	"0.wav",
	"1.wav",
	"2.WAV",
	"3.wav",
	"4.txt",
	"5.wav",
	"6.wav",
	"7.txt",
	"8.wav",
	"9.wav",
	nullptr,
	"10.wav",
	"11.wav",
	"12.wav",
	"13.wav",
	"14.wav",
	"15.wav",
	nullptr
};
#endif
#else
static const char	kNoSDCard[] = "No SD Card";
#endif
/**************************** XSDMenuItemProvider *****************************/
XSDMenuItemProvider::XSDMenuItemProvider(
	pin_t			inSDSelectPin,
	pin_t			inSDDetectPin,
	uint8_t*		inItemStrCache,
	uint16_t		inNumEntries,
	const uint32_t*	inValidTypeList)
	: XCachedMenuItemProvider(inItemStrCache, inNumEntries,
											XSDMenuItemProvider::eMaxStrLen),
	  mSDSelectPin(inSDSelectPin), mSDDetectPin(inSDDetectPin),
	  mValidTypeList(inValidTypeList)
{
}

/******************************* LoadNthItemStr *******************************/
/*
*	This is called by XCachedMenuItemProvider
*/
const char* XSDMenuItemProvider::LoadNthItemStr(
	uint16_t	inItemIndex,
	char*		outFilename)
{
	/*
	*	If the SD card is still in ...
	*/
#ifndef __MACH__
	if (digitalRead(mSDDetectPin) == LOW)
	{
		SdFat	sd;
		bool	success = sd.begin(mSDSelectPin, SD_SCK_MHZ(4));
		if (success)
		{
			FsFile		file;
			FsFile		root;
			uint16_t	fileIndex = 0;
			bool		isValid;

			if (root.open("/"))
			{
				while (file.openNext(&root, O_RDONLY))
				{
					isValid = IsValidItemFile(file, outFilename);
					file.close();
					if (isValid)
					{
						if (inItemIndex == fileIndex)
						{
							break;
						}
						fileIndex++;
					}
				}
			}
			if (inItemIndex != fileIndex)
			{
				// This condition will only happen if the SD card is removed or
				// replaced while the menu is open.
				outFilename[0] = 0;
			}
		} else
		{
			sd.initErrorHalt();
		}
	} else
	{
		strcpy(outFilename, kNoSDCard);
	}
#else
	{
		bool		isValid;
		FsFile		file;
		uint16_t	fileIndex = 0;

		for (const char* const* namePtr = kTestFilenames; *namePtr != nullptr;
				namePtr++)
		{
			file.SetName(*namePtr);
			isValid = IsValidItemFile(file, outFilename);
			if (isValid)
			{
				if (inItemIndex == fileIndex)
				{
					break;
				}
				fileIndex++;
			}
		}
	}
#endif
	return(outFilename);
}

/****************************** IsValidItemFile *******************************/
bool XSDMenuItemProvider::IsValidItemFile(
	FsFile&	inFile,
	char*	outFilename)
{
	bool	isValid = false;
	if (!inFile.isHidden() &&
		inFile.getName(outFilename, eMaxStrLen))
	{
		isValid = mValidTypeList == nullptr;
		if (!isValid)
		{
			uint32_t	fileType = FileUtils::FileTypeFromFilename(outFilename);
			isValid = FileUtils::IsValidType(fileType, mValidTypeList);
		}
	}
	return(isValid);
}

/***************************** GetWidestItemWidth *****************************/
/*
*	It's assumed that inXFont is the current font (XFont::SetFont() was called.)
*/
uint16_t XSDMenuItemProvider::GetWidestItemWidth(
	XFont*	inXFont)
{
#ifndef __MACH__
	ClearCache();
	if (inXFont != nullptr && digitalRead(mSDDetectPin) == LOW)
	{
		SdFat	sd;
		bool	success = sd.begin(mSDSelectPin, SD_SCK_MHZ(4));
		if (success)
		{
			FsFile		file;
			FsFile		root;
			uint16_t	validFileCount = 0;

			if (root.open("/"))
			{
				uint16_t	widest = 0;
				uint16_t	thisHeight, thisWidth;
				char		filename[eMaxStrLen];
				while (file.openNext(&root, O_RDONLY))
				{
					if (IsValidItemFile(file, filename))
					{
						validFileCount++;
						inXFont->MeasureStr(filename, thisHeight, thisWidth);
						if (widest < thisWidth)
						{
							widest = thisWidth;
						}
					}
					file.close();
				}
				mWidestItemWidth = widest;
			}
			mItemCount = validFileCount;
		} else
		{
			sd.initErrorHalt();
		}
	} else if (inXFont)
	{
		uint16_t	thisHeight;
		inXFont->MeasureStr(kNoSDCard, thisHeight, mWidestItemWidth);
		mItemCount = 1;
	} else
	{
		mItemCount = 0;
	}
#else
	if (mWidestItemWidth == 0 && inXFont)
	{
		FsFile		file;
		uint16_t	widest = 0;
		uint32_t	validFileCount = 0;
		uint16_t	thisHeight, thisWidth;
		char		filename[eMaxStrLen];
		
		for (uint32_t i = 0; kTestFilenames[i] != nullptr; i++)
		{
			file.SetName(kTestFilenames[i]);
			if (IsValidItemFile(file, filename))
			{
				validFileCount++;
				inXFont->MeasureStr(filename, thisHeight, thisWidth);
				if (widest < thisWidth)
				{
					widest = thisWidth;
				}
			}
		}
		mWidestItemWidth = widest;
		mItemCount = validFileCount;
	}
#endif
	UpdateMenuItemList();
	return(mWidestItemWidth);
}
