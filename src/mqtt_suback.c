#include <mqtt.h>


ssize_t mqtt_unpack_suback_response (struct mqtt_response *mqtt_response, const uint8_t *buf, size_t bufsz) {
    const uint8_t const *start = buf;
    uint32_t remaining_length = mqtt_response->fixed_header.remaining_length;
    
    /* assert remaining length is at least 2 (for packet id) */
    if (remaining_length < 2) {
        return MQTT_ERROR_SUBACK_TOO_SMALL;
    }

    /* unpack packet_id */
    mqtt_response->decoded.suback.packet_id = (uint16_t) ntohs(*(uint16_t*) buf);
    buf += 2;
    remaining_length -= 2;

    /* unpack return codes */
    mqtt_response->decoded.suback.num_return_codes = (size_t) remaining_length;
    mqtt_response->decoded.suback.return_codes = buf;
    buf += remaining_length;

    return buf - start;
}