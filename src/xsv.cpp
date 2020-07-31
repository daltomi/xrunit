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
#include <FL/Fl_Window.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/fl_ask.H>

#include <unistd.h>
#include <stdio.h>

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
#define SV_RUN_DIR " /run/runit/service"
#endif

#define SV_STATUS " status "
#define SV_UP " up "
#define SV_DOWN " down "
#define SV_LIST SV SV_STATUS SV_RUN_DIR"/*"
#define UNUSED __attribute__((unused))
#define SELECT_RESET 1
#define N_BUFFER     200
#define BTN_W 80
#define BTN_H 25
#define BTN_X 10
#define BTN_Y 10
#define BTN_PAD 15

enum {
	QUIT,
	RUN,
	DOWN,
	ADD,
	CLOSE,
	INSTALL,
	UNINSTALL,
	BTN_MAX
};

static void FillBrowser(void);
static void GetSelected(void);
static void RunSv(char const* const service, char const* const action);
static void ShowWindowModal(Fl_Window* const wnd);
static void SetButtonAlign(int const start, int const end, int const align);

static void QuitCb(UNUSED Fl_Widget* w, UNUSED void* data);
static void SelectCb(UNUSED Fl_Widget* w, UNUSED void* data);
static void ActionCb(Fl_Widget* w, UNUSED void* data);
static void AddCb(UNUSED Fl_Widget* w, UNUSED void* data);
static void TimerCb(UNUSED void* data);

static int iselect = SELECT_RESET;
static Fl_Hold_Browser* browser  = 0;

Fl_Button* btn[BTN_MAX];

int main(void)
{
	MESSAGE_DBG("TITLE: %s", TITLE);
	MESSAGE_DBG("TIME_UPDATE: %d", TIME_UPDATE);
	MESSAGE_DBG("SV: %s", SV);
	MESSAGE_DBG("SV_RUN_DIR: %s", SV_RUN_DIR);

	if (geteuid() != 0)
	{
		fl_alert("Administrator permissions are required");
		exit(EXIT_FAILURE);
	}

	fl_register_images();

	Fl_Window* window = new Fl_Window(600, 400);

	Fl_Group* group = new Fl_Group(0, 0, window->w(), 30);
	btn[QUIT] = new Fl_Button(BTN_X, BTN_Y, BTN_W, BTN_H, "Quit");
	btn[RUN] = new Fl_Button(BTN_W + BTN_PAD, BTN_Y, BTN_W, BTN_H, "Run");
	btn[DOWN] = new Fl_Button(BTN_W * 2 + BTN_PAD, BTN_Y, BTN_W, BTN_H, "Down");
	btn[ADD] = new Fl_Button(BTN_W * 3 + BTN_PAD, BTN_Y, BTN_W + 20, BTN_H, "Service...");

	SetButtonAlign(QUIT, ADD, 256);

	btn[QUIT]->image(get_icon_quit());
	btn[RUN]->image(get_icon_run());
	btn[DOWN]->image(get_icon_down());
	btn[ADD]->image(get_icon_add());

	btn[QUIT]->callback(QuitCb);
	btn[RUN]->callback(ActionCb);
	btn[DOWN]->callback(ActionCb);
	btn[ADD]->callback(AddCb,(void*)window);

	{
		Fl_Box *o = new Fl_Box(BTN_W * 6 + BTN_PAD, 0, 10, 10);
		o->box(FL_FLAT_BOX);
		o->hide();
		group->resizable(o);
	}

	group->end();

	browser = new Fl_Hold_Browser(4, 40, window->w() - 8, window->h() - 48);

	int const browserWidths[] = {
		100, 150, 0
	};

	browser->textfont(FL_COURIER);
	browser->column_widths(browserWidths);
	browser->column_char('\t');
	browser->type(FL_MULTI_BROWSER);
	browser->callback(SelectCb);

	FillBrowser();

	window->label(TITLE);
	window->resizable(browser);
	window->end();
	window->show();

	Fl::add_timeout(TIME_UPDATE, TimerCb);

	return Fl::run();
}


static void FillBrowser(void)
{

	char buffer[N_BUFFER];

	FILE* psv = 0;

	psv = popen(SV_LIST, "r");

	if (!psv)
	{
		fl_alert("Failed to open the pipe.\nCommand line: %s\n", SV_LIST);
		exit(EXIT_FAILURE);
	}

	browser->clear();

	btn[DOWN]->deactivate();
	btn[RUN]->deactivate();

	int iselect_count = SELECT_RESET;

	while (fgets(buffer, N_BUFFER, psv))
	{
		buffer[N_BUFFER - 1] = '\0';

		char* pb = buffer;

		if (iselect_count++ == iselect) {

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

		browser->add(pb);

		if (pb[0] == 'd' || pb[0] == 'D')
		{
			browser->icon(browser->size(), get_icon_down());
		}
		else if (pb[0] == 'r' || pb[0] == 'R')
		{
			browser->icon(browser->size(), get_icon_run());
		}
		else if (pb[0] == 'f' || pb[0] == 'F')
		{
			browser->icon(browser->size(), get_icon_finish());
		}
		else
		{
			STOP_DBG("State not contemplated: %s", pb);
		}
	}

	browser->select(iselect);
	pclose(psv);
}


static void QuitCb(UNUSED Fl_Widget* w, UNUSED void* data)
{
	exit(EXIT_SUCCESS);
}


static void GetSelected(void)
{
	for (int i = 1; i <= browser->size(); i++)
	{
		if (browser->selected(i))
		{
			iselect = i;
			break;
		}
	}
}


static void SelectCb(UNUSED Fl_Widget* w, UNUSED void* data)
{
	GetSelected();
	FillBrowser();
}


static void ActionCb(UNUSED Fl_Widget* w, UNUSED void* data)
{
	Fl_Button* btnId = (Fl_Button*)w;

	char const* buffer = browser->text(iselect);

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
		RunSv(service, SV_DOWN);
	}
	else
	{
		STOP_DBG("Button identifier not covered: %p", btnId);
	}

	free(service);
	service = 0;
}


static void RunSv(char const* const service, char const* const action)
{
	char cmd [N_BUFFER];

	cmd[N_BUFFER - 1] = '\0';

	strncpy(cmd, SV, N_BUFFER - 1);
	strncat(cmd, action, N_BUFFER - 1);
	strncat(cmd, service, N_BUFFER - 1);

	MESSAGE_DBG("exec cmd: %s", cmd);

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

	FillBrowser();
}


static void TimerCb(UNUSED void* data)
{
	FillBrowser();
	Fl::repeat_timeout(TIME_UPDATE, TimerCb);
}


static void SetButtonAlign(int const start, int const end, int const align)
{
	for (int i = start; i <= end; ++i)
	{
		btn[i]->align(align);
	}
}


static void ShowWindowModal(Fl_Window* const wnd)
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
	((Fl_Window*)data)->hide();
}


static void AddCb(UNUSED Fl_Widget* w, UNUSED void* data)
{
	ASSERT_DBG(data);

	Fl_Window* wndParent = (Fl_Window*)data;

	Fl_Window* wnd = new Fl_Window(wndParent->x() + 300 / 2,
							wndParent->y(),
							300,
							350,
							TITLE_SERVICE);


	btn[CLOSE] = new Fl_Button(BTN_X, BTN_Y, BTN_W, BTN_H, "Close");
	btn[INSTALL] = new Fl_Button(BTN_W + BTN_PAD, BTN_Y, BTN_W, BTN_H, "Install");
	btn[UNINSTALL] = new Fl_Button(BTN_W * 2 + BTN_PAD, BTN_Y, BTN_W, BTN_H, "Uninstall");

	SetButtonAlign(CLOSE, UNINSTALL, 256);

	btn[CLOSE]->image(get_icon_quit());
	btn[INSTALL]->image(get_icon_run());
	btn[UNINSTALL]->image(get_icon_down());

	btn[CLOSE]->callback(CloseWindowCb, (void*)wnd);

	ShowWindowModal(wnd);

	for (int i = CLOSE; i <= UNINSTALL; i++)
	{
		delete btn[i];
	}

	delete wnd;
}
