/*
*	FileUtils.cpp, Copyright Jonathan Mackey 2025
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

#include "FileUtils.h"

/******************************** IsValidType *********************************/
bool FileUtils::IsValidType(
	uint32_t		inType,
	const uint32_t*	inValidTypeList)
{
	bool	isValid = inValidTypeList == nullptr;
	if (!isValid && inType)
	{
		for (; *inValidTypeList; inValidTypeList++)
		{
			if (inType ==  *inValidTypeList)
			{
				isValid = true;
				break;
			}
		}
	}
	return(isValid);
}

/**************************** FileTypeFromFilename ****************************/
/*
*	Returns up to the first 4 characters of the file extension, if any as a
*	32 bit OSType.  This only works correctly for 8 bit characters. Uppercase
*	characters are converted to lowercase.
*	Returns zero if no extension was found.
*/
uint32_t FileUtils::FileTypeFromFilename(
	const char*		inFilePath)
{
	uint32_t	fileType = 0;
	{
		const char*	extPtr = inFilePath;
		// Point extPtr to the end of the filename
		for (; *extPtr; extPtr++){}
		// Point to the last '.' in the filename
		for (extPtr--; extPtr != inFilePath; extPtr--)
		{
			if (*extPtr != '.')
			{
				continue;
			}
			break;
		}
		/*
		*	If the extension was found...
		*/
		if (extPtr != inFilePath)
		{
			extPtr++;	// Move past the '.'
			uint8_t	extension[4] = {0};
			uint8_t	thisChar;
			for (uint32_t i = 0; i < 4; i++)
			{
				thisChar = extPtr[i];
				if (thisChar)
				{
					if (thisChar >= 'A' &&
						thisChar <= 'Z')
					{
						thisChar += 0x20;
					}
					extension[3-i] = thisChar;
					continue;
				}
				break;
			}
			fileType = *(const uint32_t*)extension;				
		}
	}
	return(fileType);
}
