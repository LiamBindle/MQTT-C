#ifndef __MQTT_H__
#define __MQTT_H__

#include <stdint.h>     //< uintXX_t ...
#include <stddef.h>     //< size_t
#include <string.h>     //< strlen ...
#include <limits.h>     //< INT_MIN
#include <sys/types.h>  //< ssize_t
#include <arpa/inet.h>  //< htons ntohs

#define MQTT_PROTOCOL_LEVEL 0x04

/* client */
struct mqtt_client {
    int socketfd;
};

/***************************************************************************
 *                              Fixed Header                               
 ***************************************************************************/

enum MQTTControlPacketType {
    MQTT_CONTROL_CONNECT=1,
    MQTT_CONTROL_CONNACK=2,
    MQTT_CONTROL_PUBLISH=3,
    MQTT_CONTROL_PUBACK=4,
    MQTT_CONTROL_PUBREC=5,
    MQTT_CONTROL_PUBREL=6,
    MQTT_CONTROL_PUBCOMP=7,
    MQTT_CONTROL_SUBSCRIBE=8,
    MQTT_CONTROL_SUBACK=9,
    MQTT_CONTROL_UNSUBSCRIBE=10,
    MQTT_CONTROL_UNSUBACK=11,
    MQTT_CONTROL_PINGREQ=12,
    MQTT_CONTROL_PINGRESP=13,
    MQTT_CONTROL_DISCONNECT=14
};

struct mqtt_fixed_header {
    uint8_t  control_type: 4;
    uint8_t  control_flags: 4;
    uint32_t remaining_length;
};

/**
 * @brief Deserialize the contents of a buffer into a struct mqtt_fixed_header object.
 * 
 * @param[out] fixed_header the fixed header object that will be initialized.
 * @param[in] buf the buffer.
 * @param[in] bufsz the total number of bytes in the buffer.
 * 
 * @returns The number of bytes that were consumed. Zero if the buffer does not 
 *          contain enough bytes to be parsed. A negative value is returned in 
 *          the event of a protocol violation.
 */
ssize_t mqtt_unpack_fixed_header(struct mqtt_fixed_header *fixed_header, const uint8_t *buf, size_t bufsz);

/**
 * @brief Serialize a fixed header object and write to the buffer.
 * 
 * @param[out] buf the buffer where the bytes will be put.
 * @param[in] bufsz the maximum number of bytes that can be put in the buffer.
 * @param[in] fixed_header the fixed header that will be serialized.
 * 
 * @returns The number of bytes that were put into the buffer. Zero if the buffer
 *          does not contain enough bytes to be parsed. A negative value is 
 *          returned in the event of a protocol violation.
 */
ssize_t mqtt_pack_fixed_header(uint8_t *buf, size_t bufsz, const struct mqtt_fixed_header *fixed_header);

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
ssize_t mqtt_pack_connection_request(
    uint8_t* buf, size_t bufsz, 
    const char* client_id,
    const char* will_topic,
    const char* will_message,
    const char* user_name,
    const char* password,
    uint8_t connect_flags,
    uint16_t keep_alive
);

/***************************************************************************
 *                               CONNACK                               
 ***************************************************************************/

enum ConnackReturnCode {
    MQTT_CONNACK_ACCEPTED = 0,
    MQTT_CONNACK_REFUSED_PROTOCOL_VERSION = 1,
    MQTT_CONNACK_REFUSED_IDENTIFIER_REJECTED = 2,
    MQTT_CONNACK_REFUSED_SERVER_UNAVAILABLE = 3,
    MQTT_CONNACK_REFUSED_BAD_USER_NAME_OR_PASSWORD = 4,
    MQTT_CONNACK_REFUSED_NOT_AUTHORIZED = 5
};

struct mqtt_connection_response {
    uint8_t session_present_flag;
    enum ConnackReturnCode connect_return_code;
};
/**
 * @brief Unpack a connection response to a struct mqtt_connection_response object.
 * 
 * @param[out] response the servers response.
 * @param[in] fixed_header the associated fixed header.
 * @param[in] buf the buffer to unpack \c response from.
 * @param[in] bufsz the number of bytes in the buffer.
 * 
 * @returns Number of bytes that were packed into the buffer. Zero if there isn't enough space in the 
 *          buffer. A negative value is an error code.
 */
ssize_t mqtt_unpack_connection_response(struct mqtt_connection_response *response, const struct mqtt_fixed_header *fixed_header, const uint8_t *buf, size_t bufsz);


/***************************************************************************
 *                               PUBLISH                               
 ***************************************************************************/
#define MQTT_PUBLISH_DUP 0x08
#define MQTT_PUBLISH_QOS(qos) ((qos << 1) 0x06)
#define MQTT_PUBLISH_RETAIN 0x01


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

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum MqttErrors {
    MQTT_ERROR_UNKNOWN=INT_MIN,
    __ALL_MQTT_ERRORS(GENERATE_ENUM)
};

const char* mqtt_error_str(enum MqttErrors error);

#endif