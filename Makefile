CC = gcc
AR = ar
CFLAGS = -g -DCURL_STATICLIB
LDFLAGS = -L./ -lcurl -lrt
INCLUDE :=

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
