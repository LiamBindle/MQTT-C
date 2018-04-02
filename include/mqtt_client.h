#ifndef __MQTT_CLIENT_H__
#define __MQTT_CLIENT_H__

#include <stddef.h>

#include <mqtt.h>



/**
 * @brief An MQTT client. 
 * @ingroup API
 * 
 * @note All members can be manipulated via the related functions.
 */
struct mqtt_client {
    /** @brief The socket connecting to the MQTT broker. */
    int socketfd;

    /** @brief The LFSR state used to generate packet ID's. */
    uint16_t pid_lfsr;

    /** @brief The keep-alive time in seconds. */
    uint16_t keep_alive;

    /** 
     * @brief A counter counting pings that have been sent to keep the connection alive. 
     * @see keep_alive
     */
    int number_of_keep_alives;

    /** 
     * @brief The timestamp of the last message sent to the buffer.
     * 
     * This is used to detect the need for keep-alive pings.
     * 
     * @see keep_alive
    */
    time_t time_of_last_send;

    /** 
     * @brief The error state of the client. 
     * 
     * error should be MQTT_OK for the entirety of the connection.
     * 
     * @note The error state will be MQTT_ERROR_CLIENT_NOT_CONNECTED until
     *       you call mqtt_connect.
     */
    enum MqttErrors error;

    /** 
     * @brief The timeout period in seconds.
     * 
     * If the broker doesn't return an ACK within response_timeout seconds a timeout
     * will occur and the message will be retransmitted. 
     * 
     * @note The default value is 30 [seconds] but you can change it at any time.
     */
    int response_timeout;

    /** @brief A counter counting the number of timeouts that have occurred. */
    int number_of_timeouts;

    /**
     * @brief Approximately much time it has typically taken to receive responses from the 
     *        broker.
     * 
     * @note This is tracked using a exponential-averaging.
     */
    double typical_response_time;

    /**
     * @brief The callback that is called whenever a publish is received from the broker.
     * 
     * Any topics that you have subscribed to will be returned from the broker as 
     * mqtt_response_publish messages. All the publishes received from the broker will 
     * be passed to this function.
     * 
     * @note A pointer to publish_response_callback_state is always passed to the callback.
     *       Use publish_response_callback_state to keep track of any state information you 
     *       need.
     */
    void (*publish_response_callback)(void** state, struct mqtt_response_publish *publish);

    /**
     * @brief A pointer to any publish_response_callback state information you need.
     * 
     * @note A pointer to this pointer will always be publish_response_callback upon 
     *       receiving a publish message from the broker.
     */
    void* publish_response_callback_state;

    /**
     * @brief The buffer where ingress data is temporarily stored.
     */
    struct {
        /** @brief The start of the receive buffer's memory. */
        uint8_t *mem_start;

        /** @brief The size of the receive buffer's memory. */
        size_t mem_size;

        /** @brief A pointer to the next writtable location in the receive buffer. */
        uint8_t *curr;

        /** @brief The number of bytes that are still writable at curr. */
        size_t curr_sz;
    } recv_buffer;

    /** @brief The sending message queue. */
    struct mqtt_message_queue mq;
};

uint16_t __mqtt_next_pid(struct mqtt_client *client);
ssize_t __mqtt_send(struct mqtt_client *client);
ssize_t __mqtt_recv(struct mqtt_client *client);


ssize_t mqtt_init(struct mqtt_client *client,
                  int sockfd,
                  uint8_t *sendbuf, size_t sendbufsz,
                  uint8_t *recvbuf, size_t recvbufsz,
                  void (*publish_response_callback)(void** state, struct mqtt_response_publish *publish));

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