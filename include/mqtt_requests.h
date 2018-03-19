#ifndef __MQTT_REQUESTS_H__
#define __MQTT_REQUESTS_H__

#include <mqtt.h>

/* mqtt_requests.h
 *
 * Functions in this file pack request packets which are sent from the client 
 * to the MQTT broker.
 * 
 * All functions are responsible for COMPLETE error-checking.
 */

/* connect */
#define MQTT_CONNECT_RESERVED       0x01
#define MQTT_CONNECT_CLEAN_SESSION  0x02
#define MQTT_CONNECT_WILL_FLAG      0x04
#define MQTT_CONNECT_WILL_QOS(qos)  (qos & 0x03) << 3
#define MQTT_CONNECT_WILL_RETAIN    0x20
#define MQTT_CONNECT_PASSWORD       0x40
#define MQTT_CONNECT_USER_NAME      0x80
ssize_t mqtt_pack_connection_request(uint8_t* buf, size_t bufsz, 
                                     const char* client_id,
                                     const char* will_topic,
                                     const char* will_message,
                                     const char* user_name,
                                     const char* password,
                                     uint8_t connect_flags,
                                     uint16_t keep_alive);

/* publish */
#define MQTT_PUBLISH_DUP 0x08
#define MQTT_PUBLISH_QOS(qos) ((qos << 1) & 0x06)
#define MQTT_PUBLISH_RETAIN 0x01
ssize_t mqtt_pack_publish_request(uint8_t *buf, size_t bufsz,
                                  const char* topic_name,
                                  uint16_t packet_id,
                                  void* application_message,
                                  size_t appilcation_message_size,
                                  uint8_t publish_flags);

/* puback pack pubrec pubrel */
ssize_t mqtt_pack_pubxxx_request(uint8_t *buf, size_t bufsz, 
                                 enum MQTTControlPacketType control_type,
                                 uint16_t packet_id);

/* subscribe */
#define MQTT_SUBSCRIBE_REQUEST_MAX_NUM_TOPICS 8
ssize_t mqtt_pack_subscribe_request(uint8_t *buf, size_t bufsz, 
                                    uint16_t packet_id, 
                                    ...); /* null terminated */

/* unsubscribe */
#define MQTT_UNSUBSCRIBE_REQUEST_MAX_NUM_TOPICS 8
ssize_t mqtt_pack_unsubscribe_request(uint8_t *buf, size_t bufsz, 
                                      uint16_t packet_id, 
                                      ...); /* null terminated */

/* ping */
ssize_t mqtt_pack_ping_request(uint8_t *buf, size_t bufsz);

/* disconnect */
ssize_t mqtt_pack_disconnect(uint8_t *buf, size_t bufsz);

#endif