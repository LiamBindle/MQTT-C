#ifndef __MQTT_H__
#define __MQTT_H__

#include <stdint.h>     //< uintXX_t ...
#include <stddef.h>     //< size_t
#include <string.h>     //< strlen ...
#include <limits.h>     //< INT_MIN
#include <sys/types.h>  //< ssize_t
#include <arpa/inet.h>  //< htons ntohs
#include <stdarg.h>

/**
 * @file
 * 
 * @mainpage 
 * Welcome 
 * 
 * @defgroup api API
 * @brief Application programmer interface documentation.
 * 
 * @defgroup packers Control Packet Serialization
 * @brief Documentation of functions and datastructures for MQTT control packet serialization.
 * 
 * @defgroup unpackers Control Packet Deserialization
 * @brief Documentation of functions and datastructures for MQTT control packet deserialization.
 *
 * @note To deserialize a packet from a buffer use \ref mqtt_unpack_response (it's the only 
 *       function you need).
 */


 /**
  * @brief An enumeration of the MQTT control packet types. 
  * @ingroup unpackers
  *
  * @see <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718021">
  * MQTT v3.1.1: MQTT Control Packet Types
  * </a>
  */
  enum MQTTControlPacketType {
    MQTT_CONTROL_CONNECT=1u,
    MQTT_CONTROL_CONNACK=2u,
    MQTT_CONTROL_PUBLISH=3u,
    MQTT_CONTROL_PUBACK=4u,
    MQTT_CONTROL_PUBREC=5u,
    MQTT_CONTROL_PUBREL=6u,
    MQTT_CONTROL_PUBCOMP=7u,
    MQTT_CONTROL_SUBSCRIBE=8u,
    MQTT_CONTROL_SUBACK=9u,
    MQTT_CONTROL_UNSUBSCRIBE=10u,
    MQTT_CONTROL_UNSUBACK=11u,
    MQTT_CONTROL_PINGREQ=12u,
    MQTT_CONTROL_PINGRESP=13u,
    MQTT_CONTROL_DISCONNECT=14u
};

/**
 * @brief The fixed header of an MQTT control packet.
 * @ingroup unpackers
 * 
 * @see <a href="http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718020">
 * MQTT v3.1.1: Fixed Header
 * </a>
 */
struct mqtt_fixed_header {
    /** The type of packet. */
    enum MQTTControlPacketType control_type;

    /** The packets control flags.*/
    uint8_t control_flags: 4;

    /** The remaining size of the packet in bytes (i.e. the size of variable header and payload).*/
    uint32_t remaining_length;
};

#define MQTT_PROTOCOL_LEVEL 0x04

/**
 * @brief An MQTT client. 
 */
struct mqtt_client {
    int socketfd;
};

/***************************************************************************
 *                               MQTT ERRORS                                
 ***************************************************************************/
#define __ALL_MQTT_ERRORS(MQTT_ERROR)                    \
    MQTT_ERROR(MQTT_ERROR_NULLPTR)                       \
    MQTT_ERROR(MQTT_ERROR_CONTROL_FORBIDDEN_TYPE)        \
    MQTT_ERROR(MQTT_ERROR_CONTROL_INVALID_FLAGS)         \
    MQTT_ERROR(MQTT_ERROR_CONTROL_WRONG_TYPE)            \
    MQTT_ERROR(MQTT_ERROR_CONNECT_NULL_CLIENT_ID)        \
    MQTT_ERROR(MQTT_ERROR_CONNECT_NULL_WILL_MESSAGE)     \
    MQTT_ERROR(MQTT_ERROR_CONNECT_FORBIDDEN_WILL_QOS)    \
    MQTT_ERROR(MQTT_ERROR_CONNACK_FORBIDDEN_FLAGS)       \
    MQTT_ERROR(MQTT_ERROR_CONNACK_FORBIDDEN_CODE)        \
    MQTT_ERROR(MQTT_ERROR_PUBLISH_FORBIDDEN_QOS)         \
    MQTT_ERROR(MQTT_ERROR_SUBSCRIBE_TOO_MANY_TOPICS)     \
    MQTT_ERROR(MQTT_ERROR_MALFORMED_RESPONSE)            \
    MQTT_ERROR(MQTT_ERROR_UNSUBSCRIBE_TOO_MANY_TOPICS)   \
    MQTT_ERROR(MQTT_ERROR_RESPONSE_INVALID_CONTROL_TYPE) \

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum MqttErrors {
    MQTT_ERROR_UNKNOWN=INT_MIN,
    __ALL_MQTT_ERRORS(GENERATE_ENUM)
};

const char* mqtt_error_str(enum MqttErrors error);

#include <mqtt_responses.h>
#include <mqtt_requests.h>

#endif