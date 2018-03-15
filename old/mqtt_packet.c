#include <mqtt.h>

ssize_t mqtt_pack(uint8_t *buf, size_t bufsz, const struct mqtt_packet *packet) {
    /* check for null pointers */
    if (packet == NULL || buf == NULL) {
        return MQTT_ERROR_NULLPTR; 
    }

    /* pack fixed header */
    if 
}

ssize_t mqtt_unpack(uint8_t *buf, size_t bufsz, const struct mqtt_packet *packet) {

}