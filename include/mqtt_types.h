#ifndef __MQTT_TYPES_H__
#define __MQTT_TYPES_H__


#include <stdint.h>     //< uintXX_t ...
#include <stddef.h>     //< size_t
#include <sys/types.h>  //< ssize_t


struct mqtt_string {
    uint16_t length; //< string length, big-endian
    char *str;       //< string
};

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

ssize_t mqtt_unpack_fixed_header(struct mqtt_fixed_header *fixed_header, const uint8_t *buf, size_t bufsz) {
    ssize_t consumed = 0;
    /* check for null pointers */
    if (fixed_header == NULL || buf == NULL) {
        return -1;
    }

    /* parse control type and flags */
    fixed_header->control_type  = *buf >> 4;
    fixed_header->control_flags = *buf & 0xF;

    /* consume byte */
    --bufsz;
    ++buf;
    ++consumed;

    /* parse remaining size */
    fixed_header->remaining_length = 0;

    do {
        



    } while(*buf & 0x80); /* while continue bit is set */ 
}

ssize_t mqtt_pack_fixed_header(void* buf, size_t bufsz, const struct mqtt_fixed_header *fixed_header);

struct mqtt_variable_header {

};
ssize_t mqtt_unpack_variable_header(struct mqtt_variable_header *variable_header, const void *buf, size_t bufsz);
ssize_t mqtt_pack_variable_header(void* buf, size_t bufsz, const struct mqtt_variable_header *variable_header);

#endif