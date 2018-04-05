
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

all: dirs $(LIBMQTT_TARGET) tests simple_publisher simple_subscriber

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

simple_publisher: examples/simple_publisher.c src/mqtt.c src/mqtt_pal.c 
	$(CC) $(CFLAGS) -Wno-unused-parameter -Wunused-variable $^ -lpthread -o $@

simple_subscriber: examples/simple_subscriber.c src/mqtt.c src/mqtt_pal.c 
	$(CC) $(CFLAGS) -Wno-unused-parameter -Wunused-variable $^ -lpthread -o $@

clean:
	rm -rf obj bin tests simple_publisher simple_subscriber