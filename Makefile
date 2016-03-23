#******************************************************************************
#  Autodesk Animator FLI files fixer
#******************************************************************************
#   @file Makefile
#      A script used by GNU Make to recompile the project.
#  @par Purpose:
#      Allows to invoke "make all" or similar commands to compile all
#      source code files and link them into executable file.
#  @par Comment:
#      None.
#  @author   Tomasz Lis
#  @date     01 Jan 2004 - 01 Mar 2007
#  @par  Copying and copyrights:
#      This program is free software; you can redistribute it and/or modify
#      it under the terms of the GNU General Public License as published by
#      the Free Software Foundation; either version 2 of the License, or
#      (at your option) any later version.
#
#******************************************************************************
ifneq (,$(findstring Windows,$(OS)))
  RES  = obj/rnc_stdres.res
  EXEEXT = .exe
  PKGFMT = zip
  PKGOS = win
else
  RES  = 
  EXEEXT =
  PKGFMT = tar.gz
  PKGOS = lin
endif

CPP  = g++
CC   = gcc
WINDRES = windres
DLLTOOL = dlltool
RM = rm -f
MV = mv -f
CP = cp -f
MKDIR = mkdir -p
ECHO = @echo
TAR = tar
ZIP = zip

BIN  = bin/fff_fli$(EXEEXT)
RES  = obj/flifix_private.res
INCS =
LIBS =
OBJS = \
obj/fff_fli.o \
obj/PrgTools.o \
obj/FLIFile.o \
$(RES)

# code optimization flags
WARNFLAGS=
DEPFLAGS =
LINKFLAGS=
LINKLIB  =
OPTFLAGS = -O3
DBGFLAGS =
CFLAGS = $(INCS) -c -fmessage-length=0 $(WARNFLAGS) $(DEPFLAGS) $(OPTFLAGS)
LDFLAGS = -fmessage-length=0 $(LINKLIB) $(OPTFLAGS) $(DBGFLAGS) $(LINKFLAGS)

# load program version
include version.mk
VER_STRING = $(VER_MAJOR).$(VER_MINOR).$(VER_RELEASE).$(VER_BUILD)

.PHONY: all all-before all-after clean clean-custom package pkg-before zip tar.gz

all: all-before $(BIN) all-after

clean: clean-custom
	-${RM} $(OBJS) $(BIN) $(LIBS)

all-before:
	$(MKDIR) obj bin

$(BIN): $(OBJS)
	-$(ECHO) 'Building target: $@'
	$(CPP) $(LDFLAGS) -o "$@" $(OBJS)
	-$(ECHO) 'Finished building target: $@'
	-$(ECHO) ' '

obj/%.o: src/%.cpp Makefile
	-$(ECHO) 'Building file: $<'
	$(CPP) $(CXXFLAGS) -o"$@" "$<"
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '

obj/%.o: src/%.c Makefile
	-$(ECHO) 'Building file: $<'
	$(CC) $(CFLAGS) -o"$@" "$<"
	-$(ECHO) 'Finished building: $<'
	-$(ECHO) ' '

obj/%.res: res/%.rc Makefile
	$(WINDRES) -i "$<" --input-format=rc -o "$@" -O coff 

package: pkg-before $(PKGFMT)

pkg-before:
	-${RM} pkg/*
	$(MKDIR) pkg
	$(CP) bin/* pkg/
	$(CP) README.md pkg/

pkg/%.tar.gz: pkg-before
	-$(ECHO) 'Creating package: $<'
	cd $(@D); \
	$(TAR) --owner=0 --group=0 --exclude=*.tar.gz --exclude=*.zip -zcf "$(@F)" .
	-$(ECHO) 'Finished creating: $<'
	-$(ECHO) ' '

tar.gz: pkg/fff_fli-$(subst .,_,$(VER_STRING))-$(PACKAGE_SUFFIX)-$(PKGOS).tar.gz

pkg/%.zip: pkg-before
	-$(ECHO) 'Creating package: $<'
	cd $(@D); \
	$(ZIP) -x*.tar.gz -x*.zip -9 -r "$(@F)" .
	-$(ECHO) 'Finished creating: $<'
	-$(ECHO) ' '

zip: pkg/fff_fli-$(subst .,_,$(VER_STRING))-$(PACKAGE_SUFFIX)-$(PKGOS).zip
