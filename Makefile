ifeq ("$(shell which fltk-config  2> /dev/null)","")
$(error 'fltk-config' NOT FOUND)
endif

ifeq ("$(shell which pkg-config  2> /dev/null)","")
$(error 'pkg-config' NOT FOUND)
endif

# optional: libnotify
ifeq ("$(LIB_NOTIFY)","1")
ifeq ("$(shell pkg-config --libs libnotify  2> /dev/null)","")
$(error 'libnotify' NOT FOUND)
endif
CXXFLAGS += -DLIB_NOTIFY $(shell pkg-config --cflags libnotify)
CXXLIBS +=  $(shell pkg-config --libs libnotify)
endif

ifeq ("$(shell which install 2> /dev/null)","")
$(error 'install (coreutils)' NOT FOUND)
endif


APP := xrunit

APP_VER := "6.5"

PKG_REV := "1"

ZIP := $(APP)-$(APP_VER)-$(PKG_REV).zip

PREFIX ?= /usr

CXX ?= g++

FLTK_EXTRA := "--use-images"

FORTIFY_SOURCE:= -Wl,-z,relro,-z,now -fstack-protector -D_FORTIFY_SOURCE=2 -O2

CXXLIBS += $(shell fltk-config --ldflags ${FLTK_EXTRA}) $(FORTIFY_SOURCE)
CXXLIBS_STATIC += $(shell fltk-config --ldstaticflags ${FLTK_EXTRA}) $(FORTIFY_SOURCE)

ifeq ("$(CXX)", "g++")
CXXLIBS_RELEASE :=-Wl,-s
endif

CXXFLAGS ?= -include include/artix.h
CXXFLAGS += -Wall $(shell fltk-config --cxxflags) -Iicons $(FORTIFY_SOURCE)
CXXFLAGS_RELEASE:= -DNDEBUG -Wno-write-strings
CXXFLAGS_DEBUG:= -g -DDEBUG -Wextra -Wimplicit-fallthrough

SOURCE := $(wildcard src/*.cpp)

OBJ := $(patsubst %.cpp,%.o,$(SOURCE))

default: release

version:
	-@sed 's/@APP_VER@/$(APP_VER)/g' src/config.h.in > src/config.h

.ONESHELL:
icons := $(shell cd icons && ./compile_and_run.sh)

release: CXXFLAGS+=$(CXXFLAGS_RELEASE)
release: CXXLIBS+=$(CXXLIBS_RELEASE)
release: version icons $(APP)

debug: CXXFLAGS+=$(CXXFLAGS_DEBUG)
debug: version icons $(APP)

static: CXXFLAGS+=$(CXXFLAGS_RELEASE)
static: CXXLIBS=$(CXXLIBS_STATIC)
static: version icons $(APP)

$(APP): $(OBJ)
	$(CXX) $(OBJ) $(CXXLIBS) -o $(APP)

.o:
	$(CXX) -c $<

dist:
	zip $(ZIP) Makefile src/*.cpp src/*.h  src/*.in README.md icons/* -x icons/icons.h -x src/config.h

install:
	-@install -Dt $(PREFIX)/bin/ -m755 $(APP)


clean:
	-@rm  -v src/*.o $(APP) src/config.h $(ZIP)
