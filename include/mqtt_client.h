#ifndef __MQTT_CLIENT_H__
#define __MQTT_CLIENT_H__

#include <stddef.h>

#include <mqtt.h>

/**
 * @brief An MQTT client. 
 */
struct mqtt_client {
    /** @brief the socket connected to the broker. */
    int socketfd;

    /** @brief */
    uint16_t pid_lfsr;

    uint16_t keep_alive;

    enum MqttErrors error;

    uint8_t *recvbuf;
    size_t recvbufsz;
    struct mqtt_message_queue mq;
};

uint16_t __mqtt_next_pid(struct mqtt_client *client);


ssize_t mqtt_init(struct mqtt_client *client,
                  int sockfd,
                  uint8_t *sendbuf, size_t sendbufsz,
                  uint8_t *recvbuf, size_t recvbufsz);

ssize_t mqtt_connect(struct mqtt_client *client,
                     const char* client_id,
                     const char* will_topic,
                     const char* will_message,
                     const char* user_name,
                     const char* password,
                     uint8_t connect_flags,
                     uint16_t keep_alive);

ssize_t mqtt_publish(struct mqtt_client *client,
                     const char* topic_name,
                     void* application_message,
                     size_t application_message_size,
                     uint8_t publish_flags);

ssize_t __mqtt_puback(struct mqtt_client *client, uint16_t packet_id);
ssize_t __mqtt_pubrec(struct mqtt_client *client, uint16_t packet_id);
ssize_t __mqtt_pubrel(struct mqtt_client *client, uint16_t packet_id);
ssize_t __mqtt_pubcomp(struct mqtt_client *client, uint16_t packet_id);

ssize_t mqtt_subscribe(struct mqtt_client *client,
                       const char* topic_name,
                       int max_qos_level);

ssize_t mqtt_unsubscribe(struct mqtt_client *client,
                         const char* topic_name);

ssize_t mqtt_ping(struct mqtt_client *client);

ssize_t mqtt_disconnect(struct mqtt_client *client);

#endif