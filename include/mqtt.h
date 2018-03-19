#ifndef __MQTT_H__
#define __MQTT_H__

#include <stdint.h>     //< uintXX_t ...
#include <stddef.h>     //< size_t
#include <string.h>     //< strlen ...
#include <limits.h>     //< INT_MIN
#include <sys/types.h>  //< ssize_t
#include <arpa/inet.h>  //< htons ntohs
#include <stdarg.h>

#include <mqtt_fixed_header.h>
#include <mqtt_responses.h>
#include <mqtt_requests.h>

#define MQTT_PROTOCOL_LEVEL 0x04

/* client */
struct mqtt_client {
    int socketfd;
};

/***************************************************************************
 *                               MQTT ERRORS                                
 ***************************************************************************/
#define __ALL_MQTT_ERRORS(MQTT_ERROR)                  \
    MQTT_ERROR(MQTT_ERROR_NULLPTR)                     \
    MQTT_ERROR(MQTT_ERROR_CONTROL_FORBIDDEN_TYPE)      \
    MQTT_ERROR(MQTT_ERROR_CONTROL_INVALID_FLAGS)       \
    MQTT_ERROR(MQTT_ERROR_CONTROL_WRONG_TYPE)          \
    MQTT_ERROR(MQTT_ERROR_CONNECT_NULL_CLIENT_ID)      \
    MQTT_ERROR(MQTT_ERROR_CONNECT_NULL_WILL_MESSAGE)   \
    MQTT_ERROR(MQTT_ERROR_CONNECT_FORBIDDEN_WILL_QOS)  \
    MQTT_ERROR(MQTT_ERROR_CONNACK_FORBIDDEN_FLAGS)     \
    MQTT_ERROR(MQTT_ERROR_CONNACK_FORBIDDEN_CODE)      \
    MQTT_ERROR(MQTT_ERROR_PUBLISH_FORBIDDEN_QOS)       \
    MQTT_ERROR(MQTT_ERROR_SUBSCRIBE_TOO_MANY_TOPICS)   \
    MQTT_ERROR(MQTT_ERROR_MALFORMED_RESPONSE)          \
    MQTT_ERROR(MQTT_ERROR_UNSUBSCRIBE_TOO_MANY_TOPICS) \

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum MqttErrors {
    MQTT_ERROR_UNKNOWN=INT_MIN,
    __ALL_MQTT_ERRORS(GENERATE_ENUM)
};

const char* mqtt_error_str(enum MqttErrors error);

#endif