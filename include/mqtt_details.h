#ifndef __MQTT_DETAILS_H__
#define __MQTT_DETAILS_H__

#include <mqtt.h>

/** @file */

/**
 * @brief Pack a MQTT string, given a c-string \p str.
 * 
 * @param[out] buf the buffer that the MQTT string will be written to.
 * @param[in] str the c-string to be written to \p buf.
 * 
 * @warning This function provides no error checking.
 * 
 * @returns strlen(str) + 2
*/
ssize_t __mqtt_pack_str(uint8_t *buf, const char* str);

/** @brief A macro to get the MQTT string length from a c-string. */
#define __mqtt_packed_cstrlen(x) (2 + strlen(x))

#endif