#include <mqtt.h>
#include <mqtt_details.h>


ssize_t mqtt_pack_connection_request(uint8_t* buf, size_t bufsz, const struct mqtt_connection_request *request) {
    struct mqtt_fixed_header fixed_header;
    uint32_t remaining_length;
    uint8_t connect_flags;
    const uint8_t const* start = buf;
    ssize_t rv;

    /* check for null pointer */
    if (request == NULL) {
        return MQTT_ERROR_NULLPTR;
    }

    /* build up connect flags */
    connect_flags = request->connect_flags & ~MQTT_CONNECT_RESERVED;

    /* pack the fixed header */
    fixed_header.control_type = MQTT_CONTROL_CONNECT;
    fixed_header.control_flags = 0x00;

    /* calculate remaining length */
    remaining_length = 10; /* size of variable header */
    if (request->client_id == NULL) {
        return MQTT_ERROR_CONNECT_NULL_CLIENT_ID;
    } else {
        /* mqtt_string length is strlen + 2 */
        remaining_length += 2 + strlen(request->client_id);
    }
    
    if (request->will_topic != NULL) {
        connect_flags |= MQTT_CONNECT_WILL_FLAG;
        remaining_length += 2 + strlen(request->will_topic);
        
        if (request->will_message == NULL) {
            return MQTT_ERROR_CONNECT_NULL_WILL_MESSAGE;
        }
        remaining_length += 2 + strlen(request->will_message);
    } else {
        connect_flags &= ~MQTT_CONNECT_WILL_FLAG;
        connect_flags &= ~MQTT_CONNECT_WILL_QOS(0x3);
        connect_flags &= ~MQTT_CONNECT_WILL_RETAIN;
    }

    if (request->user_name != NULL) {
        connect_flags |= MQTT_CONNECT_USER_NAME;
        remaining_length += 2 + strlen(request->user_name);
    } else {
        connect_flags &= ~MQTT_CONNECT_USER_NAME;
    }

    if (request->password != NULL) {
        connect_flags |= MQTT_CONNECT_PASSWORD;
        remaining_length += 2 + strlen(request->password);
    } else {
        connect_flags &= ~MQTT_CONNECT_PASSWORD;
    }

    fixed_header.remaining_length = remaining_length;

    /* pack fixed header */
    rv = mqtt_pack_fixed_header(buf, bufsz, &fixed_header);
    if (rv <= 0) return rv; /* error or not enough space in buffer */
    buf += rv;
    bufsz -= rv;

    /* check that we have enough space in the buffer still */
    if (bufsz < fixed_header.remaining_length) {
        return 0;
    }

    /* pack the variable header */
    *buf++ = 0x00;
    *buf++ = 0x04;
    *buf++ = (uint8_t) 'M';
    *buf++ = (uint8_t) 'Q';
    *buf++ = (uint8_t) 'T';
    *buf++ = (uint8_t) 'T';
    *buf++ = MQTT_PROTOCOL_LEVEL;
    *buf++ = connect_flags;
    *(uint16_t*) buf = (uint16_t) htons(request->keep_alive);
    buf += 2;

    /* pack the payload */
    buf += __mqtt_pack_str(buf, request->client_id);
    if (connect_flags & MQTT_CONNECT_WILL_FLAG) {
        buf += __mqtt_pack_str(buf, request->will_topic);
        buf += __mqtt_pack_str(buf, request->will_message);
    }
    if (connect_flags & MQTT_CONNECT_USER_NAME) {
        buf += __mqtt_pack_str(buf, request->user_name);
    }
    if (connect_flags & MQTT_CONNECT_PASSWORD) {
        buf += __mqtt_pack_str(buf, request->password);
    }

    return buf - start;
}

ssize_t mqtt_unpack_connection_response(struct mqtt_connection_response *response, const struct mqtt_fixed_header *fixed_header, const uint8_t *buf, size_t bufsz) {
    const uint8_t const *start = buf;
    /* check for null pointers */
    if (response == NULL || fixed_header == NULL || buf == NULL) {
        return MQTT_ERROR_NULLPTR;
    }

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
        response->connect_return_code = (enum ConnackReturnCode) *buf++;
    }
    return buf - start;
}