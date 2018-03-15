#ifndef __MQTT_H__
#define __MQTT_H__

#include <stdint.h>     //< uintXX_t ...
#include <stddef.h>     //< size_t
#include <string.h>     //< strlen ...
#include <limits.h>     //< INT_MIN
#include <sys/types.h>  //< ssize_t
#include <arpa/inet.h>  //< htons ntohs

#include <mqtt_fixed_header.h>
#include <mqtt_responses.h>

#define MQTT_PROTOCOL_LEVEL 0x04

/* client */
struct mqtt_client {
    int socketfd;
};

/***************************************************************************
 *                               CONNECT                               
 ***************************************************************************/

#define MQTT_CONNECT_RESERVED       0x01
#define MQTT_CONNECT_CLEAN_SESSION  0x02
#define MQTT_CONNECT_WILL_FLAG      0x04
#define MQTT_CONNECT_WILL_QOS(qos)  (qos & 0x03) << 3
#define MQTT_CONNECT_WILL_RETAIN    0x20
#define MQTT_CONNECT_PASSWORD       0x40
#define MQTT_CONNECT_USER_NAME      0x80

/**
 * @brief Pack a connection request packet.
 */
ssize_t mqtt_pack_connection_request(uint8_t* buf, size_t bufsz, 
                                     const char* client_id,
                                     const char* will_topic,
                                     const char* will_message,
                                     const char* user_name,
                                     const char* password,
                                     uint8_t connect_flags,
                                     uint16_t keep_alive);


/***************************************************************************
 *                               PUBLISH                               
 ***************************************************************************/
#define MQTT_PUBLISH_DUP 0x08
#define MQTT_PUBLISH_QOS(qos) ((qos << 1) & 0x06)
#define MQTT_PUBLISH_RETAIN 0x01

ssize_t mqtt_pack_publish_request(uint8_t *buf, size_t bufsz,
                                  const char* topic_name,
                                  uint16_t packet_id,
                                  void* application_message,
                                  size_t appilcation_message_size,
                                  uint8_t publish_flags);

/***************************************************************************
 *                               PUBACK                               
 ***************************************************************************/
ssize_t mqtt_pack_puback_request(uint8_t *buf, size_t bufsz, uint16_t packet_id);



/***************************************************************************
 *                               DISCONNECT                               
 ***************************************************************************/

/**
 * @brief Pack a disconnect packet.
 * 
 * @param[out] buf the buffer to serialize the request into.
 * @param[in] bufsz the number of bytes in the buffer.
 * 
 * @returns Number of bytes that were packed into the buffer. Zero if there isn't enough space in the 
 *          buffer. A negative value is an error code.
 */
ssize_t mqtt_pack_disconnect(uint8_t *buf, size_t bufsz);





/***************************************************************************
 *                               MQTT ERRORS                                
 ***************************************************************************/
#define __ALL_MQTT_ERRORS(MQTT_ERROR)                 \
    MQTT_ERROR(MQTT_ERROR_NULLPTR)                    \
    MQTT_ERROR(MQTT_ERROR_CONTROL_FORBIDDEN_TYPE)     \
    MQTT_ERROR(MQTT_ERROR_CONTROL_INVALID_FLAGS)      \
    MQTT_ERROR(MQTT_ERROR_CONTROL_WRONG_TYPE)         \
    MQTT_ERROR(MQTT_ERROR_CONNECT_NULL_CLIENT_ID)     \
    MQTT_ERROR(MQTT_ERROR_CONNECT_NULL_WILL_MESSAGE)  \
    MQTT_ERROR(MQTT_ERROR_CONNECT_FORBIDDEN_WILL_QOS) \
    MQTT_ERROR(MQTT_ERROR_CONNACK_FORBIDDEN_FLAGS)    \
    MQTT_ERROR(MQTT_ERROR_CONNACK_FORBIDDEN_CODE)     \
    MQTT_ERROR(MQTT_ERROR_PUBLISH_FORBIDDEN_QOS)      \

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum MqttErrors {
    MQTT_ERROR_UNKNOWN=INT_MIN,
    __ALL_MQTT_ERRORS(GENERATE_ENUM)
};

const char* mqtt_error_str(enum MqttErrors error);

#endif