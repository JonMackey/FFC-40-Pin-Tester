/*
*	FileUtils.h, Copyright Jonathan Mackey 2025
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
#ifndef FileUtils_h
#define FileUtils_h

#include <inttypes.h>

class FileUtils
{
public:
	static bool				IsValidType(
								uint32_t				inType,
								const uint32_t*			inValidTypeList);
	static uint32_t			FileTypeFromFilename(
								const char*				inFilePath);
};

#endif // FileUtils_h
