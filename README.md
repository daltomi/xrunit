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

| Directive | Description | Default |
|-------------------------------|---------|---------
| SV |  sv binary | /usr/bin/sv
| SV_DIR      |  available services directory | /etc/runit/sv
| SV_RUN_DIR      |  services directory | /run/runit/service
| TIME_UPDATE | seconds of updating the list of service | 5
| FONT        | FLTK font name  | FL_HELVETICA
| FONT_SZ     | font size | 11

```
Ej:
CXXFLAGS=-DTIME_UPDATE=2 make debug
```


This program needs to be run with administrator permissions.

