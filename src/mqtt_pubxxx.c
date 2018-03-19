#include <mqtt.h>

ssize_t mqtt_pack_pubxxx_request(uint8_t *buf, size_t bufsz, 
                                 enum MQTTControlPacketType control_type,
                                 uint16_t packet_id) 
{
    const uint8_t const *start = buf;
    struct mqtt_fixed_header fixed_header;
    ssize_t rv;
    if (buf == NULL) {
        return MQTT_ERROR_NULLPTR;
    }

    /* pack fixed header */
    fixed_header.control_type = control_type;
    if (control_type == MQTT_CONTROL_PUBREL) {
        fixed_header.control_flags = 0x02;
    } else {
        fixed_header.control_flags = 0;
    }
    fixed_header.remaining_length = 2;
    rv = mqtt_pack_fixed_header(buf, bufsz, &fixed_header);
    if (rv <= 0) {
        return rv;
    }
    buf += rv;
    bufsz -= rv;

    if (bufsz < fixed_header.remaining_length) {
        return 0;
    }
    
    *(uint16_t*) buf = (uint16_t) htons(packet_id);
    buf += 2;

    return buf - start;
}

ssize_t mqtt_unpack_pubxxx_response(struct mqtt_response *mqtt_response, const uint8_t *buf) 
{
    const uint8_t const *start = buf;
    uint16_t packet_id;

    /* parse packet_id */
    packet_id = (uint16_t) ntohs(*(uint16_t*) buf);
    buf += 2;

    if (mqtt_response->fixed_header.control_type == MQTT_CONTROL_PUBACK) {
        mqtt_response->decoded.puback.packet_id = packet_id;
    } else if (mqtt_response->fixed_header.control_type == MQTT_CONTROL_PUBREC) {
        mqtt_response->decoded.pubrec.packet_id = packet_id;
    } else if (mqtt_response->fixed_header.control_type == MQTT_CONTROL_PUBREL) {
        mqtt_response->decoded.pubrel.packet_id = packet_id;
    }

    return buf - start;
}