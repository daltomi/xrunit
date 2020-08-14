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
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>

#include "config.h"

#ifdef DEBUG
#define TITLE "xsv " VERSION " - Debug"
#else
#define TITLE "xsv " VERSION
#endif

#define TITLE_SERVICE TITLE " - Services"

#ifndef TIME_UPDATE
#define TIME_UPDATE 5
#endif

#ifndef SV
#define SV "sv"
#endif

#ifndef SV_RUN_DIR
#define SV_RUN_DIR "/run/runit/service"
#endif

#ifndef SV_DIR
#define SV_DIR "/etc/runit/sv"
#endif

#ifndef ASK_SERVICES
// It doesn't have to be the exact name
#define ASK_SERVICES "tty,dbus,udev,elogind"
#endif

#define ASK_SERVICES_DELIM ","

// popen cmds
#define SV_LIST SV " status " SV_RUN_DIR"/*"
#define LS_SV "ls -1 " SV_DIR
#define LS_SV_RUN "ls -1 " SV_RUN_DIR

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
	RESTART,
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

void FillBrowserEnable(void);
void FillBrowserList(void);
int GetSelected(Fl_Browser const* const brw);
void RunSv(char const* const service, char const* const action);
void ShowWindowModal(Fl_Double_Window* const wnd);
void SetButtonAlign(int const start, int const end, int const align);
void SetButtonFont(int const start, int const end);
void SetFont(Fl_Widget* w);
void SetFont(Fl_Hold_Browser* w);
void System(char const* const exec, char* const* argv);
void SanitizeEnv(void);
void RemoveNewLine(char* str);
bool AskIfContinue(char const* const service);

void QuitCb(UNUSED Fl_Widget* w, UNUSED void* data);
void SelectCb(Fl_Widget* w, UNUSED void* data);
void CommandCb(Fl_Widget* w, UNUSED void* data);
void IntallUninstallCb(Fl_Widget* w, UNUSED void* data);
void AddServicesCb(UNUSED Fl_Widget* w, void* data);
void TimerCb(UNUSED void* data);

static int  itemSelect[BROWSER_MAX] { [ENABLE] = SELECT_RESET, [LIST] = SELECT_RESET };

static Fl_Hold_Browser* browser[BROWSER_MAX];
static Fl_Button* btn[BTN_MAX];

static char const* STR_ADD = "Add";
static char const* STR_REMOVE = "Remove";

int main(int argc, char* argv[])
{
	ASSERT((TIME_UPDATE > 1) && (TIME_UPDATE < 100));
	ASSERT((FONT >= 0) && (FONT < SSIZE_MAX));
	ASSERT((FONT_SZ > 0) && (FONT_SZ < SSIZE_MAX));
	ASSERT((strlen(SV_DIR) > 0) && (strlen(SV_DIR) < STR_SZ));
	ASSERT((strlen(SV_RUN_DIR) > 0) && (strlen(SV_RUN_DIR) < STR_SZ));
	ASSERT((strlen(SV) > 0) && (strlen(SV) < STR_SZ));
	ASSERT(strlen(ASK_SERVICES) > 0);

	MESSAGE_DBG("TITLE: %s", TITLE);
	MESSAGE_DBG("TIME_UPDATE: %d", TIME_UPDATE);
	MESSAGE_DBG("SV: %s", SV);
	MESSAGE_DBG("SV_DIR: %s", SV_DIR);
	MESSAGE_DBG("SV_RUN_DIR: %s", SV_RUN_DIR);

	if (argc == 2)
	{
		if ((strstr(argv[1], "--version") || strstr(argv[1], "-v")))
		{
			printf("%s\n", TITLE);
			exit(EXIT_SUCCESS);
		}
	}

	if (geteuid() != 0)
	{
		fl_alert("Administrator permissions are required");
		exit(EXIT_FAILURE);
	}

	fl_register_images();
	fl_message_title_default(TITLE);

	Fl_Double_Window* wnd = new Fl_Double_Window(600, 400);
	Fl_Group* grp = new Fl_Group(0, 0, wnd->w(), 30);
	btn[QUIT] = new Fl_Button(BTN_X, BTN_Y, BTN_W, BTN_H, "Quit");
	btn[RUN] = new Fl_Button(BTN_W + BTN_PAD, BTN_Y, BTN_W, BTN_H, "Run");
	btn[DOWN] = new Fl_Button(BTN_W * 2 + BTN_PAD, BTN_Y, BTN_W, BTN_H, "Down");
	btn[RESTART] = new Fl_Button(BTN_W * 3 + BTN_PAD, BTN_Y, BTN_W, BTN_H, "Restart");
	btn[ADD] = new Fl_Button(BTN_W * 4 + BTN_PAD, BTN_Y, BTN_W + 20, BTN_H, "Service...");

	SetButtonAlign(QUIT, ADD, 256);
	SetButtonFont(QUIT, ADD);

	btn[QUIT]->image(get_icon_quit());
	btn[RUN]->image(get_icon_run());
	btn[DOWN]->image(get_icon_down());
	btn[RESTART]->image(get_icon_restart());
	btn[ADD]->image(get_icon_add());

	btn[QUIT]->callback(QuitCb);
	btn[RUN]->callback(CommandCb);
	btn[DOWN]->callback(CommandCb);
	btn[RESTART]->callback(CommandCb);
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


void SetFont(Fl_Widget* w)
{
	ASSERT_DBG(w);
	w->labelfont(FONT);
	w->labelsize(FONT_SZ);
}


void SetFont(Fl_Hold_Browser* w)
{
	ASSERT_DBG(w);
	w->textfont(FONT);
	w->textsize(FONT_SZ);
}

void SetButtonFont(int const start, int const end)
{
	for (int i = start; i <= end; i++)
	{
		SetFont(btn[i]);
	}
}


void FillBrowserEnable(void)
{
	char buffer[STR_SZ];

	FILE* psv = (FILE*)NULL;

	SanitizeEnv();

	psv = popen(SV_LIST, "r");

	if (!psv)
	{
		fl_alert("Failed to open the pipe.\nCommand line: %s", SV_LIST);
		exit(EXIT_FAILURE);
	}

	browser[ENABLE]->clear();

	btn[DOWN]->deactivate();
	btn[RUN]->deactivate();
	btn[ADD]->deactivate();
	btn[RESTART]->deactivate();

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
				btn[ADD]->activate();
			}
			else if (pb[0] == 'r' || pb[0] == 'R')
			{
				btn[DOWN]->activate();
				btn[RESTART]->activate();
				btn[ADD]->activate();
			}
		}

		char* pc = strchr(pb, ':');

		ASSERT_DBG(pc != (char*)NULL);
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
			browser[ENABLE]->icon(browser[ENABLE]->size(), get_icon_warning());
		}
		else if (pb[0] == 'w' || pb[0] == 'W')
		{
			browser[ENABLE]->icon(browser[ENABLE]->size(), get_icon_warning());
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


void FillBrowserList(void)
{
	char buffer[STR_SZ];

	FILE* pls = (FILE*)NULL;
	FILE* plsRun = (FILE*)NULL;

	SanitizeEnv();

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

	SanitizeEnv();

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


void QuitCb(UNUSED Fl_Widget* w, UNUSED void* data)
{
	exit(EXIT_SUCCESS);
}


int GetSelected(Fl_Browser const* const brw)
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


void SelectCb(Fl_Widget* w, UNUSED void* data)
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


bool AskIfContinue(char const* const service)
{
	ASSERT_DBG(service);
	ASSERT_DBG(service[0] != '\0');

	char* str = strdup(ASK_SERVICES);

	ASSERT_DBG(str);

	char* tok = strtok(str, ASK_SERVICES_DELIM);

	ASSERT_DBG(tok);

	do
	{
		if (strstr(service, tok))
		{
			MESSAGE_DBG("ASK_SERVICES: %s", tok);

			if (0 == fl_choice("Warning: Protected service name detected: %s\n"
							"Do you continue?", "Cancel", "Continue", 0, service))
			{
				free(str);
				return false;
			}
		}
	} while ((tok = strtok(0, ASK_SERVICES_DELIM)));

	free(str);
	return true;
}


void CommandCb(Fl_Widget* w, UNUSED void* data)
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
		RunSv(service, "up");
	}
	else if (btnId == btn[RESTART])
	{
		if (AskIfContinue(service))
		{
			RunSv(service, "restart");
		}
	}
	else if (btnId == btn[DOWN])
	{
		if (AskIfContinue(service))
		{
			RunSv(service, "down");
		}
	}
	else
	{
		STOP_DBG("Button identifier not covered: %p", btnId);
	}

	free(service);
	service = (char*)NULL;
}


void RemoveNewLine(char* str)
{
	char* cp = strchr(str, '\n');

	if (cp)
	{
		*cp = '\0';
	}
}


void IntallUninstallCb(Fl_Widget* w, UNUSED void* data)
{
	ASSERT_DBG(w);

	char dest[STR_SZ];
	char* argv[5];

	Fl_Button* btnId = (Fl_Button*)w;

	int const item = GetSelected(browser[LIST]);
	char const* const itemText = browser[LIST]->text(item);

	if (btnId == btn[UNINSTALL])
	{
		if (not AskIfContinue(itemText))
		{
			return;
		}
	}

	strncpy(dest, SV_RUN_DIR, STR_SZ - 1);
	strncat(dest, "/", 1);
	strncat(dest, itemText, STR_SZ - strlen(dest) - 1);
	dest[STR_SZ - 1] = '\0';

	RemoveNewLine(dest);

	if (btnId == btn[INSTALL])
	{
		char src[STR_SZ];

		strncpy(src, SV_DIR, STR_SZ - 1);
		strncat(src, "/", 1);
		strncat(src,  itemText, STR_SZ - strlen(src) - 1);
		src[STR_SZ - 1] = '\0';

		RemoveNewLine(src);

		argv[0] = "ln";
		argv[1] = "-s";
		argv[2] = src;
		argv[3] = dest;
		argv[4] = (char*)NULL;
		System("ln", argv);
	}
	else if (btnId == btn[UNINSTALL])
	{
		argv[0] = "unlink";
		argv[1] = dest;
		argv[2] = (char*)NULL;
		System("unlink", argv);
	}
	else
	{
		STOP_DBG("Button identifier not covered: %p", btnId);
	}

	FillBrowserEnable();
	FillBrowserList();
}


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


void RunSv(char const* const service, char const* const action)
{
	char* argv[4];
	argv[0] = SV;
	argv[1] = (char*)action;
	argv[2] = (char*)service;
	argv[3] = (char*)NULL;

	System(SV, argv);
	FillBrowserEnable();
}


void TimerCb(UNUSED void* data)
{
	FillBrowserEnable();
	Fl::repeat_timeout(TIME_UPDATE, TimerCb);
}


void SetButtonAlign(int const start, int const end, int const align)
{
	for (int i = start; i <= end; ++i)
	{
		btn[i]->align(align);
	}
}


void ShowWindowModal(Fl_Double_Window* const wnd)
{
	ASSERT_DBG(wnd);

	wnd->set_modal();
	wnd->show();
	while (wnd->shown())
	{
		Fl::wait();
	}

}


void CloseWindowCb(UNUSED Fl_Widget* w, void* data)
{
	ASSERT_DBG(data);
	((Fl_Double_Window*)data)->hide();
}


void AddServicesCb(UNUSED Fl_Widget* w, void* data)
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
