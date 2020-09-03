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
#include "system.h"
#include "icons.h"

void FillBrowserEnable(void);
void FillBrowserList(void);
int GetSelected(Fl_Browser const* const brw);
void RunSv(char const* const service, char const* const action);
void ShowWindowModal(Fl_Double_Window* const wnd);
void SetButtonAlign(int const start, int const end, int const align);
void SetButtonFont(int const start, int const end);
void SetFont(Fl_Widget* w);
void SetFont(Fl_Hold_Browser* w);
void SetFont(Fl_Text_Editor* w);
void RemoveNewLine(std::string& str);
bool AskIfContinue(char const* const service);
void MakeServicePath(std::string const& service, std::string& path);
void MakeServiceRunDirPath(std::string const& service, std::string& path);
void ShowNotify(int const id, char const* const service);
char* ExtractServiceNameFromPath(char const* const service);
char* ExtractServiceNameFromSV(char const* const service);

void QuitCb(UNUSED Fl_Widget* w, UNUSED void* data);
void SelectCb(Fl_Widget* w, UNUSED void* data);
void CommandCb(Fl_Widget* w, UNUSED void* data);
void LoadUnloadCb(Fl_Widget* w, UNUSED void* data);
void AddServicesCb(UNUSED Fl_Widget* w, void* data);
void TimerCb(UNUSED void* data);
void EditNewCb(Fl_Widget* w, void* data);
void DeleteServiceCb(UNUSED Fl_Widget* w, void* data);
void EnabledDisabledServiceCb(Fl_Widget* w, void* data);

static int itemSelect[BROWSER_MAX] { [ENABLE] = SELECT_RESET, [LIST] = SELECT_RESET };

static Fl_Hold_Browser* browser[BROWSER_MAX];
static Fl_Button* btn[BTN_MAX];
static Fl_Text_Buffer* tbuf[TBUF_MAX];
static Fl_Text_Editor* tedt[TEDT_MAX];

static char const* STR_LOAD = "Load";
static char const* STR_UNLOAD = "Unload";
static char const* STR_EDIT = "Edit...";
static char const* STR_NEW = "New...";


static void Exit(void)
{
	NotifyEnd();
}

int main(int argc, char* argv[])
{
	ASSERT((TIME_UPDATE > 1) && (TIME_UPDATE < 100));
	ASSERT((FONT >= 0) && (FONT < SSIZE_MAX));
	ASSERT((FONT_SZ >= 8) && (FONT_SZ <= 14));
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

	atexit(Exit);

	Fl::add_timeout(TIME_UPDATE, TimerCb);

	return Fl::run();
}


void ShowNotify(int const id, char const* const service)
{
	ASSERT_DBG_STRING(service);
	char str[256];
	char* body = NULL;
	char* name = ExtractServiceNameFromSV(service);

	switch (id)
	{
		case NOTIFY_DOWN:
			body = NOTIFY_STR_DOWN;
			break;
		case NOTIFY_UP:
			body = NOTIFY_STR_UP;
			break;
		case NOTIFY_RESTART:
			body = NOTIFY_STR_RESTART;
			break;
		case NOTIFY_DELETE:
			body = NOTIFY_STR_DELETE;
			break;
		default:
			STOP_DBG("Notify identifier not covered: %d", id);
	}

	snprintf(str, 255, body, name);
	str[255] = '\0';
	NotifyShow(NOTIFY_STR_SUMMARY, str);
	free(name);
}


void SetFont(Fl_Widget* w)
{
	ASSERT_DBG(w);
	w->labelfont(FONT);
	w->labelsize(FONT_SZ);
}


void SetFont(Fl_Text_Editor* w)
{
	ASSERT_DBG(w);
	w->textfont(FL_COURIER);
	w->textsize(FONT_SZ);
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

	FILE* pipe = PipeOpen(SV_LIST);

	browser[ENABLE]->clear();

	btn[DOWN]->deactivate();
	btn[RUN]->deactivate();
	btn[ADD]->deactivate();
	btn[RESTART]->deactivate();

	int iselect_count = SELECT_RESET;

	while (fgets(buffer, STR_SZ, pipe))
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

	PipeClose(pipe);

	if (browser[ENABLE]->size() == 0)
	{
		fl_alert("No runit service found: %s", SV_LIST);
		exit(EXIT_FAILURE);
	}

	browser[ENABLE]->select(itemSelect[ENABLE]);
}


static void FillBrowserList_All(char const* path)
{
	browser[LIST]->add(path, (void*)STR_UNLOAD);
	browser[LIST]->icon(browser[LIST]->size(), get_icon_disable());
}


static void FillBrowserList_Load(char const* path)
{
	for (int item = 1; item <= browser[LIST]->size(); ++item)
	{
		char const* const find = browser[LIST]->text(item);

		if (strstr(path, find))
		{
			browser[LIST]->icon(item, get_icon_enable());
			browser[LIST]->data(item, (void*)STR_LOAD);
			break;
		}
	}
}


void FillBrowserList(void)
{
	int iselect_count = SELECT_RESET;

	btn[LOAD]->deactivate();
	btn[UNLOAD]->deactivate();

	browser[LIST]->clear();

	ListDirectories(SV_DIR, FillBrowserList_All);

	ListDirectories(SV_RUN_DIR, FillBrowserList_Load);

	for (int item = 1; item <= browser[LIST]->size(); ++item)
	{
		if (iselect_count++ == itemSelect[LIST])
		{
			char* data = (char*)browser[LIST]->data(item);

			if (data == STR_LOAD)
			{
				btn[UNLOAD]->activate();
			}
			else if (data == STR_UNLOAD)
			{
				btn[LOAD]->activate();
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


char* ExtractServiceNameFromSV(char const* const service)
{
	ASSERT_DBG_STRING(service);

	char* name = strdup(service);
	ASSERT_DBG(name);

	char* b = strchr(name, '/');

	/* algunos nombre de servicios no tienen ruta absoluta */
	if (b == NULL)
	{
		return name;
	}

	char* e = strchr(b, ':');
	ASSERT_DBG(e);

	std::ptrdiff_t const len = e - b;

	memmove(name, b, len);

	name[len] = '\0';

	return name;
}


char* ExtractServiceNameFromPath(char const* const service)
{
	char* servicePath = ExtractServiceNameFromSV(service);
	char* name = strdup(basename(servicePath));
	free(servicePath);
	return name;
}


void CommandCb(Fl_Widget* w, UNUSED void* data)
{
	Fl_Button* btnId = (Fl_Button*)w;

	char const* itemText = browser[ENABLE]->text(itemSelect[ENABLE]);

	char* service = ExtractServiceNameFromPath(itemText);

	if (btnId == btn[RUN])
	{
		RunSv(service, "up");
		ShowNotify(NOTIFY_UP, service);
	}
	else if (btnId == btn[RESTART])
	{
		if (AskIfContinue(service))
		{
			RunSv(service, "restart");
			ShowNotify(NOTIFY_RESTART, service);
		}
	}
	else if (btnId == btn[DOWN])
	{
		if (AskIfContinue(service))
		{
			RunSv(service, "down");
			ShowNotify(NOTIFY_DOWN, service);
		}
	}
	else
	{
		STOP_DBG("Button identifier not covered: %p", btnId);
	}

	free(service);
}


void RemoveNewLine(std::string& str)
{
	size_t const pos = str.find('\n');

	if (pos != std::string::npos)
	{
		str.erase(pos);
	}
}


void LoadUnloadCb(Fl_Widget* w, UNUSED void* data)
{
	ASSERT_DBG(w);

	Fl_Button* btnId = (Fl_Button*)w;

	int const item = GetSelected(browser[LIST]);
	char const* const service = browser[LIST]->text(item);

	if (btnId == btn[UNLOAD])
	{
		if (not AskIfContinue(service))
		{
			return;
		}
	}

	std::string dest;
	MakeServiceRunDirPath(service, dest);
	RemoveNewLine(dest);

	if (btnId == btn[LOAD])
	{
		std::string src;
		MakeServicePath(service, src);
		RemoveNewLine(src);
		Link(src.c_str(), dest.c_str());
		ShowNotify(NOTIFY_UP, service);
	}
	else if (btnId == btn[UNLOAD])
	{
		Unlink(dest.c_str());
		ShowNotify(NOTIFY_DOWN, service);
	}
	else
	{
		STOP_DBG("Button identifier not covered: %p", btnId);
	}

	FillBrowserEnable();
	FillBrowserList();
}


void RunSv(char const* const service, char const* const action)
{
	ASSERT_DBG_STRING(service);
	ASSERT_DBG_STRING(action);

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
							430,
							380,
							TITLE_SERVICE);

	btn[CLOSE] = new Fl_Button(BTN_X, BTN_Y, BTN_W, BTN_H, "Close");
	btn[LOAD] = new Fl_Button(BTN_W + BTN_PAD, BTN_Y, BTN_W, BTN_H, STR_LOAD);
	btn[UNLOAD] = new Fl_Button(BTN_W * 2 + BTN_PAD, BTN_Y, BTN_W, BTN_H, STR_UNLOAD);
	btn[EDIT] = new Fl_Button(BTN_W * 3 + BTN_PAD, BTN_Y, BTN_W, BTN_H, STR_EDIT);
	btn[NEW] = new Fl_Button(BTN_W * 4 + BTN_PAD, BTN_Y, BTN_W, BTN_H, STR_NEW);
	browser[LIST] = new Fl_Hold_Browser(4, 40, wnd->w() - 8, wnd->h() - 48);

	btn[CLOSE]->image(get_icon_quit());
	btn[LOAD]->image(get_icon_add());
	btn[UNLOAD]->image(get_icon_remove());
	btn[EDIT]->image(get_icon_edit());
	btn[NEW]->image(get_icon_new());

	browser[LIST]->callback(SelectCb);
	btn[CLOSE]->callback(CloseWindowCb, (void*)wnd);
	btn[LOAD]->callback(LoadUnloadCb);
	btn[UNLOAD]->callback(LoadUnloadCb);
	btn[EDIT]->callback(EditNewCb, (void*)wnd);
	btn[NEW]->callback(EditNewCb, (void*)wnd);

	SetFont(browser[LIST]);
	SetButtonFont(CLOSE, NEW);
	SetButtonAlign(CLOSE, NEW, 256);

	FillBrowserList();

	ShowWindowModal(wnd);

	for (int i = CLOSE; i <= NEW; i++)
	{
		delete btn[i];
	}

	delete browser[LIST];
	delete wnd;
}


void MakeServiceRunDirPath(std::string const& service, std::string& path)
{
	path = SV_RUN_DIR;
	path += "/";
	path += service;
}


void MakeServicePath(std::string const& service, std::string& path)
{
	path = SV_DIR;
	path +=  "/";
	path += service;
}


static void MakeLogDirPath(std::string const& service, std::string& path)
{
	MakeServicePath(service, path);
	path += "/log/";
}


static void MakeServiceDirPath(std::string const& service, std::string& path)
{
	MakeServicePath(service, path);
	path += "/";
}


static void MakeServiceRunPath(std::string const& service, std::string& path)
{
	MakeServiceDirPath(service, path);
	path += "run";
}


static void MakeFinishPath(std::string const& service, std::string& path)
{
	MakeServiceDirPath(service, path);
	path += "finish";
}


static void MakeDownPath(std::string const& service, std::string& path)
{
	MakeServiceDirPath(service, path);
	path += "down";
}


static void MakeLogRunPath(std::string const& service, std::string& path)
{
	MakeLogDirPath(service, path);
	path += "run";
}


static unsigned long CalculateHash(Fl_Text_Buffer const* const txtBuff)
{
	char* text = txtBuff->text();

	if (text == (char*)NULL)
	{
		return 0UL;
	}

	if (text[0] == '\0')
	{
		free(text);
		return 0UL;
	}

	unsigned long hash = Hash(text);
	free(text);
	return hash;
}


static bool IfNotEqualHash(Fl_Text_Buffer const* const txtBuff, unsigned long const hash)
{
	unsigned long const txtBuffHash = CalculateHash(txtBuff);
	return (txtBuffHash != hash);
}


static void EditLoad(struct NewEditData* saveNewEditData)
{
	bool const showError = true;
	int const item = GetSelected(browser[LIST]);
	std::string service = browser[LIST]->text(item);

	RemoveNewLine(service);

	std::string path;

	MakeServiceRunPath(service, path);

	if (!FileAccessOk(path.c_str(), not showError))
	{
		fl_alert("The service '%s' exists but the 'run' file was not found.", service.c_str());
	}

	tbuf[TBUF_SERV]->loadfile(path.c_str());
	saveNewEditData->hash[TBUF_SERV] = CalculateHash(tbuf[TBUF_SERV]);
	saveNewEditData->box[BOX_TIME_SERV]->copy_label(GetModifyFileTime(path.c_str()));

	MakeLogRunPath(service, path);
	tbuf[TBUF_LOG]->loadfile(path.c_str());
	saveNewEditData->hash[TBUF_LOG] = CalculateHash(tbuf[TBUF_LOG]);
	saveNewEditData->box[BOX_TIME_LOG]->copy_label(GetModifyFileTime(path.c_str()));

	MakeFinishPath(service, path);
	tbuf[TBUF_FINISH]->loadfile(path.c_str());
	saveNewEditData->hash[TBUF_FINISH] = CalculateHash(tbuf[TBUF_FINISH]);
	saveNewEditData->box[BOX_TIME_FINISH]->copy_label(GetModifyFileTime(path.c_str()));
}


static void NewEditSaveCb(UNUSED Fl_Widget* w, void* data)
{
	struct NewEditData* saveNewEditData = (struct NewEditData*)data;

	char const* const service = saveNewEditData->service;

	if (strlen(service) == 0 || strchr(service, ' '))
	{
		fl_alert("The name of the Service is not indicated.");
		return;
	}

	if (tbuf[TBUF_SERV]->length() == 0)
	{
		fl_alert("Nothing to save.");
		return;
	}

	int const id = saveNewEditData->id;
	bool showError = true;
	std::string path;
	std::string dir;

	switch (id)
	{
	case NEW:
		MakeServiceDirPath(service, dir);

		if (DirAccessOk(dir.c_str(), not showError))
		{
			fl_alert("The service '%s' already exists.", service);
			return;
		}

		MakeDir(dir.c_str(), showError);

		/* Fallthrough */

	case EDIT:

		MakeServiceRunPath(service, path);

		if (IfNotEqualHash(tbuf[TBUF_SERV], saveNewEditData->hash[TBUF_SERV]))
		{
			MESSAGE_DBG("Save run, service: %s", service);	
			tbuf[TBUF_SERV]->savefile(path.c_str());
			FileToExecutableMode(path.c_str());
		}

		if (tbuf[TBUF_LOG]->length() > 0 && IfNotEqualHash(tbuf[TBUF_LOG], saveNewEditData->hash[TBUF_LOG]))
		{
			MakeLogRunPath(service, path);
			MakeLogDirPath(service, dir);

			if (!DirAccessOk(dir.c_str(), not showError))
			{
				MakeDir(dir.c_str(), showError);
			}

			MESSAGE_DBG("Save log, service: %s", service);	
			tbuf[TBUF_LOG]->savefile(path.c_str());
			FileToExecutableMode(path.c_str());
		}

		if (tbuf[TBUF_FINISH]->length() > 0 && IfNotEqualHash(tbuf[TBUF_FINISH], saveNewEditData->hash[TBUF_FINISH]))
		{
			MESSAGE_DBG("Save finish, service: %s", service);	
			MakeFinishPath(service, path);
			tbuf[TBUF_FINISH]->savefile(path.c_str());
			FileToExecutableMode(path.c_str());
		}
	}

	((Fl_Double_Window*)saveNewEditData->data)->hide();
}


static void ChangeStateEnabledDisabledButtons(std::string const& service)
{
	std::string pathDown;

	bool showError = true;

	MakeDownPath(service, pathDown);

	if (FileAccessOk(pathDown.c_str(), not showError))
	{
		btn[DISABLED]->deactivate();
		btn[ENABLED]->activate();
	}
	else
	{
		btn[DISABLED]->activate();
		btn[ENABLED]->deactivate();
	}
}


void EditNewCb(Fl_Widget* w, void* data)
{
	ASSERT_DBG(data);

	Fl_Double_Window* wndParent = (Fl_Double_Window*)data;

	Fl_Button* btnId = (Fl_Button*)w;

	int id = NEW;

	char const* title = TITLE_SERVICE_NEW;

	if (btnId == btn[EDIT])
	{
		title = TITLE_SERVICE_EDIT;
		id = EDIT;
	}

	int const item = GetSelected(browser[LIST]);
	std::string service = browser[LIST]->text(item);
	RemoveNewLine(service);

	Fl_Double_Window* wnd = new Fl_Double_Window(wndParent->x() + 300 / 2,
							wndParent->y(),
							500,
							365,
							title);

	Fl_Input *input = new Fl_Input(130, 15, 180, 25);
	input->label("Service name:");
	input->textfont(FONT);
	input->textsize(FONT_SZ);
	SetFont(input);

	struct NewEditData saveNewEditData;
	saveNewEditData.id = id;
	saveNewEditData.data = (void*)wnd;

	if (id == EDIT)
	{
		input->type(FL_INPUT_READONLY);
		input->value(service.c_str());
		saveNewEditData.service = input->value();
	}
	else /* NEW */
	{
		input->value(" "); // force asign _value
		saveNewEditData.service = input->value();
		input->value(""); // clean
	}

	tbuf[TBUF_SERV] = new Fl_Text_Buffer();
	tbuf[TBUF_LOG] = new Fl_Text_Buffer();
	tbuf[TBUF_FINISH] = new Fl_Text_Buffer();

	saveNewEditData.hash[TBUF_SERV] = 0;
	saveNewEditData.hash[TBUF_LOG] = 0;
	saveNewEditData.hash[TBUF_FINISH] = 0;

	Fl_Tabs* tabs = new Fl_Tabs(15, 50, 475, 265);

		Fl_Group* grp0 = new Fl_Group(15, 75, 475, 300, "Service");
			tedt[TEDT_SERV] = new Fl_Text_Editor(20, 80, 460, 230);
			tedt[TEDT_SERV]->box(FL_FLAT_BOX);
			tedt[TEDT_SERV]->buffer(tbuf[TBUF_SERV]);
			Fl_Box* boxTimeServ = new Fl_Box(15, 500 - 80 - 90, 460, 10, NULL);
			boxTimeServ->align(Fl_Align(133 | FL_ALIGN_INSIDE));
		grp0->end();

		Fl_Group* grp1 = new Fl_Group(15, 75, 475, 300, "Log (optional)");
			grp1->hide();
			tedt[TEDT_LOG] = new Fl_Text_Editor(20, 80, 460, 230);
			tedt[TEDT_LOG]->box(FL_FLAT_BOX);
			tedt[TEDT_LOG]->buffer(tbuf[TBUF_LOG]);
			Fl_Box* boxTimeLog = new Fl_Box(15, 500 - 80 - 90, 460, 10, NULL);
			boxTimeLog->align(Fl_Align(133 | FL_ALIGN_INSIDE));
		grp1->end();

		Fl_Group* grp2 = new Fl_Group(15, 75, 475, 300, "Finish (optional)");
			grp2->hide();
			tedt[TEDT_FINISH] = new Fl_Text_Editor(20, 80, 460, 230);
			tedt[TEDT_FINISH]->box(FL_FLAT_BOX);
			tedt[TEDT_FINISH]->buffer(tbuf[TBUF_FINISH]);
			Fl_Box* boxTimeFinish = new Fl_Box(15, 500 - 80 - 90, 460, 10, NULL);
			boxTimeFinish->align(Fl_Align(133 | FL_ALIGN_INSIDE));
		grp2->end();

		Fl_Group* grp3 = new Fl_Group(15, 75, 475, 300, "Extras...");
			grp3->hide();
			btn[DELETE] = new Fl_Button(25, 90, BTN_W, BTN_H, "Delete...");
			btn[DISABLED] = new Fl_Button(25, 90 + 60, BTN_W, BTN_H, "Disable...");
			btn[ENABLED] = new Fl_Button(25, 90 + 60 * 2, BTN_W, BTN_H, "Enable...");
			Fl_Box* box0 = new Fl_Box(30 + BTN_W, 92, 475 - (30 + BTN_W), 60, "Use it to permanently remove the service. It cannot be undone.");
			Fl_Box* box1 = new Fl_Box(30 + BTN_W, 92 + 60, 475 - (30 + BTN_W), 60,"Use it to disable the service permanently, including when rebooting the system.");
			Fl_Box* box2 = new Fl_Box(30 + BTN_W, 92 + 60 * 2, 475 - (30 + BTN_W), 60, "Use it to re-enable the service that is currently disabled.");
			box0->align(Fl_Align(133 | FL_ALIGN_INSIDE));
			box1->align(Fl_Align(133 | FL_ALIGN_INSIDE));
			box2->align(Fl_Align(133 | FL_ALIGN_INSIDE));
		grp3->end();
	tabs->end();


	SetFont(boxTimeLog);
	SetFont(boxTimeFinish);
	SetFont(boxTimeServ);
	SetFont(grp0);
	SetFont(grp1);
	SetFont(grp2);
	SetFont(grp3);
	SetFont(box0);
	SetFont(box1);
	SetFont(box2);
	SetFont(tedt[TEDT_SERV]);
	SetFont(tedt[TEDT_LOG]);
	SetFont(tedt[TEDT_FINISH]);

	btn[SAVE] = new Fl_Button(310, 355 - BTN_H, BTN_W, BTN_H, "Save files");
	btn[CANCEL] = new Fl_Button(310 + BTN_W + 2, 355 - BTN_H, BTN_W, BTN_H, "Close");

	btn[SAVE]->callback(NewEditSaveCb, (void*)&saveNewEditData);
	btn[CANCEL]->callback(CloseWindowCb, (void*)wnd);

	btn[DELETE]->image(get_icon_warning());
	btn[SAVE]->image(get_icon_save());
	btn[CANCEL]->image(get_icon_quit());
	btn[DISABLED]->image(get_icon_down());
	btn[ENABLED]->image(get_icon_run());

	SetButtonFont(SAVE, ENABLED);
	SetButtonAlign(SAVE, ENABLED, 256);

	if (id == EDIT)
	{
		ChangeStateEnabledDisabledButtons(service);

		btn[DELETE]->callback(DeleteServiceCb, (void*)&saveNewEditData);
		btn[ENABLED]->callback(EnabledDisabledServiceCb, (void*)&saveNewEditData);
		btn[DISABLED]->callback(EnabledDisabledServiceCb, (void*)&saveNewEditData);

		saveNewEditData.box[BOX_TIME_SERV] = boxTimeServ;
		saveNewEditData.box[BOX_TIME_LOG] = boxTimeLog;
		saveNewEditData.box[BOX_TIME_FINISH] = boxTimeFinish;
		EditLoad(&saveNewEditData);
	}
	else /* NEW */
	{
		grp3->deactivate();
	}

	wnd->end();

	ShowWindowModal(wnd);

	delete tbuf[TBUF_SERV];
	delete tbuf[TBUF_LOG];
	delete tbuf[TBUF_FINISH];
	delete tedt[TEDT_SERV];
	delete tedt[TEDT_LOG];
	delete tedt[TEDT_FINISH];
	delete btn[DELETE];
	delete btn[SAVE];
	delete btn[CANCEL];
	delete btn[DISABLED];
	delete btn[ENABLED];
	delete box0;
	delete box1;
	delete box2;
	delete boxTimeServ;
	delete boxTimeLog;
	delete boxTimeFinish;
	delete grp0;
	delete grp1;
	delete grp2;
	delete grp3;
	delete tabs;
	delete input;
	delete wnd;

	FillBrowserList();
}


void DeleteServiceCb(UNUSED Fl_Widget* w, void* data)
{
	ASSERT_DBG(data);

	struct NewEditData* saveNewEditData = (struct NewEditData*)data;
	char const* const service = saveNewEditData->service;

	std::string path;
	bool showError = true;

	MakeServiceDirPath(service, path);

	int const ret = fl_choice("Alert: This action cannot be undone.\nThe directory '%s' will be removed.\nAre you sure to continue?",
							 "No", "Yes, delete", NULL, path.c_str());

	if (ret == 0)
	{
		return;
	}

	MakeServiceRunDirPath(service, path);

	if (FileAccessOk(path.c_str(), not showError))
	{
		Unlink(path.c_str());
	}

	MakeServiceDirPath(service, path);

	if (DirAccessOk(path.c_str(), showError))
	{
		RemoveRecursive(path.c_str());
	}

	ShowNotify(NOTIFY_DELETE, service);

	((Fl_Double_Window*)saveNewEditData->data)->hide();
}


void EnabledDisabledServiceCb(Fl_Widget* w, void* data)
{
	ASSERT_DBG(w);
	ASSERT_DBG(data);

	Fl_Button const* const btnId = (Fl_Button*)w;

	struct NewEditData* saveNewEditData = (struct NewEditData*)data;
	char const* const service = saveNewEditData->service;
	char const* msg = "Are you sure to disable the service?";
	char const* msg_yes = "Yes, disable";

	if (btnId == btn[ENABLED])
	{
		msg = "Are you sure to re-enable the service?";
		msg_yes = "Yes, re-enable";
	}

	int const ret = fl_choice(msg, "No", msg_yes, NULL);

	if (ret == 0)
	{
		return;
	}

	std::string fileDown;

	MakeDownPath(service, fileDown);

	if (btnId == btn[DISABLED])
	{
		bool showError = true;
		MakeFile(fileDown.c_str(), showError);
	}
	else
	{
		Unlink(fileDown.c_str());
	}

	ChangeStateEnabledDisabledButtons(service);
}
