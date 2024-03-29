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

#ifndef NOTIFY_H_INCLUDE
#define NOTIFY_H_INCLUDE

#ifdef LIB_NOTIFY
#include <libnotify/notify.h>

void NotifyShow(gchar const* summary, gchar const* body);
void NotifyEnd();

#else

void NotifyShow(UNUSED char const* summary, UNUSED char const* body);
void NotifyEnd();

#endif // LIB_NOTIFY

#endif
