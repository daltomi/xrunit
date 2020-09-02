# xsv

Graphical interface for `sv (control and manage services monitored by runsv)`


<img src="https://git.disroot.org/daltomi/xsv/raw/branch/master/screenshot_00.png"/>

<img src="https://git.disroot.org/daltomi/xsv/raw/branch/master/screenshot_01.png"/>


### Support

I use ArtixLinux, and the project is designed for it.

If you use another distribution you can experiment with the
preprocessor directives and always compile in debug mode.



### Dependencies

* C++ standard: >= c++11

* Libraries : **fltk** (>= 1.3.4rc1), **libnotify**

* Build:  **g++**, **make**, **fltk-config**

### Build
```bash
make debug
-- or --
make release
```

* This program needs to be run with administrator permissions.

### Preprocessor directives

| Directive | Description | Default | Type |
|-------------------------------|---------|---------|---------
| SV |  sv binary | /usr/bin/sv | string
| SV_DIR      |  available services directory | /etc/runit/sv | string
| SV_RUN_DIR      |  services directory | /run/runit/service | string
| TIME_UPDATE | seconds of updating the list of service | 5 | integer
| FONT        | FLTK font name  | FL_HELVETICA | integer
| FONT_SZ     | font size | 11 (range 8..14)| integer
| ASK_SERVICES | ask about these services before down/remove | tty,dbus,udev,elogind | string

Example

File `my_define.h`

```
#define SV "/usr/local/bin/sv"
#define SV_DIR "/etc/local/runit/sv"
```

Then run make in your terminal:

```
CXXFLAGS="-include my_define.h" make debug
```

### Makefile targets:

| Target | Description |
|--------|--------------|
| clean  |  Clean the project directory |
| debug  | Build the executable with debug symbols |
| release | Build the executable for performance |
| dist   | Create a compressed file with the project files |

