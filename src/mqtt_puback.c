#include <mqtt.h>

ssize_t mqtt_pack_puback_request(uint8_t *buf, size_t bufsz, uint16_t packet_id) 
{
    const uint8_t const *start = buf;
    struct mqtt_fixed_header fixed_header;
    ssize_t rv;
    if (buf == NULL) {
        return MQTT_ERROR_NULLPTR;
    }

    /* pack fixed header */
    fixed_header.control_type = MQTT_CONTROL_PUBACK;
    fixed_header.control_flags = 0;
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

ssize_t mqtt_unpack_puback_response(struct mqtt_response *mqtt_response, const uint8_t *buf, size_t bufsz) 
{
    const uint8_t const *start = buf;

    /* check for null ptr */
    if (buf == NULL || mqtt_response == NULL) {
        return MQTT_ERROR_NULLPTR;
    }

    if (bufsz < mqtt_response->fixed_header.remaining_length) {
        return 0;
    }

    /* parse packet_id */
    mqtt_response->decoded.puback.packet_id = (uint16_t) ntohs(*(uint16_t*) buf);
    buf += 2;

    return buf - start;
}