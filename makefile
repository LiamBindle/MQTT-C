
CC = gcc
CFLAGS = -Wextra -Wall -std=gnu99 -Iinclude -fPIC -Lbin
LDFLAGS = -shared
VPATH = include:src

BINDIR = bin
OBJDIR = obj
SRCDIR = src
SOURCES := $(wildcard $(SRCDIR)/*.c)
SOURCES := $(patsubst $(SRCDIR)/%.c,%.c,$(SOURCES))
OBJECTS := $(addprefix $(OBJDIR)/,$(SOURCES:%.c=%.o))

LIBMQTT_TARGET := $(BINDIR)/libmqtt.so
LIBMQTT_DEPENDENCIES := $(OBJECTS)

all: dirs $(LIBMQTT_TARGET) tests

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -c $(CFLAGS) $< -o $@

dirs:
	mkdir -p obj
	mkdir -p bin

$(LIBMQTT_TARGET): $(LIBMQTT_DEPENDENCIES)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

TESTS_CFLAGS := -Wno-unused-parameter -Wunused-variable -Wl,-rpath=$(abspath ./bin)
tests: tests.c $(LIBMQTT_TARGET)
	$(CC) $(CFLAGS) $(TESTS_CFLAGS) $< -lcmocka -lmqtt -o $@

clean:
	rm -rf obj bin