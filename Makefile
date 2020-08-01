ifeq ("$(shell which fltk-config  2> /dev/null)","")
$(error 'fltk-config' NOT FOUND)
endif

APP := xsv

APP_VER := "0.7"

CXX ?= g++

FLTK_EXTRA := "--use-images"

CPPLIBS := $(shell fltk-config --ldflags ${FLTK_EXTRA})

ifeq ("$(CXX)", "g++")
CPPLIBS_RELEASE :=-Wl,-s
endif

CXXFLAGS += -Wall $(shell fltk-config --cxxflags) -Iicons
CXXFLAGS_RELEASE:= -O3 -DNDEBUG
CXXFLAGS_DEBUG:= -O2 -g -DDEBUG -Wextra -Wimplicit-fallthrough


SOURCE := $(wildcard src/*.cpp)

OBJ := $(patsubst %.cpp,%.o,$(SOURCE))

default: release

version:
	-@sed 's/@APP_VER@/$(APP_VER)/g' src/config.h.in > src/config.h

.ONESHELL:
icons := $(shell cd icons && ./compile_and_run.sh)

release: CXXFLAGS+=$(CXXFLAGS_RELEASE)
release: CPPLIBS+=$(CPPLIBS_RELEASE)
release: version icons $(APP)

debug: CXXFLAGS+=$(CXXFLAGS_DEBUG)
debug: version icons $(APP)


$(APP): $(OBJ)
	$(CXX) $(OBJ) $(CPPLIBS) -o $(APP)

.o:
	$(CXX) -c $<


clean:
	rm  -v src/*.o $(APP) src/config.h $(ZIP)
