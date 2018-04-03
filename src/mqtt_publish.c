#include <mqtt.h>

ssize_t mqtt_pack_publish_request(uint8_t *buf, size_t bufsz,
                                  const char* topic_name,
                                  uint16_t packet_id,
                                  void* application_message,
                                  size_t application_message_size,
                                  uint8_t publish_flags)
{
    const uint8_t const *start = buf;
    ssize_t rv;
    struct mqtt_fixed_header fixed_header;
    uint16_t remaining_length;
    uint8_t temp;

    /* check for null pointers */
    if(buf == NULL || topic_name == NULL) {
        return MQTT_ERROR_NULLPTR;
    }

    /* build the fixed header */
    fixed_header.control_type = MQTT_CONTROL_PUBLISH;

    /* calculate remaining length */
    remaining_length = __mqtt_packed_cstrlen(topic_name) + 2; /* variable header size */
    remaining_length += application_message_size;
    fixed_header.remaining_length = remaining_length;

    /* force dup to 0 if qos is 0 */
    temp = publish_flags & 0x06; /* mask */
    if (temp == 0) {
        /* qos is zero */
        publish_flags &= ~MQTT_PUBLISH_DUP;
    }

    /* make sure that qos is not 3 */
    if (temp == 0x06) {
        return MQTT_ERROR_PUBLISH_FORBIDDEN_QOS;
    }
    fixed_header.control_flags = publish_flags;

    /* pack fixed header */
    rv = mqtt_pack_fixed_header(buf, bufsz, &fixed_header);
    if (rv <= 0) {
        /* something went wrong */
        return rv;
    }
    buf += rv;
    bufsz -= rv;

    /* check that buffer is big enough */
    if (bufsz < remaining_length) {
        return 0;
    }

    /* pack variable header */
    buf += __mqtt_pack_str(buf, topic_name);
    *(uint16_t*) buf = (uint16_t) MQTT_PAL_HTONS(packet_id);
    buf += 2;

    /* pack payload */
    memcpy(buf, application_message, application_message_size);
    buf += application_message_size;

    return buf - start;
}

ssize_t mqtt_unpack_publish_response(struct mqtt_response *mqtt_response, const uint8_t *buf)
{    
    const uint8_t const *start = buf;
    struct mqtt_fixed_header *fixed_header;
    struct mqtt_response_publish *response;
    
    fixed_header = &(mqtt_response->fixed_header);
    response = &(mqtt_response->decoded.publish);

    /* get flags */
    response->dup_flag = (fixed_header->control_flags & MQTT_PUBLISH_DUP) >> 3;
    response->qos_level = (fixed_header->control_flags & 0x06) >> 1;
    response->retain_flag = fixed_header->control_flags & MQTT_PUBLISH_RETAIN;

    /* make sure that remaining length is valid */
    if (mqtt_response->fixed_header.remaining_length < 4) {
        return MQTT_ERROR_MALFORMED_RESPONSE;
    }

    /* parse variable header */
    response->topic_name_size = (uint16_t) MQTT_PAL_NTOHS(*(uint16_t*) buf);
    buf += 2;
    response->topic_name = buf;
    buf += response->topic_name_size;

    response->packet_id = (uint16_t) MQTT_PAL_NTOHS(*(uint16_t*) buf);
    buf += 2;

    /* get payload */
    response->application_message = buf;
    response->application_message_size = fixed_header->remaining_length - response->topic_name_size - 4;
    buf += response->application_message_size;
    
    /* return number of bytes consumed */
    return buf - start;
}