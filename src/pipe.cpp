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
#include "pipe.h"

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

