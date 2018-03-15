#include <mqtt.h>
#include <mqtt_details.h>

#define MAX_NUM_SUBSCRIBE_TOPIC 8

ssize_t mqtt_pack_subscribe_request(uint8_t *buf, size_t bufsz, uint16_t packet_id, ...) {
    va_list args;
    const uint8_t const *start = buf;
    ssize_t rv;
    struct mqtt_fixed_header fixed_header;
    unsigned int num_subs = 0;
    unsigned int i;
    const char *topic[MAX_NUM_SUBSCRIBE_TOPIC];
    uint8_t max_qos[MAX_NUM_SUBSCRIBE_TOPIC];

    /* parse all subscriptions */
    va_start(args, packet_id);
    while(1) {
        topic[num_subs] = va_arg(args, const char*);
        if (topic[num_subs] == NULL) {
            /* end of list */
            break;
        }

        max_qos[num_subs] = (uint8_t) va_arg(args, unsigned int);

        ++num_subs;
        if (num_subs >= MAX_NUM_SUBSCRIBE_TOPIC) {
            return MQTT_ERROR_SUBSCRIBE_TOO_MANY_TOPICS;
        }
    }
    va_end(args);

    /* build the fixed header */
    fixed_header.control_type = MQTT_CONTROL_SUBSCRIBE;
    fixed_header.control_flags = 2u;
    fixed_header.remaining_length = 2u; /* size of variable header */
    for(i = 0; i < num_subs; ++i) {
        /* payload is topic name + max qos (1 byte) */
        fixed_header.remaining_length += __mqtt_packed_cstrlen(topic[i]) + 1;
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
    *(uint16_t*) buf = (uint16_t) htons(packet_id);
    buf += 2;


    /* pack payload */
    for(i = 0; i < num_subs; ++i) {
        buf += __mqtt_pack_str(buf, topic[i]);
        *buf++ = max_qos[i];
    }

    return buf - start;
}