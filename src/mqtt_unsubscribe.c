#include <mqtt.h>


ssize_t mqtt_pack_unsubscribe_request(uint8_t *buf, size_t bufsz, uint16_t packet_id, ...) {
    va_list args;
    const uint8_t const *start = buf;
    ssize_t rv;
    struct mqtt_fixed_header fixed_header;
    unsigned int num_subs = 0;
    unsigned int i;
    const char *topic[MQTT_UNSUBSCRIBE_REQUEST_MAX_NUM_TOPICS];

    /* parse all subscriptions */
    va_start(args, packet_id);
    while(1) {
        topic[num_subs] = va_arg(args, const char*);
        if (topic[num_subs] == NULL) {
            /* end of list */
            break;
        }

        ++num_subs;
        if (num_subs >= MQTT_UNSUBSCRIBE_REQUEST_MAX_NUM_TOPICS) {
            return MQTT_ERROR_UNSUBSCRIBE_TOO_MANY_TOPICS;
        }
    }
    va_end(args);

    /* build the fixed header */
    fixed_header.control_type = MQTT_CONTROL_UNSUBSCRIBE;
    fixed_header.control_flags = 2u;
    fixed_header.remaining_length = 2u; /* size of variable header */
    for(i = 0; i < num_subs; ++i) {
        /* payload is topic name */
        fixed_header.remaining_length += __mqtt_packed_cstrlen(topic[i]);
    }

    /* pack the fixed header */
    rv = mqtt_pack_fixed_header(buf, bufsz, &fixed_header);
    if (rv <= 0) {
        return rv;
    }
    buf += rv;
    bufsz -= rv;

    /* check that the buffer has enough space */
    if (bufsz < fixed_header.remaining_length) {
        return 0;
    }

    /* pack variable header */
    *(uint16_t*) buf = (uint16_t) MQTT_PAL_HTONS(packet_id);
    buf += 2;


    /* pack payload */
    for(i = 0; i < num_subs; ++i) {
        buf += __mqtt_pack_str(buf, topic[i]);
    }

    return buf - start;
}