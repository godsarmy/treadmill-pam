# vim: filetype=make
#
#

.SUFFIXES:            # Delete the default suffixes
.SUFFIXES: .c .o

TARGET=login_authz

SRC=\
    login_authz.c
LIBS=\
    ../lib/http.c ../lib/request.c
LIBPATH = ../lib

LN = ln -sf

INCLUDES = -I$(LIBPATH)

ifneq (,$(findstring linux,$(ID_EXEC)))
  BUILDTARGETS = build32
  INSTALLTARGETS = install32
  CC=gcc
  CFLAGS = -std=gnu99 -g -O2 $(INCLUDES) -Wall $(DEFINES) -DSTDLOG
  LDFLAGS = -shared -Wl,-export-dynamic
endif

ifneq (,$(findstring x86_64.linux,$(ID_EXEC)))
  BUILDTARGETS += build64
  INSTALLTARGETS += install64
  ID64_EXEC = .exec/$(ID_EXEC)
  ID32_EXEC = .exec/$(ID_EXEC:x86_64%=ia32%)
  LIBDIR32 = lib/security
  LIBDIR64 = lib64/security
  BIN = bin
else
  ID32_EXEC = .exec/$(ID_EXEC)
  LIBDIR32 = lib/security
  BIN = bin
endif

ifeq (,$(PWD))
  PWD         := $(shell pwd)
endif

INSTALLBASE = ../../install
BUILD32 = ../build32
BUILD64 = ../build64

###############################################################################
# Targets
#
###############################################################################
# Build rules
#
build:   $(BUILDTARGETS)
install: $(INSTALLTARGETS)
	$(LN) .exec/@sys $(INSTALLBASE)/exec

.PHONY: build install clean

$(BUILD32):
	mkdir -p $@

$(BUILD64):
	mkdir -p $@

BINARY32 = $(SRC:%.c=$(BUILD32)/%)
BINARY64 = $(SRC:%.c=$(BUILD64)/%)

$(BINARY64): $(SRC) $(LIBS)
ifneq (,$(findstring x86_64.linux,$(ID_EXEC)))
	$(CC) $(CFLAGS) -o $@ $^
endif

$(BINARY32): $(SRC) $(LIBS)
ifneq (,$(findstring x86_64.linux,$(ID_EXEC)))
	$(CC) $(CFLAGS) -m32 -o $@ $^
else
	$(CC) $(CFLAGS) -o $@ $^
endif

build64: $(BUILD64) $(BINARY64)
build32: $(BUILD32) $(BINARY32)

###############################################################################
# Install rules
#
INSTALL32 = $(INSTALLBASE)/$(ID32_EXEC)
INSTALL64 = $(INSTALLBASE)/$(ID64_EXEC)

$(INSTALL32):
	mkdir -p $@

$(INSTALL64):
	mkdir -p $@

TARGET32 = $(TARGET:%=$(INSTALL32)/$(BIN)/%)
TARGET64 = $(TARGET:%=$(INSTALL64)/$(BIN)/%)

$(TARGET32): $(BINARY32)
	install -C -D -m 555 $< $@

$(TARGET64): $(BINARY64)
	install -C -D -m 555 $< $@

install32: $(INSTALL32) $(BUILD32) $(TARGET32)
install64: $(INSTALL64) $(BUILD64) $(TARGET64)

clean:
	$(RM) $(BINARY32) $(BINARY64)

clobber: clean
	$(RM) $(TARGET32) $(TARGET64)

