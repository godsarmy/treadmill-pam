# vim: filetype=make
#
# Use this makefile to build cloud/treadmill_pam_unshare
#

.SUFFIXES:            # Delete the default suffixes
.SUFFIXES: .c .o

###############################################################################
TARGET=pam_treadmill
SRC=\
    pam_treadmill.c
LIB_SRC=\
    http.c request.c

###############################################################################
LN = ln -sf
PYTHON = /ms/dist/python/PROJ/core/2.7.1/bin/python


###############################################################################
all:
	@echo "There are 2 distinct steps to this build procedure."
	@echo "  gmake build"
	@echo "  gmake install"
	@echo "They must be executed separately."

###############################################################################
# Determine the M/P/R from the $PWD.
#
ifeq (,$(PWD))
PWD         := $(shell pwd)
endif

###############################################################################

###############################################################################
BUILD32 = ../build32
BUILD64 = ../build64

INSTALLBASE = ../../install

INSTALLC  = $(INSTALLBASE)/common
INSTALLM  = $(INSTALLC)/man/man3

INCLUDES = -I/usr/include/security

# all linux platforms (rel4, rel5, rel6)
#
# ia32.linux.2.6.glibc.2.3
# x86_64.linux.2.6.glibc.2.3
#
ifneq (,$(findstring linux,$(ID_EXEC)))
  BUILDTARGETS = build32
  INSTALLTARGETS = install32
  CC=gcc
  CFLAGS = -std=gnu99 -g -O2 $(INCLUDES) -Wall -fPIC -DPIC -pedantic $(DEFINES)
  LDFLAGS = -shared -Wl,-export-dynamic
endif

ifneq (,$(findstring x86_64.linux,$(ID_EXEC)))
  BUILDTARGETS += build64
  INSTALLTARGETS += install64
  ID64_EXEC = .exec/$(ID_EXEC)
  ID32_EXEC = .exec/$(ID_EXEC:x86_64%=ia32%)
  LIBDIR32 = lib/security
  LIBDIR64 = lib64/security
else
  ID32_EXEC = .exec/$(ID_EXEC)
  LIBDIR32 = lib/security
endif

###############################################################################
# Targets
#
###############################################################################
# Build rules
#
build: $(BUILDTARGETS)

$(BUILD32):
	mkdir -p $@

$(BUILD64):
	mkdir -p $@

OBJECTS32 = $(SRC:%.c=$(BUILD32)/%.o)
OBJECTS64 = $(SRC:%.c=$(BUILD64)/%.o)

OBJECTLIBS32 = $(foreach lib,$(LIB_SRC), $(lib:%.c=$(BUILD32)/%.o))
OBJECTLIBS64 = $(foreach lib,$(LIB_SRC), $(lib:%.c=$(BUILD64)/%.o))

OBJECT32_SUFFIX = $(BUILD32)/%.o
OBJECT64_SUFFIX = $(BUILD64)/%.o

$(OBJECT64_SUFFIX): %.c
ifneq (,$(findstring x86_64.linux,$(ID_EXEC)))
	$(CC) $(CFLAGS) -c -o $@ $<
endif

$(OBJECT32_SUFFIX): %.c
ifneq (,$(findstring x86_64.linux,$(ID_EXEC)))
	$(CC) $(CFLAGS) -c -m32 -o $@ $<
else
	$(CC) $(CFLAGS) -c -o $@ $<
endif

###############################################################################
# Link rules
#
LINK32 = $(TARGET:%=$(BUILD32)/%.so.1)
LINK64 = $(TARGET:%=$(BUILD64)/%.so.1)

$(LINK32): $(OBJECTS32) $(OBJECTLIBS32)
ifneq (,$(findstring x86_64.linux,$(ID_EXEC)))
	$(CC) $(CFLAGS) -m32 -o $@ $^ $(LDFLAGS)
else
ifneq (,$(findstring linux,$(ID_EXEC)))
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
endif
endif

$(LINK64): $(OBJECTS64) $(OBJECTLIBS64)
ifneq (,$(findstring x86_64.linux,$(ID_EXEC)))
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
endif

build32: $(BUILD32) $(LINK32)
build64: $(BUILD64) $(LINK64)

###############################################################################
# Install rules
#
install: $(INSTALLTARGETS)

INSTALL32 = $(INSTALLBASE)/$(ID32_EXEC)
INSTALL64 = $(INSTALLBASE)/$(ID64_EXEC)

$(INSTALL32):
	mkdir -p $@

$(INSTALL64):
	mkdir -p $@

TARGET32 = $(TARGET:%=$(INSTALL32)/$(LIBDIR32)/%.so.1)
TARGET64 = $(TARGET:%=$(INSTALL64)/$(LIBDIR64)/%.so.1)
COMPAT32 = $(TARGET:%=$(INSTALL64)/$(LIBDIR32)/%.so.1)

$(TARGET32): $(LINK32)
	install -C -D -m 555 $< $@
	$(LN) $(shell basename $@) $(basename $@)

$(TARGET64): $(LINK64)
	install -C -D -m 555 $< $@
	$(LN) $(shell basename $@) $(basename $@)

$(COMPAT32): $(TARGET32)
	mkdir -p $(shell dirname $@)
# XXX: This calculate the relative path for the symlink
	$(LN) $(shell echo 'import os; print os.path.relpath("$(TARGET32)", \
		"$(INSTALL64)/$(LIBDIR32)")' | $(PYTHON)) $(basename $@)

install32: $(INSTALL32) $(BUILD32) $(TARGET32)
install64: $(INSTALL64) $(BUILD32) $(BUILD64) $(TARGET64) $(COMPAT32)

clean:
	$(RM) $(BUILD32)/* $(BUILD64)/* || true

clobber: clean
	@echo $(RM) $(TARGET32) $(TARGET64) $(COMPAT32)

.PHONY: build install clean

.KEEP_STATE:

noop:
	@echo nothing to do on $(ID_EXEC)
