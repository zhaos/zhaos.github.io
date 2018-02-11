CC = gcc
AR = ar
CFLAGS = -g -Wall #-DCURL_STATICLIB
LDFLAGS = -L./ -lcurl -lrt -lpthread
INCLUDE :=

ROOTDIR:= $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
DIR := $(notdir $(patsubst %/,%,$(dir $(ROOTDIR))))

PROGRAM := mycurl

SOURCES := $(wildcard *.c)
OBJS    := $(patsubst %.c, %.o, $(SOURCES))

.PHONY: clean

$(PROGRAM): $(OBJS)
	$(CC) $^ $(LDFLAGS) -o $@
%.o: %.c
	$(CC) -c $^ $(CFLAGS) $(INCLUDE)

clean:
	rm *.o $(PROGRAM) -f

rebuild: clean
	make
