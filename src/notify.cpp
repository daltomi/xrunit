/*
	Copyright © 2020 daltomi <daltomi@disroot.org>

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
#include "notify.h"

static NotifyNotification *notify = NULL;

static int NotifyInit();


void NotifyShow(gchar const* summary, gchar const* body)
{
	GError *err = NULL;
	gboolean is_ok = FALSE;

	if (NotifyInit() == 0)
	{
		WARNING("notify_init failed.");
		return;
	}

	if (notify == NULL)
	{
		notify = notify_notification_new(NULL, NULL, NULL);

		if (notify == NULL)
		{
			WARNING("notify_notification_new failed. Possibly your notify"
				" daemon is not running.");
			goto error_notification_new;
		}

		notify_notification_set_timeout(notify, 4000);
		notify_notification_set_urgency(notify, NOTIFY_URGENCY_CRITICAL);
	}

	notify_notification_update(notify, summary, body, NULL);

	is_ok = notify_notification_show(notify, &err);

	if (is_ok == FALSE)
	{
		WARNING("notify_notification_show. It is possible that your notification "
			"daemon is not running or is suspended.");
		goto error_notification_show;
	}
	return;

error_notification_show:
	g_error_free(err);

error_notification_new:
	NotifyEnd();
}


static int NotifyInit()
{
	if (!(notify_is_initted() || notify_init("xsv")))
	{
		return 0;
	}

	return 1;
}


void NotifyEnd()
{
	if (notify != NULL)
	{
		notify_uninit();
		g_object_unref(G_OBJECT(notify));
		notify = NULL;
	}
}
