#ifndef __MQTT_CLIENT_H__
#define __MQTT_CLIENT_H__

/** @file */

#include <stddef.h>
#include <time.h>   /* time(void*) */

#include <mqtt.h>



/**
 * @brief An MQTT client. 
 * @ingroup details
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
    enum MQTTErrors error;

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

/**
 * @brief Generate a new next packet ID.
 * @ingroup details
 * 
 * Packet ID's are generated using a max-length LFSR.
 * 
 * @param client The MQTT client.
 * 
 * @returns The new packet ID that should be used.
 */
uint16_t __mqtt_next_pid(struct mqtt_client *client);

/**
 * @brief Handles egress client traffic.
 * @ingroup details
 * 
 * @param client The MQTT client.
 * 
 * @returns MQTT_OK upon success, an \ref MQTTErrors otherwise. 
 */
ssize_t __mqtt_send(struct mqtt_client *client);

/**
 * @brief Handles ingress client traffic.
 * @ingroup details
 * 
 * @param client The MQTT client.
 * 
 * @returns MQTT_OK upon success, an \ref MQTTErrors otherwise. 
 */
ssize_t __mqtt_recv(struct mqtt_client *client);


/**
 * @brief Initialize an MQTT client.
 * @ingroup api
 * 
 * This function \em must be called before any other API function calls.
 * 
 * @pre None.
 * 
 * @param[out] client The MQTT client.
 * @param[in] sockfd The socket file descriptor connected to the MQTT broker. 
 * @param[in] sendbuf A buffer that will be used for sending messages to the broker.
 * @param[in] sendbufsz The size of \p sendbuf in bytes.
 * @param[in] recvbuf A buffer that will be used for receiving messages from the broker.
 * @param[in] recvbufsz The size of \p recvbuf in bytes.
 * @param[in] publish_response_callback The callback to call whenever application messages
 *            are received from the broker. 
 * 
 * @post mqtt_connect must be called.
 * 
 * @note \p sockfd is a non-blocking TCP connection.
 * @note If \p sendbuf fills up completely during runtime a \c MQTT_ERROR_SEND_BUFFER_IS_FULL
 *       error will be set. Similarly if \p recvbuf is ever to small to receive a message from
 *       the broker an MQTT_ERROR_RECV_BUFFER_TOO_SMALL error will be set.
 * @note A pointer to \ref mqtt_client.publish_response_callback_state is always passed as the 
 *       \c state argument to \p publish_response_callback. Note that the second argument is 
 *       the mqtt_response_publish that was received from the broker.
 * 
 * @returns \c MQTT_OK upon success, an \ref MQTTErrors otherwise.
 */
enum MQTTErrors mqtt_init(struct mqtt_client *client,
                          int sockfd,
                          uint8_t *sendbuf, size_t sendbufsz,
                          uint8_t *recvbuf, size_t recvbufsz,
                          void (*publish_response_callback)(void** state, struct mqtt_response_publish *publish));

/**
 * @brief Establishes a session with the MQTT broker.
 * @ingroup api
 * 
 * @pre mqtt_init must have been called.
 * 
 * @param[in,out] client The MQTT client.
 * @param[in] client_id The unique name identifying the client.
 * @param[in] will_topic The topic name of client's \p will_message. If no will message is 
 *            desired set to \c NULL.
 * @param[in] will_message The application message (data) to be published in the event the 
 *            client ungracefully disconnects. Set to \c NULL if \p will_topic is \c NULL.
 * @param[in] will_message_size The size of \p will_message in bytes.
 * @param[in] user_name The username to use when establishing the session with the MQTT broker.
 *            Set to \c NULL if a username is not required.
 * @param[in] password The password to use when establishing the session with the MQTT broker.
 *            Set to \c NULL if a password is not required.
 * @param[in] connect_flags Additional \ref MQTTConnectFlags to use when establishing the connection. 
 *            These flags are for forcing the session to start clean, 
 *            \c MQTT_CONNECT_CLEAN_SESSION, the QOS level to publish the \p will_message with 
 *            (provided \c will_message != \c NULL), MQTT_CONNECT_WILL_QOS_[0,1,2], and whether 
 *            or not the broker should retain the \c will_message, MQTT_CONNECT_WILL_RETAIN.
 * @param[in] keep_alive The keep-alive time in seconds. A reasonable value for this is 400 [seconds]. 
 * 
 * @returns \c MQTT_OK upon success, an \ref MQTTErrors otherwise.
 */
enum MQTTErrors mqtt_connect(struct mqtt_client *client,
                             const char* client_id,
                             const char* will_topic,
                             const void* will_message,
                             size_t will_message_size,
                             const char* user_name,
                             const char* password,
                             uint8_t connect_flags,
                             uint16_t keep_alive);

/* 
    todo: will_message should be a void*
*/

/**
 * @brief Publish an application message.
 * @ingroup api
 * 
 * Publishes an application message to the MQTT broker.
 * 
 * @pre mqtt_connect must have been called.
 * 
 * @param[in,out] client The MQTT client.
 * @param[in] topic_name The name of the topic.
 * @param[in] application_message The data to be published.
 * @param[in] application_message_size The size of \p application_message in bytes.
 * @param[in] publish_flags \ref MQTTPublishFlags to be used, namely the QOS level to 
 *            publish at (MQTT_PUBLISH_QOS_[0,1,2]) or whether or not the broker should 
 *            retain the publish (MQTT_PUBLISH_RETAIN).
 * 
 * @returns \c MQTT_OK upon success, an \ref MQTTErrors otherwise.
 */
enum MQTTErrors mqtt_publish(struct mqtt_client *client,
                             const char* topic_name,
                             void* application_message,
                             size_t application_message_size,
                             uint8_t publish_flags);

/**
 * @brief Acknowledge an ingree publish with QOS==1.
 * @ingroup details
 *
 * @param[in,out] client The MQTT client.
 * @param[in] packet_id The packet ID of the ingress publish being acknowledged.
 * 
 * @returns \c MQTT_OK upon success, an \ref MQTTErrors otherwise. 
 */
ssize_t __mqtt_puback(struct mqtt_client *client, uint16_t packet_id);

/**
 * @brief Acknowledge an ingree publish with QOS==2.
 * @ingroup details
 *
 * @param[in,out] client The MQTT client.
 * @param[in] packet_id The packet ID of the ingress publish being acknowledged.
 * 
 * @returns \c MQTT_OK upon success, an \ref MQTTErrors otherwise. 
 */
ssize_t __mqtt_pubrec(struct mqtt_client *client, uint16_t packet_id);

/**
 * @brief Acknowledge an ingree PUBREC packet.
 * @ingroup details
 *
 * @param[in,out] client The MQTT client.
 * @param[in] packet_id The packet ID of the ingress PUBREC being acknowledged.
 * 
 * @returns \c MQTT_OK upon success, an \ref MQTTErrors otherwise. 
 */
ssize_t __mqtt_pubrel(struct mqtt_client *client, uint16_t packet_id);

/**
 * @brief Acknowledge an ingree PUBREL packet.
 * @ingroup details
 *
 * @param[in,out] client The MQTT client.
 * @param[in] packet_id The packet ID of the ingress PUBREL being acknowledged.
 * 
 * @returns \c MQTT_OK upon success, an \ref MQTTErrors otherwise. 
 */
ssize_t __mqtt_pubcomp(struct mqtt_client *client, uint16_t packet_id);


/**
 * @brief Subscribe to a topic.
 * @ingroup api
 * 
 * @pre mqtt_connect must have been called.
 * 
 * @param[in,out] client The MQTT client.
 * @param[in] topic_name The name of the topic to subscribe to.
 * @param[in] max_qos_level The maximum QOS level with which the broker can send application
 *            messages for this topic.
 * 
 * @returns \c MQTT_OK upon success, an \ref MQTTErrors otherwise. 
 */
enum MQTTErrors mqtt_subscribe(struct mqtt_client *client,
                               const char* topic_name,
                               int max_qos_level);

/**
 * @brief Unsubscribe from a topic.
 * @ingroup api
 * 
 * @pre mqtt_connect must have been called.
 * 
 * @param[in,out] client The MQTT client.
 * @param[in] topic_name The name of the topic to unsubscribe from.
 * 
 * @returns \c MQTT_OK upon success, an \ref MQTTErrors otherwise. 
 */
enum MQTTErrors mqtt_unsubscribe(struct mqtt_client *client,
                                 const char* topic_name);

/**
 * @brief Ping the broker. 
 * @ingroup api
 * 
 * @pre mqtt_connect must have been called.
 * 
 * @param[in,out] client The MQTT client.
 * 
 * @returns \c MQTT_OK upon success, an \ref MQTTErrors otherwise.
 */
enum MQTTErrors mqtt_ping(struct mqtt_client *client);

/**
 * @brief Terminate the session with the MQTT broker. 
 * @ingroup api
 * 
 * @pre mqtt_connect must have been called.
 * 
 * @param[in,out] client The MQTT client.
 * 
 * @note To re-establish the session, mqtt_connect must be called.
 * 
 * @returns \c MQTT_OK upon success, an \ref MQTTErrors otherwise.
 */
enum MQTTErrors mqtt_disconnect(struct mqtt_client *client);

#endif