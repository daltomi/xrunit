# xsv

Graphical interface for `sv (control service by runsv)`


<img src="https://git.disroot.org/daltomi/xsv/raw/branch/master/screenshot.png"/>


## Only tested on Artix Linux


### Dependencies

* Libraries : **fltk**

* Build:  **g++**, **make**, **fltk-config**

### Build
```bash
make debug
-- or --
make release
```

### Preprocessor directives

| Directive | Description | Default |
|-------------------------------|---------|---------
| SV |  sv binary | /usr/bin/sv
| SV_RUN_DIR      |  services directory | /run/runit/service
| TIME_UPDATE | seconds of updating the list of service | 5
```
Ej:
CXXFLAGS=-DTIME_UPDATE=2 make debug
```




