#ifndef __MQTT_RESPONSES_H__
#define __MQTT_RESPONSES_H__

#include <stdint.h>     //< uintXX_t ...
#include <stddef.h>     //< size_t
#include <sys/types.h>  //< ssize_t


#include <mqtt_fixed_header.h>


enum ConnackReturnCode {
    MQTT_CONNACK_ACCEPTED = 0,
    MQTT_CONNACK_REFUSED_PROTOCOL_VERSION = 1,
    MQTT_CONNACK_REFUSED_IDENTIFIER_REJECTED = 2,
    MQTT_CONNACK_REFUSED_SERVER_UNAVAILABLE = 3,
    MQTT_CONNACK_REFUSED_BAD_USER_NAME_OR_PASSWORD = 4,
    MQTT_CONNACK_REFUSED_NOT_AUTHORIZED = 5
};
struct mqtt_response_connack {
    uint8_t session_present_flag;
    enum ConnackReturnCode return_code;
};

struct mqtt_response_publish {
    uint8_t dup_flag;
    uint8_t qos_level;
    uint8_t retain_flag;

    uint16_t topic_name_size;
    const void* topic_name;
    uint16_t packet_id;

    const void* application_message;
    size_t appilcation_message_size;
};

struct mqtt_response_puback {
    uint16_t packet_id;
};
struct mqtt_response_pubrec {
    uint16_t packet_id;
};
struct mqtt_response_pubrel {
    uint16_t packet_id;
};
struct mqtt_response_pubcomp {
    uint16_t packet_id;
};

#define MQTT_SUBACK_SUCCESS_MAX_QOS_0 0x00
#define MQTT_SUBACK_SUCCESS_MAX_QOS_1 0x01
#define MQTT_SUBACK_SUCCESS_MAX_QOS_2 0x02
#define MQTT_SUBACK_FAILURE           0x80
struct mqtt_response_suback {
    uint16_t packet_id;
    const uint8_t *return_codes;
    size_t num_return_codes;
};

struct mqtt_response {
    struct mqtt_fixed_header fixed_header;
    union {
        struct mqtt_response_connack connack;
        struct mqtt_response_publish publish;
        struct mqtt_response_puback  puback;
        struct mqtt_response_pubrec  pubrec;
        struct mqtt_response_pubrel  pubrel;
        struct mqtt_response_pubcomp pubcomp;
        struct mqtt_response_suback  suback;
    } decoded;
};


ssize_t mqtt_unpack_connack_response(struct mqtt_response *mqtt_response, const uint8_t *buf, size_t bufsz);
ssize_t mqtt_unpack_publish_response(struct mqtt_response *mqtt_response, const uint8_t *buf, size_t bufsz);
ssize_t mqtt_unpack_pubxxx_response (struct mqtt_response *mqtt_response, const uint8_t *buf, size_t bufsz);
ssize_t mqtt_unpack_suback_response (struct mqtt_response *mqtt_response, const uint8_t *buf, size_t bufsz);


#endif