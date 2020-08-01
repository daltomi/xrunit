# xsv

Graphical interface for `sv (control and manage services monitored by runsv)`


<img src="https://git.disroot.org/daltomi/xsv/raw/branch/master/screenshot.png"/>


## Tested on Artix Linux, alpha state.


### Dependencies

* Libraries : **fltk**

* Build:  **g++**, **make**, **fltk-config**

### Build
```bash
make debug
-- or --
make release
```

* Use debug mode while in alpha, bug report.

### Preprocessor directives

| Directive | Description | Default | Type |
|-------------------------------|---------|---------|---------
| SV |  sv binary | /usr/bin/sv | string
| SV_DIR      |  available services directory | /etc/runit/sv | string
| SV_RUN_DIR      |  services directory | /run/runit/service | string
| TIME_UPDATE | seconds of updating the list of service | 5 | integer
| FONT        | FLTK font name  | FL_HELVETICA | integer
| FONT_SZ     | font size | 11 | integer
| ASK_DOWN_SERVICES | service name | tty,dbus,udev,elogind | string

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


This program needs to be run with administrator permissions.

