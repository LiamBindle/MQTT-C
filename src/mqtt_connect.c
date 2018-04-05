#include <mqtt.h>


ssize_t mqtt_pack_connection_request(uint8_t* buf, size_t bufsz,
                                     const char* client_id,
                                     const char* will_topic,
                                     const void* will_message,
                                     size_t will_message_size,
                                     const char* user_name,
                                     const char* password,
                                     uint8_t connect_flags,
                                     uint16_t keep_alive)
{
    struct mqtt_fixed_header fixed_header;
    uint32_t remaining_length;
    const uint8_t const* start = buf;
    ssize_t rv;
    uint8_t temp;

    /* pack the fixed headr */
    fixed_header.control_type = MQTT_CONTROL_CONNECT;
    fixed_header.control_flags = 0x00;

    /* calculate remaining length and build connect_flags at the same time */
    connect_flags = connect_flags & ~MQTT_CONNECT_RESERVED;
    remaining_length = 10; /* size of variable header */

    if (client_id == NULL) {
        /* client_id is a mandatory parameter */
        return MQTT_ERROR_CONNECT_NULL_CLIENT_ID;
    } else {
        /* mqtt_string length is strlen + 2 */
        remaining_length += __mqtt_packed_cstrlen(client_id);
    }

    if (will_topic != NULL) {
        /* there is a will */
        connect_flags |= MQTT_CONNECT_WILL_FLAG;
        remaining_length += __mqtt_packed_cstrlen(will_topic);

        if (will_message == NULL) {
            /* if there's a will there MUST be a will message */
            return MQTT_ERROR_CONNECT_NULL_WILL_MESSAGE;
        }
        remaining_length += 2 + will_message_size; /* size of will_message */

        /* assert that the will QOS is valid (i.e. not 3) */
        temp = connect_flags & 0x18; /* mask to QOS */
        if (temp == 0x18) {
            /* bitwise equality with QoS 3 (invalid)*/
            return MQTT_ERROR_CONNECT_FORBIDDEN_WILL_QOS;
        }
    } else {
        /* there is no will so set all will flags to zero */
        connect_flags &= ~MQTT_CONNECT_WILL_FLAG;
        connect_flags &= ~0x18;
        connect_flags &= ~MQTT_CONNECT_WILL_RETAIN;
    }

    if (user_name != NULL) {
        /* a user name is present */
        connect_flags |= MQTT_CONNECT_USER_NAME;
        remaining_length += __mqtt_packed_cstrlen(user_name);
    } else {
        connect_flags &= ~MQTT_CONNECT_USER_NAME;
    }

    if (password != NULL) {
        /* a password is present */
        connect_flags |= MQTT_CONNECT_PASSWORD;
        remaining_length += __mqtt_packed_cstrlen(password);
    } else {
        connect_flags &= ~MQTT_CONNECT_PASSWORD;
    }

    /* fixed header length is now calculated*/
    fixed_header.remaining_length = remaining_length;

    /* pack fixed header and perform error checks */
    rv = mqtt_pack_fixed_header(buf, bufsz, &fixed_header);
    if (rv <= 0) {
        /* something went wrong */
        return rv;
    }
    buf += rv;
    bufsz -= rv;

    /* check that the buffer has enough space to fit the remaining length */
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
    *(uint16_t*) buf = (uint16_t) MQTT_PAL_HTONS(keep_alive);
    buf += 2;

    /* pack the payload */
    buf += __mqtt_pack_str(buf, client_id);
    if (connect_flags & MQTT_CONNECT_WILL_FLAG) {
        buf += __mqtt_pack_str(buf, will_topic);
        memcpy(buf, will_message, will_message_size);
        buf += will_message_size;
    }
    if (connect_flags & MQTT_CONNECT_USER_NAME) {
        buf += __mqtt_pack_str(buf, user_name);
    }
    if (connect_flags & MQTT_CONNECT_PASSWORD) {
        buf += __mqtt_pack_str(buf, password);
    }

    /* return the number of bytes that were consumed */
    return buf - start;
}
