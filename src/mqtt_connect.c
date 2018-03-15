#include <mqtt.h>
#include <mqtt_details.h>


ssize_t mqtt_pack_connection_request(uint8_t* buf, size_t bufsz, const struct mqtt_connection_request *packet) {
    struct mqtt_fixed_header fixed_header;
    uint32_t remaining_length;
    uint8_t connect_flags;
    const uint8_t const* start = buf;
    ssize_t rv;

    /* check for null pointer */
    if (packet == NULL) {
        return MQTT_ERROR_NULLPTR;
    }

    connect_flags = packet->connect_flags & (0xFE);

    /* pack the fixed header */
    fixed_header.control_type = MQTT_CONTROL_CONNECT;
    fixed_header.control_flags = 0x00;

    /* calculate remaining length */
    remaining_length = 10; /* size of variable header */
    if (packet->client_id == NULL) {
        return MQTT_ERROR_CONNECT_NULL_CLIENT_ID;
    } else {
        /* mqtt_string length is strlen + 2 */
        remaining_length += 2 + strlen(packet->client_id);
    }
    
    if (packet->will_topic != NULL) {
        connect_flags |= MQTT_CONNECT_WILL_FLAG;
        remaining_length += 2 + strlen(packet->will_topic);
        
        if (packet->will_message == NULL) {
            return MQTT_ERROR_CONNECT_NULL_WILL_MESSAGE;
        }
        remaining_length += 2 + strlen(packet->will_message);
    }

    if (packet->user_name != NULL) {
        connect_flags |= MQTT_CONNECT_USER_NAME;
        remaining_length += 2 + strlen(packet->user_name);
    }

    if (packet->password != NULL) {
        connect_flags |= MQTT_CONNECT_PASSWORD;
        remaining_length += 2 + strlen(packet->password);
    }

    fixed_header.remaining_length = remaining_length;

    /* pack fixed header */
    rv = mqtt_pack_fixed_header(buf, bufsz, &fixed_header);
    if (rv < 0) return rv; /* error */
    buf += rv;
    bufsz -= rv;

    /* check that we have enough space in the buffer still */
    if (bufsz < fixed_header.remaining_length) {
        return 0;
    }

    /* pack the variable header */
    *(buf++) = 0x00;
    *(buf++) = 0x04;
    *(buf++) = (uint8_t) 'M';
    *(buf++) = (uint8_t) 'Q';
    *(buf++) = (uint8_t) 'T';
    *(buf++) = (uint8_t) 'T';
    *(buf++) = MQTT_PROTOCOL_LEVEL;
    *(buf++) = connect_flags;
    *(uint16_t*) buf = (uint16_t) htons(packet->keep_alive);
    buf += 2;

    /* pack the payload */
    buf += __mqtt_pack_str(buf, packet->client_id);

    if (connect_flags & MQTT_CONNECT_WILL_FLAG) {
        buf += __mqtt_pack_str(buf, packet->will_topic);
        buf += __mqtt_pack_str(buf, packet->will_message);
    }

    if (connect_flags & MQTT_CONNECT_USER_NAME) {
        buf += __mqtt_pack_str(buf, packet->user_name);
    }

    if (connect_flags & MQTT_CONNECT_PASSWORD) {
        buf += __mqtt_pack_str(buf, packet->password);
    }

    return buf - start;
}