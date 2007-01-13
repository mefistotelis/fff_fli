CPP  = g++
CC   = gcc
WINDRES = windres
DLLTOOL = dlltool
RM = rm -f
MV = mv -f
CP = cp -f
MKDIR = mkdir -p

BIN  = bin/fff_fli$(EXEEXT)
RES  = obj/flifix_private.res
LIBS =
OBJS = \
obj/fff_fli.o \
obj/PrgTools.o \
obj/FLIFile.o \
$(RES)

CXXFLAGS=-c -fpermissive -Wno-write-strings -D__DEBUG__
CFLAGS=-c -Wno-write-strings -D__DEBUG__
LDFLAGS=

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after

clean: clean-custom
	-${RM} $(OBJS) $(BIN) $(LIBS)

all-before:
	$(MKDIR) obj bin

$(BIN): $(OBJS)
	$(CPP) $(LDFLAGS) -o "$@" $(OBJS)

obj/%.o: src/%.cpp Makefile
	$(CPP) $(CXXFLAGS) -o"$@" "$<"

obj/%.o: src/%.c Makefile
	$(CC) $(CFLAGS) -o"$@" "$<"

obj/%.res: res/%.rc Makefile
	$(WINDRES) -i "$<" --input-format=rc -o "$@" -O coff 
