# xsv

Graphical interface for sv - runit


<img src="https://git.disroot.org/daltomi/xsv/raw/branch/master/screenshot_00.png"/>

<img src="https://git.disroot.org/daltomi/xsv/raw/branch/master/screenshot_01.png"/>


### Support

I use Artix Linux, and the project is designed for it.

If you use another distribution you can experiment with the
preprocessor directives, always compile in debug mode.

### Releases

This is a *rolling release project*, therefore, there will always be only **one release**, the last one, the previous one is eliminated.
___

### Build

#### Dependencies

* C++ standard: >= c++11

* Libraries : **fltk** (>= 1.3.4rc1), **libnotify** (*optional)

* Build:  **g++**, **make**, **fltk-config**, **install**(coreutils)

#### Make

```bash
make debug
-- or --
make release

----------------

make install (default PREFIX=/usr)
-- or --
PREFIX=/usr/local make install
```
##### Makefile targets:

| Target | Description |
|--------|--------------|
| clean  |  Clean the project directory |
| debug  | Build the executable with debug symbols |
| release | Build the executable for performance |
| install | Copy the executable to $PREFIX/bin |
| dist   | Create a compressed file with the project files |


(*) `libnotify`: Optional compilation option. By default it is no. Use `LIB_NOTIFY=1 make` to activate.

___

### Notes:

* This program needs to be run with administrator permissions.

___

### Preprocessor directives

*(only if you don't use Artix Linux)*

* sv program

| Directive | Description | Default | Type |
|-------------------------------|---------|---------|---------
| SV |  sv binary | /usr/bin/sv | string


* Directories:

| Directive | Description | Default | Type |
|-------------------------------|---------|---------|---------
| SV_DIR      |  available services directory | /etc/runit/sv | string
| SV_RUN_DIR      |  services directory | /run/runit/service | string
| SYS_LOG_DIR | system log directory | /var/log | string


* Properties:

| Directive | Description | Default | Type |
|-------------------------------|---------|---------|---------
| TIME_UPDATE | seconds of updating the list of service | 5 | integer
| FONT        | FLTK font name  | FL_HELVETICA | integer
| FONT_SZ     | font size | 11 (range 8..14)| integer
| ASK_SERVICES | ask about these services before down/remove | tty,dbus,udev,elogind | string


Example:

File `my_define.h`

```
#define SV "/usr/local/bin/sv"
#define SV_DIR "/etc/local/runit/sv"
```

Then run make in your terminal:

```
CXXFLAGS="-include my_define.h" make debug
```
