/*
	Copyright (C) 2020 daltomi <daltomi@disroot.org>

	This file is part of xsv.

	xsv is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	xsv is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with xsv.  If not, see <http://www.gnu.org/licenses/>.
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
			fl_alert("The command could not be executed:\n%s", exec);
			exit(EXIT_FAILURE);
		}

		if (WIFEXITED(ret))
		{
			if (WEXITSTATUS(ret) != 0)
			{
				fl_alert("The command was executed but ended with error.\n%s", exec);
				exit(EXIT_FAILURE);
			}
		}

		exit(EXIT_SUCCESS);
	}

	while ((wpid = wait(&status)) > 0);
}


void SanitizeEnv(void)
{
	char* display = getenv("DISPLAY");

	ASSERT(display);

	char* xauthority = getenv("XAUTHORITY");

	ASSERT(xauthority);

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
	ASSERT_DBG(fileName);

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


bool DirAccessOk(char const* const dirName, bool showError)
{
	ASSERT_DBG(dirName);

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


FILE* PipeOpen(char const* const cmd)
{
	ASSERT_DBG(cmd);

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
	ASSERT_DBG(fileName);

	errno = 0;

	mode_t const user = S_IRUSR | S_IWUSR |  S_IXUSR;
	mode_t const group = S_IXGRP | S_IRGRP;
	mode_t const other = S_IXOTH | S_IROTH;

	if (-1 == chmod(fileName,  user | group | other))
	{
		fl_alert("File mode change failed: %s\nError:%s", fileName, strerror(errno));
	}
}

bool MakeDir(char const* const dirName, bool showError)
{
	ASSERT_DBG(dirName);

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
