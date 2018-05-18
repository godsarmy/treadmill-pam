# vim: filetype=make
#
#

.SUFFIXES:            # Delete the default suffixes
.SUFFIXES: .c .o

TARGET=unit_test

SRC=\
    AllTests.c CuTest.c mytest.c
LIBS=\
    ../lib/http.c ../lib/request.c
LIBPATH = ../lib

LN = ln -sf

INCLUDES = -I$(LIBPATH)

CC=gcc
CFLAGS = -std=gnu99 -g -O2 $(INCLUDES) -Wall

PWD         := $(shell pwd)

test: $(TARGET)
	$(PWD)/$(TARGET)

build: $(TARGET)

$(TARGET): $(SRC) $(LIBS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) $(TARGET)
