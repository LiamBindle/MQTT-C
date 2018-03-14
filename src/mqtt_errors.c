#include <mqtt_errors.h>

static const char *MQTT_ERRORS_STR[] = {
    "MQTT_UNKNOWN_ERROR",
    __ALL_MQTT_ERRORS(GENERATE_STRING)
};

const char* mqtt_error_str(enum MqttErrors error) {
    int offset = error - MQTT_ERROR_UNKNOWN;
    if (offset >= 0) {
        return MQTT_ERRORS_STR[offset];
    } else {
        return MQTT_ERRORS_STR[0];
    }
}