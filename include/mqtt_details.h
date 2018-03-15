#ifndef __MQTT_DETAILS_H__
#define __MQTT_DETAILS_H__

#include <mqtt.h>

ssize_t __mqtt_pack_str(uint8_t *buf, const char* str);

#endif