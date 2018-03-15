#include <mqtt.h>
#include <mqtt_details.h>


ssize_t mqtt_unpack_connack_response(struct mqtt_response *mqtt_response, const uint8_t *buf, size_t bufsz) {
    const uint8_t const *start = buf;
    struct mqtt_fixed_header *fixed_header;
    struct mqtt_response_connack *response;
    /* check for null pointers */
    if (mqtt_response == NULL || buf == NULL) {
        return MQTT_ERROR_NULLPTR;
    }
    fixed_header = &(mqtt_response->fixed_header);
    response = &(mqtt_response->decoded.connack);

    /* check that the fixed header is the correct type */
    if (fixed_header->control_type != MQTT_CONTROL_CONNACK) {
        return MQTT_ERROR_CONTROL_WRONG_TYPE;
    }

    /* check that the buffer has enough bytes */
    if (fixed_header->remaining_length > bufsz) {
        return 0;
    }

    /* unpack */
    if (*buf & 0xFE) {
        /* only bit 1 can be set */
        return MQTT_ERROR_CONNACK_FORBIDDEN_FLAGS;
    } else {
        response->session_present_flag = *buf++;
    }

    if (*buf > 5u) {
        /* only bit 1 can be set */
        return MQTT_ERROR_CONNACK_FORBIDDEN_CODE;
    } else {
        response->return_code = (enum ConnackReturnCode) *buf++;
    }
    return buf - start;
}