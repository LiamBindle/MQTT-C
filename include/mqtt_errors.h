#ifndef __MQTT_ERRORS_H__
#define __MQTT_ERRORS_H__

#include <limits.h>

#define __ALL_MQTT_ERRORS(MQTT_ERROR) \
    MQTT_ERROR(MQTT_ERROR_NULLPTR) \
    MQTT_ERROR(MQTT_ERROR_CONTROL_FORBIDDEN_TYPE)   \
    MQTT_ERROR(MQTT_ERROR_CONTROL_INVALID_FLAGS)  \

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum MqttErrors {
    MQTT_ERROR_UNKNOWN=INT_MIN,
    __ALL_MQTT_ERRORS(GENERATE_ENUM)
};

const char* mqtt_error_str(enum MqttErrors error);

#endif