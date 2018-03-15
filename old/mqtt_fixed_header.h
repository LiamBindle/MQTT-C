#ifndef __MQTT_FIXED_HEADER_H__
#define __MQTT_FIXED_HEADER_H__

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

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

/***************************************************************************
 *                              Fixed Header                               
 ***************************************************************************/
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