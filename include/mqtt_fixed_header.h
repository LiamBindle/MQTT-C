#ifndef __MQTT_FIXED_HEADER_H__
#define __MQTT_FIXED_HEADER_H__

#include <stdint.h>     //< uintXX_t ...
#include <stddef.h>     //< size_t
#include <sys/types.h>  //< ssize_t

/***************************************************************************
 *                              Fixed Header                               
 ***************************************************************************/

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

#endif