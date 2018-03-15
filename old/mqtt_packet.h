#ifndef __MQTT_PACKET_H__
#define __MQTT_PACKET_H__

#include <stdint.h>
#include <stddef.h> 

#include <mqtt_fixed_header.h>


struct mqtt_variable_header_connect {

};

struct mqtt_packet_connect {
    struct mqtt_fixed_header fixed_header;

    /* variable header */
}

struct mqtt_packet_connect {
    const char* client_id;
    const char* will_topic;
    const char* will_message; 
    const char* user_name;
    const char* password;
    uint8_t connect_flags;
    uint16_t keep_alive;
};
ssize_t mqtt_pack_fixed_header_connect(uint8_t *buf, size_t bufsz, struct mqtt_packet_connect *connection_request);
ssize_t mqtt_pack_variable_header_connect(uint8_t *buf, size_t bufsz, struct mqtt_packet_connect *connection_request);
ssize_t mqtt_pack_payload_connect(uint8_t *buf, size_t bufsz, struct mqtt_packet_connect *connection_request);


enum ConnackReturnCode {
    MQTT_CONNACK_ACCEPTED = 0,
    MQTT_CONNACK_REFUSED_PROTOCOL_VERSION = 1,
    MQTT_CONNACK_REFUSED_IDENTIFIER_REJECTED = 2,
    MQTT_CONNACK_REFUSED_SERVER_UNAVAILABLE = 3,
    MQTT_CONNACK_REFUSED_BAD_USER_NAME_OR_PASSWORD = 4,
    MQTT_CONNACK_REFUSED_NOT_AUTHORIZED = 5
};
struct mqtt_packet_connack {
    uint8_t session_present_flag;
    enum ConnackReturnCode connect_return_code;
};
ssize_t mqtt_unpack_variable_header_connack(uint8_t *buf, size_t bufsz, struct mqtt_packet_connect *connection_request);

struct mqtt_packet_publish {
    uint8_t dup_flag;
    uint8_t qos_level;
    uint8_t retain;
    const char* topic_name;
    uint16_t packet_id;
    void* application_message;
    size_t appilcation_message_size;
};
ssize_t mqtt_pack_fixed_header_publish(uint8_t *buf, size_t bufsz, struct mqtt_packet_connect *request);
ssize_t mqtt_pack_variable_header_publish(uint8_t *buf, size_t bufsz, struct mqtt_packet_connect *request);
ssize_t mqtt_pack_payload_publish(uint8_t *buf, size_t bufsz, struct mqtt_packet_connect *request);

ssize_t mqtt_unpack_variable_header_publish(uint8_t *buf, size_t bufsz, struct mqtt_packet_connect *request);
ssize_t mqtt_unpack_payload_publish(uint8_t *buf, size_t bufsz, struct mqtt_packet_connect *request);

struct mqtt_packet {
    enum MQTTControlPacketType control_type;
    union {
        struct mqtt_packet_connect connect;
        struct mqtt_packet_connack connack;
        struct mqtt_packet_publish publish;
    } decoded;
};

struct mqtt_packer {
    struct mqtt_packet packet;
    struct mqtt_fixed_header fixed_header;
    union {

    } variable_headers;
};

ssize_t mqtt_pack(uint8_t *buf, size_t bufsz, const struct mqtt_packet *packet);
ssize_t mqtt_unpack(uint8_t *buf, size_t bufsz, const struct mqtt_packet *packet);

#endif