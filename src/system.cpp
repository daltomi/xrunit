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
#include "config.h"
#include "system.h"

void System(char const* const exec, char* const* argv)
{
	int wpid = 0, status = 0;

	SanitizeEnv();

	if (fork() == 0)
	{
		errno = 0;

		int ret = execvp(exec, argv);

		if (ret == -1)
		{
			STOP("There was a failure while executing the process %s, error=%s\n", exec, strerror(errno));
		}

		if (127 == WEXITSTATUS(ret))
		{
			STOP("The command could not be executed:\n%s", exec);
		}

		if (WIFEXITED(ret))
		{
			if (WEXITSTATUS(ret) != 0)
			{
				STOP("The command was executed but ended with error.\n%s", exec);
			}
		}

		exit(EXIT_SUCCESS);
	}

	while ((wpid = wait(&status)) > 0);
}


void SanitizeEnv(void)
{
	char* display = getenv("DISPLAY");

	ASSERT_STRING(display);

	char* xauthority = getenv("XAUTHORITY");

	ASSERT_STRING(xauthority);

	if (clearenv() != 0)
	{
		STOP("clearenv() function failed");
	}

	size_t const n = confstr(_CS_PATH, 0, 0);

	if (n == 0)
	{
		STOP("confstr(_CS_PATH, 0, 0) function failed");
	}

	char* pathbuf = (char*)calloc(n, sizeof(char));

	ASSERT_DBG(pathbuf);

	if (confstr(_CS_PATH, pathbuf, n) == 0)
	{
		free(pathbuf);
		STOP("confstr(_CS_PATH, pathbuf, n) function failed");
	}

	errno = 0;

	if (setenv("PATH", pathbuf, 1) == -1)
	{
		free(pathbuf);
		STOP("setenv(PATH) function failed: %s", strerror(errno));
	}

	free(pathbuf);
	pathbuf = (char*)NULL;

	errno = 0;

	if (setenv("IFS", "\t\n", 1) == -1)
	{
		STOP("setenv(IFS) function failed: %s", strerror(errno));
	}

	errno = 0;

	if (setenv("DISPLAY", display, 1) == -1)
	{
		STOP("setenv(DISPLAY) function failed: %s", strerror(errno));
	}

	errno = 0;

	if (setenv("XAUTHORITY", xauthority, 1) == -1)
	{
		STOP("setenv(XAUTHORITY) function failed: %s", strerror(errno));
	}
}


bool FileAccessOk(char const* const fileName, bool showError)
{
	ASSERT_DBG_STRING(fileName);

	if (-1 == access(fileName, F_OK | R_OK | W_OK))
	{
		if (showError)
		{
			fl_alert("File: %s, error: %s", fileName, strerror(errno));
		}

		return false;
	}

	return true;
}


bool isFileTypeELF(char const* const fileName, bool showError)
{
	errno = 0;

	char buffer[4] = {0};

	int fd = open(fileName, O_RDONLY);

	if (-1 == fd)
	{
		goto showError_returnFalse;
	}

	errno = 0;

	if (read(fd, buffer, 4) == -1)
	{
		close(fd);
		goto showError_returnFalse;

	}

	close(fd);

	if (buffer[0] == 0x7F &&
		buffer[1] == 0x45 &&
		buffer[2] == 0x4C &&
		buffer[3] == 0x46)
	{
		WARNING_DBG("ELF file detected: %s", fileName);
		return true;
	}

	return false;

showError_returnFalse:

	if (showError)
	{
		fl_alert("File: %s, error: %s", fileName, strerror(errno));
	}
	return false;
}


bool DirAccessOk(char const* const dirName, bool showError)
{
	ASSERT_DBG_STRING(dirName);

	struct stat st;

	errno = 0;

	if (lstat(dirName, &st) == -1)
	{
		if (showError)
		{
			fl_alert("Dir: %s, error: %s", dirName, strerror(errno));
		}
		return false;
	}

	if (S_IFDIR == (st.st_mode & S_IFMT))
	{
		return true;
	}
	return false;
}


char* GetModifyFileTime(char const* const fileName)
{
	ASSERT_DBG_STRING(fileName);

	struct stat st;

	if (lstat(fileName, &st) == -1)
	{
		return NULL;
	}

	return ctime(&st.st_mtime);
}


FILE* PipeOpen(char const* const cmd)
{
	ASSERT_DBG_STRING(cmd);

	SanitizeEnv();

	FILE* pipe = popen(cmd, "r");

	if (!pipe)
	{
		fl_alert("Failed to open the pipe.\nCommand line: %s", cmd);
		exit(EXIT_FAILURE);
	}

	return pipe;
}


void PipeClose(FILE* pipe)
{
	ASSERT_DBG(pipe);
	pclose(pipe);
}


void FileToExecutableMode(char const* const fileName)
{
	ASSERT_DBG_STRING(fileName);

	errno = 0;

	mode_t const user = S_IRUSR | S_IWUSR | S_IXUSR;
	mode_t const group = S_IXGRP | S_IRGRP;
	mode_t const other = S_IXOTH | S_IROTH;

	if (-1 == chmod(fileName,  user | group | other))
	{
		fl_alert("File mode change failed: %s\nError:%s", fileName, strerror(errno));
	}
}

bool MakeDir(char const* const dirName, bool showError)
{
	ASSERT_DBG_STRING(dirName);

	errno = 0;

	if (mkdir(dirName, 0777) == -1)
	{
		if (showError)
		{
			fl_alert("Make dir failed: %s\nError:%s", dirName, strerror(errno));
		}
		return false;
	}

	return true;
}


bool MakeFile(char const* const fileName, bool showError)
{
	ASSERT_DBG_STRING(fileName);

	int fd = -1;

	mode_t const mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

	errno = 0;

	fd = creat(fileName, mode);

	if (fd == -1)
	{
		if (showError)
		{
			fl_alert("The file '%s' could not be created.\nError:%s", fileName, strerror(errno));
		}
		return false;
	}
	close(fd);
	return true;
}


static int RemoveRecursiveCb(char const* path, UNUSED const struct stat* sb, int flag, UNUSED struct FTW* ftw)
{
	if (flag != FTW_NS || flag != FTW_DNR)
	{
		errno = 0;

		if (remove(path) == -1)
		{
			MESSAGE("Failed remove:'%s'\nError:%s", path, strerror(errno));
		}
	}

	return 0;
}


void RemoveRecursive(char const* const path)
{
	ASSERT_DBG_STRING(path);

	errno = 0;

	if (nftw(path, RemoveRecursiveCb, 20, FTW_DEPTH) == -1)
	{
		fl_alert("There was a failure to recursively\n"
				"delete the directory '%s'\nError:%s", path, strerror(errno));
	}
}


void ListDirectories(char const* const path, void(*ListDirectoriesCb)(char const* path))
{
	ASSERT_DBG_STRING(path);
	ASSERT_DBG(ListDirectoriesCb);

	struct dirent** dirList = NULL;

	errno = 0;

	int n = scandir(path, &dirList, NULL, alphasort);

	if (n == -1)
	{
		fl_alert("There was a failure to list directories: '%s'\nError:%s", path, strerror(errno));
		return;
	}

	while (n--)
	{
		char const* const dir = dirList[n]->d_name;

		if (!strpbrk(dir, "."))
		{
			ListDirectoriesCb(dir);
		}
		free(dirList[n]);
	}

	free(dirList);
}


bool Link(char const* const target, char const* const linkpath)
{
	ASSERT_DBG_STRING(target);
	ASSERT_DBG_STRING(linkpath);

	errno = 0;

	if (symlink(target, linkpath) == -1)
	{
		fl_alert("The link '%s' could not be created.\nError:%s", linkpath, strerror(errno));
		return false;
	}

	return true;
}


bool Unlink(char const* const pathname)
{
	ASSERT_DBG_STRING(pathname);

	errno = 0;

	if (unlink(pathname) == -1)
	{
		fl_alert("The unlink '%s' failed.\nError:%s", pathname, strerror(errno));
		return false;
	}

	return true;
}


/*http://www.cse.yorku.ca/~oz/hash.html*/
unsigned long Hash(char* str)
{
	ASSERT_DBG_STRING(str);

	unsigned long hash = 5381UL;
	int c;

	while ((c = *str++))
	{
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}

	return hash;
}
