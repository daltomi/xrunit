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
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/fl_ask.H>

#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>

#include "config.h"

#ifdef DEBUG
#define TITLE "xsv  " VERSION " - Debug"
#else
#define TITLE "xsv " VERSION
#endif

#define TITLE_SERVICE TITLE " - Services"

#ifndef TIME_UPDATE
#define TIME_UPDATE 5
#endif

#ifndef SV
#define SV "/usr/bin/sv"
#endif

#ifndef SV_RUN_DIR
#define SV_RUN_DIR "/run/runit/service"
#endif

#ifndef SV_DIR
#define SV_DIR "/etc/runit/sv"
#endif

#ifndef ASK_DOWN_SERVICES
// It doesn't have to be the exact name
#define ASK_DOWN_SERVICES "tty,dbus,udev,elogind"
#endif

#define ASK_DOWN_SERVICE_DELIM ","

#define SV_STATUS " status "
#define SV_UP " up "
#define SV_DOWN " down "
#define SV_LIST SV SV_STATUS " " SV_RUN_DIR"/*"
#define LS_SV "ls -1 " SV_DIR
#define LS_SV_RUN "ls -1 " SV_RUN_DIR
#define SYMLINK "ln -s "
#define UNLINK "unlink "

#define UNUSED __attribute__((unused))

#define SELECT_RESET 1
#define STR_SZ  200
#define BTN_W 80
#define BTN_H 25
#define BTN_X 10
#define BTN_Y 10
#define BTN_PAD 15

#ifndef FONT
#define FONT FL_HELVETICA
#endif

#ifndef FONT_SZ
#define FONT_SZ 11
#endif

enum {
/* Not move */

/* Fl_Button */
	QUIT,
	RUN,
	DOWN,
	ADD,
	CLOSE,
	INSTALL,
	UNINSTALL,
	BTN_MAX,
/* Fl_Hold_Browser */
	ENABLE = 0,
	LIST,
	BROWSER_MAX
};

static void FillBrowserEnable(void);
static void FillBrowserList(void);
static int GetSelected(Fl_Browser const* const brw);
static void RunSv(char const* const service, char const* const action);
static void ShowWindowModal(Fl_Double_Window* const wnd);
static void SetButtonAlign(int const start, int const end, int const align);
static void SetButtonFont(int const start, int const end);
static void SetFont(Fl_Widget* w);
static void SetFont(Fl_Hold_Browser* w);
static void System(char const* const cmd);
static void RemoveNewLine(char* str);

static void QuitCb(UNUSED Fl_Widget* w, UNUSED void* data);
static void SelectCb(UNUSED Fl_Widget* w, UNUSED void* data);
static void RunDownCb(Fl_Widget* w, UNUSED void* data);
static void IntallUninstallCb(Fl_Widget* w, UNUSED void* data);
static void AddServicesCb(UNUSED Fl_Widget* w, UNUSED void* data);
static void TimerCb(UNUSED void* data);

static int  itemSelect[BROWSER_MAX] { [ENABLE] = SELECT_RESET, [LIST] = SELECT_RESET };

static Fl_Hold_Browser* browser[BROWSER_MAX];
static Fl_Button* btn[BTN_MAX];

static char const* STR_ADD = "Add";
static char const* STR_REMOVE = "Remove";

int main(void)
{
	ASSERT((TIME_UPDATE > 1) && (TIME_UPDATE < 100));
	ASSERT((FONT >= 0) && (FONT < SSIZE_MAX));
	ASSERT((FONT_SZ > 0) && (FONT_SZ < SSIZE_MAX));
	ASSERT((strlen(SV_DIR) > 0) && (strlen(SV_DIR) < STR_SZ));
	ASSERT((strlen(SV_RUN_DIR) > 0) && (strlen(SV_RUN_DIR) < STR_SZ));
	ASSERT((strlen(SV) > 0) && (strlen(SV) < STR_SZ));
	ASSERT(strlen(ASK_DOWN_SERVICES) > 0);

	MESSAGE_DBG("TITLE: %s", TITLE);
	MESSAGE_DBG("TIME_UPDATE: %d", TIME_UPDATE);
	MESSAGE_DBG("SV: %s", SV);
	MESSAGE_DBG("SV_DIR: %s", SV_DIR);
	MESSAGE_DBG("SV_RUN_DIR: %s", SV_RUN_DIR);

	if (geteuid() != 0)
	{
		fl_alert("Administrator permissions are required");
		exit(EXIT_FAILURE);
	}

	fl_register_images();

	Fl_Double_Window* wnd = new Fl_Double_Window(600, 400);
	Fl_Group* grp = new Fl_Group(0, 0, wnd->w(), 30);
	btn[QUIT] = new Fl_Button(BTN_X, BTN_Y, BTN_W, BTN_H, "Quit");
	btn[RUN] = new Fl_Button(BTN_W + BTN_PAD, BTN_Y, BTN_W, BTN_H, "Run");
	btn[DOWN] = new Fl_Button(BTN_W * 2 + BTN_PAD, BTN_Y, BTN_W, BTN_H, "Down");
	btn[ADD] = new Fl_Button(BTN_W * 3 + BTN_PAD, BTN_Y, BTN_W + 20, BTN_H, "Service...");

	SetButtonAlign(QUIT, ADD, 256);
	SetButtonFont(QUIT, ADD);

	btn[QUIT]->image(get_icon_quit());
	btn[RUN]->image(get_icon_run());
	btn[DOWN]->image(get_icon_down());
	btn[ADD]->image(get_icon_add());

	btn[QUIT]->callback(QuitCb);
	btn[RUN]->callback(RunDownCb);
	btn[DOWN]->callback(RunDownCb);
	btn[ADD]->callback(AddServicesCb,(void*)wnd);

	{
		Fl_Box *o = new Fl_Box(BTN_W * 6 + BTN_PAD, 0, 10, 10);
		o->box(FL_FLAT_BOX);
		o->hide();
		grp->resizable(o);
	}

	grp->end();

	browser[ENABLE] = new Fl_Hold_Browser(4, 40, wnd->w() - 8, wnd->h() - 48);

	int const columnWidths[] = {
		100, 150, 0
	};

	SetFont(browser[ENABLE]);
	browser[ENABLE]->column_widths(columnWidths);
	browser[ENABLE]->column_char('\t');
	browser[ENABLE]->type(FL_MULTI_BROWSER);
	browser[ENABLE]->callback(SelectCb);

	FillBrowserEnable();

	wnd->label(TITLE);
	wnd->resizable(browser[ENABLE]);
	wnd->end();
	wnd->show();

	Fl::add_timeout(TIME_UPDATE, TimerCb);

	return Fl::run();
}


static void SetFont(Fl_Widget* w)
{
	ASSERT_DBG(w);
	w->labelfont(FONT);
	w->labelsize(FONT_SZ);
}


static void SetFont(Fl_Hold_Browser* w)
{
	ASSERT_DBG(w);
	w->textfont(FONT);
	w->textsize(FONT_SZ);
}

static void SetButtonFont(int const start, int const end)
{
	for (int i = start; i <= end; i++)
	{
		SetFont(btn[i]);
	}
}


static void FillBrowserEnable(void)
{
	char buffer[STR_SZ];

	FILE* psv = 0;

	psv = popen(SV_LIST, "r");

	if (!psv)
	{
		fl_alert("Failed to open the pipe.\nCommand line: %s", SV_LIST);
		exit(EXIT_FAILURE);
	}

	browser[ENABLE]->clear();

	btn[DOWN]->deactivate();
	btn[RUN]->deactivate();

	int iselect_count = SELECT_RESET;

	while (fgets(buffer, STR_SZ, psv))
	{
		buffer[STR_SZ - 1] = '\0';

		char* pb = buffer;

		if (iselect_count++ == itemSelect[ENABLE])
		{

			if (pb[0] == 'd' || pb[0] == 'D')
			{
				btn[RUN]->activate();
			}
			else
			{
				btn[DOWN]->activate();
			}
		}

		char* pc = strchr(pb, ':');

		ASSERT_DBG(pc != 0);
		ASSERT_DBG(*pc != '\0');

		*pc =  '\t';

		browser[ENABLE]->add(pb);

		if (pb[0] == 'd' || pb[0] == 'D')
		{
			browser[ENABLE]->icon(browser[ENABLE]->size(), get_icon_down());
		}
		else if (pb[0] == 'r' || pb[0] == 'R')
		{
			browser[ENABLE]->icon(browser[ENABLE]->size(), get_icon_run());
		}
		else if (pb[0] == 'f' || pb[0] == 'F')
		{
			browser[ENABLE]->icon(browser[ENABLE]->size(), get_icon_finish());
		}
		else
		{
			STOP_DBG("State not contemplated: %s", pb);
		}
	}

	pclose(psv);

	if (browser[ENABLE]->size() == 0)
	{
		fl_alert("No runit service found: %s", SV_LIST);
		exit(EXIT_FAILURE);
	}

	browser[ENABLE]->select(itemSelect[ENABLE]);
}


static void FillBrowserList(void)
{
	char buffer[STR_SZ];

	FILE* pls = 0;
	FILE* plsRun = 0;

	pls = popen(LS_SV, "r");

	if (!pls)
	{
		fl_alert("Failed to open the pipe.\nCommand line: %s", LS_SV);
		exit(EXIT_FAILURE);
	}

	btn[INSTALL]->deactivate();
	btn[UNINSTALL]->deactivate();

	browser[LIST]->clear();

	int iselect_count = SELECT_RESET;
	int item = 1;

	while (fgets(buffer, STR_SZ, pls))
	{
		buffer[STR_SZ - 1] = '\0';

		browser[LIST]->add(buffer, (void*)STR_REMOVE);
		browser[LIST]->icon(item++, get_icon_disable());
	}

	pclose(pls);

	plsRun = popen(LS_SV_RUN, "r");

	if (!plsRun)
	{
		fl_alert("Failed to open the pipe.\nCommand line: %s", LS_SV_RUN);
		exit(EXIT_FAILURE);
	}

	while (fgets(buffer, STR_SZ, plsRun))
	{
		buffer[STR_SZ - 1] = '\0';

		for (int item = 1; item <= browser[LIST]->size(); ++item)
		{
			char const* const find = browser[LIST]->text(item);

			if (strstr(buffer, find))
			{
				browser[LIST]->icon(item, get_icon_enable());
				browser[LIST]->data(item, (void*)STR_ADD);
				break;
			}

		}
	}

	pclose(plsRun);

	for (int item = 1; item <= browser[LIST]->size(); ++item)
	{
		if (iselect_count++ == itemSelect[LIST])
		{
			char* data = (char*)browser[LIST]->data(item);

			if (data == STR_ADD)
			{
				btn[UNINSTALL]->activate();
			}
			else if (data == STR_REMOVE)
			{
				btn[INSTALL]->activate();
			}
			else
			{
				STOP_DBG("State not contemplated: %s", data);
			}
		}
	}

	browser[LIST]->select(itemSelect[LIST]);
}


static void QuitCb(UNUSED Fl_Widget* w, UNUSED void* data)
{
	exit(EXIT_SUCCESS);
}


static int GetSelected(Fl_Browser const* const brw)
{
	for (int i = 1; i <= brw->size(); i++)
	{
		if (brw->selected(i))
		{
			return i;
		}
	}

	int iselected = SELECT_RESET;

	if (brw == browser[ENABLE])
	{
		iselected = itemSelect[ENABLE];
	}
	else
	{
		iselected = itemSelect[LIST];
	}

	return iselected;
}


static void SelectCb(UNUSED Fl_Widget* w, UNUSED void* data)
{
	ASSERT_DBG(w);

	Fl_Hold_Browser* b = (Fl_Hold_Browser*)w;

	int iselected = GetSelected(b);

	if (b == browser[ENABLE])
	{
		itemSelect[ENABLE] = iselected;
		FillBrowserEnable();
	}
	else
	{
		itemSelect[LIST] = iselected;
		FillBrowserList();
	}

}


static void RunDownCb(UNUSED Fl_Widget* w, UNUSED void* data)
{
	Fl_Button* btnId = (Fl_Button*)w;

	char const* buffer = browser[ENABLE]->text(itemSelect[ENABLE]);

	char* service = strdup(buffer);

	ASSERT_DBG(service);

	char* b = strchr(service, '/');
	ASSERT_DBG(b);

	char* e = strchr(b, ':');
	ASSERT_DBG(e);

	std::ptrdiff_t const len = e - b;

	memmove(service, b, len);

	service[len] = '\0';

	if (btnId == btn[RUN])
	{
		RunSv(service, SV_UP);
	}
	else if (btnId == btn[DOWN])
	{
		char* str = strdup(ASK_DOWN_SERVICES);

		ASSERT_DBG(str);

		char* tok = strtok(str, ASK_DOWN_SERVICE_DELIM);

		ASSERT_DBG(tok);

		do
		{
			if (strstr(service, tok))
			{
				MESSAGE_DBG("ASK_DOWN_SERVICES: %s", tok);

				if (0 == fl_choice("Warning: Priority service name detected: '%s'\n"
								"Do you continue?", "Cancel", "Continue", 0, service))
				{
					free(str);
					goto clean;
				}
			}
		} while ((tok = strtok(0, ASK_DOWN_SERVICE_DELIM)));

		free(str);
		RunSv(service, SV_DOWN);
	}
	else
	{
		STOP_DBG("Button identifier not covered: %p", btnId);
	}

clean:
	free(service);
	service = 0;
}


static void RemoveNewLine(char* str)
{
	char* cp = strchr(str, '\n');

	if (cp)
	{
		*cp = '\0';
	}
}


static void IntallUninstallCb(Fl_Widget* w, UNUSED void* data)
{
	ASSERT_DBG(w);

	char dest[STR_SZ];
	char src[STR_SZ];
	#define STR_SZ_2 STR_SZ * 2
	char cmd[STR_SZ_2];

	Fl_Button* btnId = (Fl_Button*)w;

	int const item = GetSelected(browser[LIST]);
	char const* const itemText = browser[LIST]->text(item);

	strncpy(dest, SV_RUN_DIR, STR_SZ - 1);
	strncat(dest, "/", 1);
	strncat(dest, itemText, STR_SZ - 1);
	dest[STR_SZ - 1] = '\0';

	RemoveNewLine(dest);

	if (btnId == btn[INSTALL])
	{
		strncpy(src, SV_DIR, STR_SZ - 1);
		strncat(src, "/", 1);
		strncat(src,  itemText, STR_SZ - 1);
		src[STR_SZ - 1] = '\0';

		RemoveNewLine(src);

		strncpy(cmd, SYMLINK, STR_SZ_2 - 1);
		strncat(cmd, src, STR_SZ_2 - 1);
		strcat(cmd, " ");
		strncat(cmd, dest,STR_SZ_2 - 1);
	}
	else if (btnId == btn[UNINSTALL])
	{
		strncpy(cmd, UNLINK, STR_SZ_2 - 1);
		strcat(cmd, " ");
		strncat(cmd, dest, STR_SZ_2 - 1);
	}
	else
	{
		STOP_DBG("Button identifier not covered: %p", btnId);
	}

	cmd[STR_SZ_2 - 1] = '\0';

	RemoveNewLine(cmd);
	System(cmd);

	FillBrowserEnable();
	FillBrowserList();
}


static void System(char const* const cmd)
{
	int const ret = system(cmd);

	if (ret == -1)
	{
		fl_alert("system(): internal error");
		exit(EXIT_FAILURE);
	}

	if (127 == WEXITSTATUS(ret))
	{
		fl_alert("The command could not be executed:\n%s", cmd);
		exit(EXIT_FAILURE);
	}

	if (WIFEXITED(ret))
	{
		if (WEXITSTATUS(ret) != 0)
		{
			fl_alert("The command was executed but ended with error.\n%s", cmd);
			exit(EXIT_FAILURE);
		}
	}
}


static void RunSv(char const* const service, char const* const action)
{
	char cmd [STR_SZ];

	cmd[STR_SZ - 1] = '\0';

	strncpy(cmd, SV, STR_SZ - 1);
	strncat(cmd, action, STR_SZ - 1);
	strncat(cmd, service, STR_SZ - 1);

	MESSAGE_DBG("exec cmd: %s", cmd);

	System(cmd);

	FillBrowserEnable();
}


static void TimerCb(UNUSED void* data)
{
	FillBrowserEnable();
	Fl::repeat_timeout(TIME_UPDATE, TimerCb);
}


static void SetButtonAlign(int const start, int const end, int const align)
{
	for (int i = start; i <= end; ++i)
	{
		btn[i]->align(align);
	}
}


static void ShowWindowModal(Fl_Double_Window* const wnd)
{
	ASSERT_DBG(wnd);

	wnd->set_modal();
	wnd->show();
	while (wnd->shown())
	{
		Fl::wait();
	}

}


static void CloseWindowCb(UNUSED Fl_Widget* w, void* data)
{
	ASSERT_DBG(data);
	((Fl_Double_Window*)data)->hide();
}


static void AddServicesCb(UNUSED Fl_Widget* w, UNUSED void* data)
{
	ASSERT_DBG(data);

	Fl_Double_Window* wndParent = (Fl_Double_Window*)data;

	Fl_Double_Window* wnd = new Fl_Double_Window(wndParent->x() + 300 / 2,
							wndParent->y(),
							300,
							350,
							TITLE_SERVICE);

	btn[CLOSE] = new Fl_Button(BTN_X, BTN_Y, BTN_W, BTN_H, "Close");
	btn[INSTALL] = new Fl_Button(BTN_W + BTN_PAD, BTN_Y, BTN_W, BTN_H, STR_ADD);
	btn[UNINSTALL] = new Fl_Button(BTN_W * 2 + BTN_PAD, BTN_Y, BTN_W, BTN_H, STR_REMOVE);
	browser[LIST] = new Fl_Hold_Browser(4, 40, wnd->w() - 8, wnd->h() - 48);

	SetFont(browser[LIST]);
	SetButtonFont(CLOSE, UNINSTALL);

	browser[LIST]->callback(SelectCb);

	SetButtonAlign(CLOSE, UNINSTALL, 256);

	btn[CLOSE]->image(get_icon_quit());
	btn[INSTALL]->image(get_icon_add());
	btn[UNINSTALL]->image(get_icon_remove());

	btn[CLOSE]->callback(CloseWindowCb, (void*)wnd);
	btn[INSTALL]->callback(IntallUninstallCb);
	btn[UNINSTALL]->callback(IntallUninstallCb);

	FillBrowserList();

	ShowWindowModal(wnd);

	for (int i = CLOSE; i <= UNINSTALL; i++)
	{
		delete btn[i];
	}

	delete browser[LIST];
	delete wnd;
}
