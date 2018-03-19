#include <mqtt.h>

ssize_t mqtt_unpack_unsuback_response(struct mqtt_response *mqtt_response, const uint8_t *buf) 
{
    const uint8_t const *start = buf;

    if (mqtt_response->fixed_header.remaining_length != 2) {
        return MQTT_ERROR_MALFORMED_RESPONSE;
    }

    /* parse packet_id */
    mqtt_response->decoded.unsuback.packet_id = (uint16_t) ntohs(*(uint16_t*) buf);
    buf += 2;

    return buf - start;
}