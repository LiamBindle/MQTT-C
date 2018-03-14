#include <arpa/inet.h> //< htons, ntohs

#include <mqtt.h>
#include <mqtt_rules.h>

ssize_t mqtt_unpack_variable_header(struct mqtt_variable_header *variable_header, const uint8_t *buf, size_t bufsz) {
    /* check for null pointers */
    if (variable_header == NULL || buf == NULL) {
        return MQTT_ERROR_NULLPTR;
    }

    /* check for too small of a buffer */
    if (bufsz < 2) {
        return 0;
    }

    /* convert the big-endian to little */
    variable_header->packet_idenfier = (uint16_t) ntohs(*(uint16_t*) buf);
    return 2;
}
ssize_t mqtt_pack_variable_header(uint8_t* buf, size_t bufsz, const struct mqtt_variable_header *variable_header) {
    /* check for null pointers */
    if (variable_header == NULL || buf == NULL) {
        return MQTT_ERROR_NULLPTR;
    }

    /* check for too small of a buffer */
    if (bufsz < 2) {
        return 0;
    }

    /* put in big-endian */
    *(uint16_t*) buf = (uint16_t) htons(variable_header->packet_idenfier);
    return 2;
}