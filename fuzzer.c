#include <stdint.h>
#include <stddef.h>
#include "mqtt.h"

extern int LLVMFuzzerTestOneInput(const char *data, size_t size) {
    struct mqtt_response response;

    return mqtt_unpack_response(&response,(uint8_t *)data,size);
}
