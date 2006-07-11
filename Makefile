CPP  = g++
CC   = gcc
WINDRES = windres
DLLTOOL = dlltool
RM = rm -f
MV = mv -f
CP = cp -f
MKDIR = mkdir -p

BIN=bin/fff_fli$(EXEEXT)
LIBS =
OBJS = \
obj/fff_fli.o \
$(RES)

CXXFLAGS=-c -fpermissive -Wno-write-strings
LDFLAGS=

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after

clean: clean-custom
	-${RM} $(OBJS) $(BIN) $(LIBS)

all-before:
	$(MKDIR) obj bin

$(BIN): $(OBJS)
	$(CPP) $(OBJS) -o "$@" $(LDFLAGS)

obj/%.o: src/%.cpp Makefile
	$(CPP) $(CXXFLAGS) -o"$@" "$<"

