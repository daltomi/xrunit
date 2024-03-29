/*
	Copyright © 2020 Daniel T. Borelli <danieltborelli@gmail.com>

	This file is part of xrunit.

	xrunit is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	xrunit is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with xrunit.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SYSTEM_H_INCLUDE
#define SYSTEM_H_INCLUDE

void System(char const* const exec, char* const* argv);

void SanitizeEnv(void);

bool FileAccessOk(char const* const fileName, bool showError);

bool DirAccessOk(char const* const dirName, bool showError);

bool isFileTypeELF(char const* const fileName, bool showError);

FILE* PipeOpen(char const* const cmd);

void PipeClose(FILE* pipe);

void FileToExecutableMode(char const* const fileName);

bool MakeDir(char const* const dirName, bool showError);

bool MakeFile(char const* const fileName, bool showError);

void RemoveRecursive(char const* const path);

bool Link(char const* const target, char const* const linkpath);

bool Unlink(char const* const pathname);

char* GetModifyFileTime(char const* const fileName);

unsigned long Hash(char* str);

void ListDirectories(char const* const path, void(*ListDirectoriesCb)(char const* path));

#endif
