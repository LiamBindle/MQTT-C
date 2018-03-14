#ifndef __MQTT_H__
#define __MQTT_H__


#include <stdint.h>     //< uintXX_t ...
#include <stddef.h>     //< size_t
#include <sys/types.h>  //< ssize_t

#include <mqtt_errors.h>


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

ssize_t mqtt_unpack_fixed_header(struct mqtt_fixed_header *fixed_header, const uint8_t *buf, size_t bufsz);
ssize_t mqtt_pack_fixed_header(uint8_t *buf, size_t bufsz, const struct mqtt_fixed_header *fixed_header);

struct mqtt_variable_header {

};
ssize_t mqtt_unpack_variable_header(struct mqtt_variable_header *variable_header, const uint8_t *buf, size_t bufsz);
ssize_t mqtt_pack_variable_header(uint8_t* buf, size_t bufsz, const struct mqtt_variable_header *variable_header);

#endif