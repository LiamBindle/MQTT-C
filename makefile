
CC = gcc
CFLAGS = -Wextra -Wall -std=gnu99 -Iinclude -Wno-empty-body -Wno-unused-parameter # -Wno-unused-variable -Wno-duplicate-decl-specifier

MQTT_C_SOURCES = src/mqtt.c src/mqtt_pal.c
MQTT_C_EXAMPLES = bin/simple_publisher bin/simple_subscriber bin/reconnect_subscriber bin/bio_publisher bin/openssl_publisher
MQTT_C_UNITTESTS = bin/tests
BINDIR = bin

all: bin/bluazu # $(BINDIR) $(MQTT_C_UNITTESTS) $(MQTT_C_EXAMPLES)

bin/bluazu : examples/bluazu.c $(MQTT_C_SOURCES)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(BINDIR)