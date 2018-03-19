#include <mqtt.h>

ssize_t mqtt_unpack_unsuback_response(struct mqtt_response *mqtt_response, const uint8_t *buf, size_t bufsz) 
{
    const uint8_t const *start = buf;

    /* check for null ptr */
    if (buf == NULL || mqtt_response == NULL) {
        return MQTT_ERROR_NULLPTR;
    }

    if (mqtt_response->fixed_header.remaining_length < 2) {
        return MQTT_ERROR_MALFORMED_RESPONSE;
    }

    /* parse packet_id */
    mqtt_response->decoded.unsuback.packet_id = (uint16_t) ntohs(*(uint16_t*) buf);
    buf += 2;

    return buf - start;
}