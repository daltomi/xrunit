# xrunit

Graphical interface for sv - runit

[![Release](https://img.shields.io/github/v/release/daltomi/xrunit)](https://github.com/daltomi/xrunit/releases/latest)

<img src="https://github.com/daltomi/xrunit/raw/master/preview00.png"/>


### Support

Each distribution has its own `runit` configuration.
You can experiment with the preprocessor directives to adapt it to yours (always compile in debug mode).
Or look in directory `include` if you find a definition file for your distro.
At the end of the **Build** section you will find information on how to compile using this file.

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

_Note: The default values are based on ArtixLinux._


* sv program

| Directive | Description | Default | Type |
|-------------------------------|---------|---------|---------
| SV |  sv binary | /usr/bin/sv | string



* Directories:

| Directive | Description | Default | Type | ENV= |
|-------------------------------|---------|---------|---------|---------
| SV_DIR      |  available services directory | /etc/runit/sv | string | SVDIR
| SV_RUN_DIR  |  services directory | /run/runit/service | string  | -
| SYS_LOG_DIR | system log directory | /var/log | string | -



* Properties:

| Directive | Description | Default | Type |
|-------------------------------|---------|---------|---------
| TIME_UPDATE | seconds of updating the list of service | 5 | integer
| FONT        | FLTK font name  | FL_HELVETICA | integer
| FONT_SZ     | font size | 11 (range 8..14)| integer
| ASK_SERVICES | ask about these services before down/remove | tty,dbus,udev,elogind | string



#### Examples:

- New file `include/new-host.h`:

```c
#pragma once

#include "hosts_os.h"

#define HOST_OS "<name>" <see include/hosts_os.h>

// overrides the default
#define SV "/usr/local/bin/sv"
#define SV_DIR "/etc/local/runit/sv"
#define FONT_SZ 12
```

Then run make in your terminal:

```bash
CXXFLAGS="-include include/new-host.h" make debug
```


- Another example:

Using an already defined one: `include/voidlinux.h`

```bash
CXXFLAGS="-include include/voidlinux.h" make debug
```
