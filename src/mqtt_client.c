#include <mqtt_client.h>

uint16_t __mqtt_next_pid(struct mqtt_client *client) {
    if (client->pid_lfsr == 0) {
        client->pid_lfsr = 163u;
    }
    /* LFSR taps taken from: https://en.wikipedia.org/wiki/Linear-feedback_shift_register */
    unsigned lsb = client->pid_lfsr & 1;
    (client->pid_lfsr) >>= 1;
    if (lsb) {
        client->pid_lfsr ^= 0xB400u;
    }
    return client->pid_lfsr;
}

enum MQTTErrors mqtt_init(struct mqtt_client *client,
               int sockfd,
               uint8_t *sendbuf, size_t sendbufsz,
               uint8_t *recvbuf, size_t recvbufsz,
               void (*publish_response_callback)(void** state,struct mqtt_response_publish *publish))
{
    if (client == NULL || sendbuf == NULL || recvbuf == NULL) {
        return MQTT_ERROR_NULLPTR;
    }
    client->socketfd = sockfd;

    mqtt_mq_init(&client->mq, sendbuf, sendbufsz);

    client->recv_buffer.mem_start = recvbuf;
    client->recv_buffer.mem_size = recvbufsz;
    client->recv_buffer.curr = client->recv_buffer.mem_start;
    client->recv_buffer.curr_sz = client->recv_buffer.mem_size;

    client->error = MQTT_ERROR_CLIENT_NOT_CONNECTED;
    client->response_timeout = 30;
    client->number_of_timeouts = 0;
    client->number_of_keep_alives = 0;
    client->typical_response_time = -1.0;
    client->publish_response_callback = publish_response_callback;
    return MQTT_OK;
}

/** 
 * A macro function that:
 *      1) Checks that the client isn't in an error state.
 *      2) Attempts to pack to client's message queue.
 *          a) handles errors
 *          b) if mq buffer is too small, cleans it and tries again
 *      3) Upon successful pack, registers the new message.
 */
#define MQTT_CLIENT_TRY_PACK(tmp, msg, client, pack_call)           \
    if (client->error < 0) {                                        \
        return client->error;                                       \
    }                                                               \
    tmp = pack_call;                                                \
    if (tmp < 0) {                                                  \
        client->error = tmp;                                        \
        return tmp;                                                 \
    } else if (tmp == 0) {                                          \
        mqtt_mq_clean(&client->mq);                                 \
        tmp = pack_call;                                            \
        if (tmp < 0) {                                              \
            client->error = tmp;                                    \
            return tmp;                                             \
        } else if(tmp == 0) {                                       \
            client->error = MQTT_ERROR_SEND_BUFFER_IS_FULL;         \
            return MQTT_ERROR_SEND_BUFFER_IS_FULL;                  \
        }                                                           \
    }                                                               \
    msg = mqtt_mq_register(&client->mq, tmp);                       \


enum MQTTErrors mqtt_connect(struct mqtt_client *client,
                     const char* client_id,
                     const char* will_topic,
                     const void* will_message,
                     size_t will_message_size,
                     const char* user_name,
                     const char* password,
                     uint8_t connect_flags,
                     uint16_t keep_alive)
{
    ssize_t rv;
    struct mqtt_queued_message *msg;

    /* update the client's state */
    client->keep_alive = keep_alive;
    if (client->error == MQTT_ERROR_CLIENT_NOT_CONNECTED) {
        client->error = MQTT_OK;
    }
    
    /* try to pack the message */
    MQTT_CLIENT_TRY_PACK(rv, msg, client, 
        mqtt_pack_connection_request(
            client->mq.curr, client->mq.curr_sz,
            client_id, will_topic, will_message, 
            will_message_size,user_name, password, 
            connect_flags, keep_alive
        )
    );
    /* save the control type of the message */
    msg->control_type = MQTT_CONTROL_CONNECT;

    return MQTT_OK;
}

enum MQTTErrors mqtt_publish(struct mqtt_client *client,
                     const char* topic_name,
                     void* application_message,
                     size_t application_message_size,
                     uint8_t publish_flags)
{
    uint16_t packet_id = __mqtt_next_pid(client);
    ssize_t rv;
    struct mqtt_queued_message *msg;

    /* try to pack the message */
    MQTT_CLIENT_TRY_PACK(
        rv, msg, client, 
        mqtt_pack_publish_request(
            client->mq.curr, client->mq.curr_sz,
            topic_name,
            packet_id,
            application_message,
            application_message_size,
            publish_flags
        )
    );
    /* save the control type and packet id of the message */
    msg->control_type = MQTT_CONTROL_PUBLISH;
    msg->packet_id = packet_id;

    return MQTT_OK;
}

ssize_t __mqtt_puback(struct mqtt_client *client, uint16_t packet_id) {
    ssize_t rv;
    struct mqtt_queued_message *msg;

    /* try to pack the message */
    MQTT_CLIENT_TRY_PACK(
        rv, msg, client, 
        mqtt_pack_pubxxx_request(
            client->mq.curr, client->mq.curr_sz,
            MQTT_CONTROL_PUBACK,
            packet_id
        )
    );
    /* save the control type and packet id of the message */
    msg->control_type = MQTT_CONTROL_PUBACK;
    msg->packet_id = packet_id;

    return MQTT_OK;
}

ssize_t __mqtt_pubrec(struct mqtt_client *client, uint16_t packet_id) {
    ssize_t rv;
    struct mqtt_queued_message *msg;

    /* try to pack the message */
    MQTT_CLIENT_TRY_PACK(
        rv, msg, client, 
        mqtt_pack_pubxxx_request(
            client->mq.curr, client->mq.curr_sz,
            MQTT_CONTROL_PUBREC,
            packet_id
        )
    );
    /* save the control type and packet id of the message */
    msg->control_type = MQTT_CONTROL_PUBREC;
    msg->packet_id = packet_id;

    return MQTT_OK;
}

ssize_t __mqtt_pubrel(struct mqtt_client *client, uint16_t packet_id) {
    ssize_t rv;
    struct mqtt_queued_message *msg;

    /* try to pack the message */
    MQTT_CLIENT_TRY_PACK(
        rv, msg, client, 
        mqtt_pack_pubxxx_request(
            client->mq.curr, client->mq.curr_sz,
            MQTT_CONTROL_PUBREL,
            packet_id
        )
    );
    /* save the control type and packet id of the message */
    msg->control_type = MQTT_CONTROL_PUBREL;
    msg->packet_id = packet_id;

    return MQTT_OK;
}

ssize_t __mqtt_pubcomp(struct mqtt_client *client, uint16_t packet_id) {
    ssize_t rv;
    struct mqtt_queued_message *msg;

    /* try to pack the message */
    MQTT_CLIENT_TRY_PACK(
        rv, msg, client, 
        mqtt_pack_pubxxx_request(
            client->mq.curr, client->mq.curr_sz,
            MQTT_CONTROL_PUBCOMP,
            packet_id
        )
    );
    /* save the control type and packet id of the message */
    msg->control_type = MQTT_CONTROL_PUBCOMP;
    msg->packet_id = packet_id;

    return MQTT_OK;
}

enum MQTTErrors mqtt_subscribe(struct mqtt_client *client,
                       const char* topic_name,
                       int max_qos_level)
{
    uint16_t packet_id = __mqtt_next_pid(client);
    ssize_t rv;
    struct mqtt_queued_message *msg;

    /* try to pack the message */
    MQTT_CLIENT_TRY_PACK(
        rv, msg, client, 
        mqtt_pack_subscribe_request(
            client->mq.curr, client->mq.curr_sz,
            packet_id,
            topic_name,
            max_qos_level,
            NULL
        )
    );
    /* save the control type and packet id of the message */
    msg->control_type = MQTT_CONTROL_SUBSCRIBE;
    msg->packet_id = packet_id;

    return MQTT_OK;
}

enum MQTTErrors mqtt_unsubscribe(struct mqtt_client *client,
                         const char* topic_name)
{
    uint16_t packet_id = __mqtt_next_pid(client);
    ssize_t rv;
    struct mqtt_queued_message *msg;

    /* try to pack the message */
    MQTT_CLIENT_TRY_PACK(
        rv, msg, client, 
        mqtt_pack_unsubscribe_request(
            client->mq.curr, client->mq.curr_sz,
            packet_id,
            topic_name,
            NULL
        )
    );
    /* save the control type and packet id of the message */
    msg->control_type = MQTT_CONTROL_UNSUBSCRIBE;
    msg->packet_id = packet_id;

    return MQTT_OK;
}

enum MQTTErrors mqtt_ping(struct mqtt_client *client) 
{
    ssize_t rv;
    struct mqtt_queued_message *msg;

    /* try to pack the message */
    MQTT_CLIENT_TRY_PACK(
        rv, msg, client, 
        mqtt_pack_ping_request(
            client->mq.curr, client->mq.curr_sz
        )
    );
    /* save the control type and packet id of the message */
    msg->control_type = MQTT_CONTROL_PINGREQ;

    return MQTT_OK;
}

enum MQTTErrors mqtt_disconnect(struct mqtt_client *client) 
{
    ssize_t rv;
    struct mqtt_queued_message *msg;

    /* try to pack the message */
    MQTT_CLIENT_TRY_PACK(
        rv, msg, client, 
        mqtt_pack_disconnect(
            client->mq.curr, client->mq.curr_sz
        )
    );
    /* save the control type and packet id of the message */
    msg->control_type = MQTT_CONTROL_DISCONNECT;

    return MQTT_OK;
}

ssize_t __mqtt_send(struct mqtt_client *client) 
{
    uint8_t inspected;

    if (client->error < 0 && client->error != MQTT_ERROR_SEND_BUFFER_IS_FULL) {
        return client->error;
    }

    /* loop through all messages in the queue */
    int len = mqtt_mq_length(&client->mq);
    for(int i = 0; i < len; ++i) {
        struct mqtt_queued_message *msg = mqtt_mq_get(&client->mq, i);
        int resend = 0;
        if (msg->state == MQTT_QUEUED_UNSENT) {
            /* message has not been sent to lets send it */
            resend = 1;
        } else if (msg->state == MQTT_QUEUED_AWAITING_ACK) {
            /* check for timeout */
            if (time(NULL) > msg->time_sent + client->response_timeout) {
                resend = 1;
                client->number_of_timeouts += 1;
            }
        }

        /* goto next message if we don't need to send */
        if (!resend) {
            continue;
        }

        /* we're sending the message */
        size_t sent = 0;
        while(sent < msg->size) {
            ssize_t tmp = send(client->socketfd, msg->start + sent, msg->size - sent, 0);
            if (tmp < 1) {
                client->error = MQTT_ERROR_SOCKET_ERROR;
                return MQTT_ERROR_SOCKET_ERROR;
            }
            sent += (size_t) tmp;
        }

        /* update timeout watcher */
        client->time_of_last_send = time(NULL);
        msg->time_sent = client->time_of_last_send;

        /* 
        Determine the state to put the message in.
        Control Types:
        MQTT_CONTROL_CONNECT     -> awaiting
        MQTT_CONTROL_CONNACK     -> n/a
        MQTT_CONTROL_PUBLISH     -> qos == 0 ? complete : awaiting
        MQTT_CONTROL_PUBACK      -> complete
        MQTT_CONTROL_PUBREC      -> awaiting
        MQTT_CONTROL_PUBREL      -> awaiting
        MQTT_CONTROL_PUBCOMP     -> complete
        MQTT_CONTROL_SUBSCRIBE   -> awaiting
        MQTT_CONTROL_SUBACK      -> n/a
        MQTT_CONTROL_UNSUBSCRIBE -> awaiting
        MQTT_CONTROL_UNSUBACK    -> n/a
        MQTT_CONTROL_PINGREQ     -> awaiting
        MQTT_CONTROL_PINGRESP    -> n/a
        MQTT_CONTROL_DISCONNECT  -> complete
        */
        switch (msg->control_type) {
        case MQTT_CONTROL_PUBACK:
        case MQTT_CONTROL_PUBCOMP:
        case MQTT_CONTROL_DISCONNECT:
            msg->state = MQTT_QUEUED_COMPLETE;
            break;
        case MQTT_CONTROL_PUBLISH:
            inspected = 0x03 & ((msg->start[0]) >> 1); /* qos */
            if (inspected == 0) {
                msg->state = MQTT_QUEUED_COMPLETE;
            } else if (inspected == 1) {
                msg->state = MQTT_QUEUED_AWAITING_ACK;
                /*set DUP flag for subsequent sends */ 
                msg->start[1] |= MQTT_PUBLISH_DUP;
            } else {
                msg->state = MQTT_QUEUED_AWAITING_ACK;
            }
            break;
        case MQTT_CONTROL_CONNECT:
        case MQTT_CONTROL_PUBREC:
        case MQTT_CONTROL_PUBREL:
        case MQTT_CONTROL_SUBSCRIBE:
        case MQTT_CONTROL_UNSUBSCRIBE:
        case MQTT_CONTROL_PINGREQ:
            msg->state = MQTT_QUEUED_AWAITING_ACK;
            break;
        default:
            client->error = MQTT_ERROR_MALFORMED_REQUEST;
            return MQTT_ERROR_MALFORMED_REQUEST;
        }
    }

    /* check for keep-alive */
    time_t keep_alive_timeout = client->time_of_last_send + (time_t) ((float) (client->keep_alive) * 0.75);
    if (time(NULL) > keep_alive_timeout) {
        ssize_t rv = mqtt_ping(client);
        if (rv != MQTT_OK) {
            client->error = rv;
            return rv;
        }
    }

    return MQTT_OK;
}

ssize_t __mqtt_recv(struct mqtt_client *client) 
{
    struct mqtt_response response;

    /* read until there is nothing left to read */
    while(1) {
        /* read in as many bytes as possible */
        ssize_t rv, consumed;

        rv = mqtt_pal_recvall(client->socketfd, client->recv_buffer.curr, client->recv_buffer.curr_sz, 0);
        if (rv < 0) {
            /* an error occurred */
            client->error = rv;
            return rv;
        } else {
            client->recv_buffer.curr += rv;
            client->recv_buffer.curr_sz -= rv;
        }

        /* attempt to parse */
        consumed = mqtt_unpack_response(&response, client->recv_buffer.mem_start, client->recv_buffer.curr - client->recv_buffer.mem_start);

        if (consumed < 0) {
            client->error = consumed;
            return consumed;
        } else if (consumed == 0) {
            /* if curr_sz is 0 then the buffer is too small to ever fit the message */
            if (client->recv_buffer.curr_sz == 0) {
                client->error = MQTT_ERROR_RECV_BUFFER_TOO_SMALL;
                return MQTT_ERROR_RECV_BUFFER_TOO_SMALL;
            }

            /* just need to wait for the rest of the data */
            return MQTT_OK;
        }

        /* response was unpacked successfully */
        struct mqtt_queued_message *msg = NULL;

        /*
        The switch statement below manages how the client responds to messages from the broker.

        Control Types (that we expect to receive from the broker):
        MQTT_CONTROL_CONNACK:
            -> release associated CONNECT
            -> handle response
        MQTT_CONTROL_PUBLISH:
            -> stage response, none if qos==0, PUBACK if qos==1, PUBREC if qos==2
            -> call publish callback        TODO: prevent redelivery of QOS==2
        MQTT_CONTROL_PUBACK:
            -> release associated PUBLISH
        MQTT_CONTROL_PUBREC:
            -> release PUBLISH
            -> stage PUBREL
        MQTT_CONTROL_PUBREL:
            -> release associated PUBREC
            -> stage PUBCOMP
        MQTT_CONTROL_PUBCOMP:
            -> release PUBREL
        MQTT_CONTROL_SUBACK:
            -> release SUBSCRIBE
            -> handle response
        MQTT_CONTROL_UNSUBACK:
            -> release UNSUBSCRIBE
        MQTT_CONTROL_PINGRESP:
            -> release PINGREQ
        */
        switch (response.fixed_header.control_type) {
            case MQTT_CONTROL_CONNACK:
                /* release associated CONNECT */
                msg = mqtt_mq_find(&client->mq, MQTT_CONTROL_CONNECT, NULL);
                if (msg == NULL) {
                    client->error = MQTT_ERROR_ACK_OF_UNKNOWN;
                    return MQTT_ERROR_ACK_OF_UNKNOWN;
                }
                msg->state = MQTT_QUEUED_COMPLETE;
                /* initialize typical response time */
                client->typical_response_time = (double) (time(NULL) - msg->time_sent);
                /* check that connection was successful */
                if (response.decoded.connack.return_code != MQTT_CONNACK_ACCEPTED) {
                    client->error = MQTT_ERROR_CONNECTION_REFUSED;
                    return MQTT_ERROR_CONNECTION_REFUSED;
                }
                break;
            case MQTT_CONTROL_PUBLISH:
                /* stage response, none if qos==0, PUBACK if qos==1, PUBREC if qos==2 */
                if (response.decoded.publish.qos_level == MQTT_PUBLISH_QOS_1) {
                    rv = __mqtt_puback(client, response.decoded.publish.packet_id);
                    if (rv != MQTT_OK) {
                        client->error = rv;
                        return rv;
                    }
                } else if (response.decoded.publish.qos_level == MQTT_PUBLISH_QOS_2) {
                    /* check if this is a duplicate */
                    if (mqtt_mq_find(&client->mq, MQTT_CONTROL_PUBREC, &response.decoded.publish.packet_id) != NULL) {
                        break;
                    }

                    rv = __mqtt_pubrec(client, response.decoded.publish.packet_id);
                    if (rv != MQTT_OK) {
                        client->error = rv;
                        return rv;
                    }
                }
                /* call publish callback  TODO: prevent redelivery of QOS==2*/
                client->publish_response_callback(&client->publish_response_callback_state, &response.decoded.publish);
                break;
            case MQTT_CONTROL_PUBACK:
                /* release associated PUBLISH */
                msg = mqtt_mq_find(&client->mq, MQTT_CONTROL_PUBLISH, &response.decoded.puback.packet_id);
                if (msg == NULL) {
                    client->error = MQTT_ERROR_ACK_OF_UNKNOWN;
                    return MQTT_ERROR_ACK_OF_UNKNOWN;
                }
                msg->state = MQTT_QUEUED_COMPLETE;
                /* update response time */
                client->typical_response_time = 0.875 * (client->typical_response_time) + 0.125 * (double) (time(NULL) - msg->time_sent);
                break;
            case MQTT_CONTROL_PUBREC:
                /* check if this is a duplicate */
                if (mqtt_mq_find(&client->mq, MQTT_CONTROL_PUBREL, &response.decoded.pubrec.packet_id) != NULL) {
                    break;
                }
                /* release associated PUBLISH */
                msg = mqtt_mq_find(&client->mq, MQTT_CONTROL_PUBLISH, &response.decoded.pubrec.packet_id);
                if (msg == NULL) {
                    client->error = MQTT_ERROR_ACK_OF_UNKNOWN;
                    return MQTT_ERROR_ACK_OF_UNKNOWN;
                }
                msg->state = MQTT_QUEUED_COMPLETE;
                /* update response time */
                client->typical_response_time = 0.875 * (client->typical_response_time) + 0.125 * (double) (time(NULL) - msg->time_sent);
                /* stage PUBREL */
                rv = __mqtt_pubrel(client, response.decoded.pubrec.packet_id);
                if (rv != MQTT_OK) {
                    client->error = rv;
                    return rv;
                }
                break;
            case MQTT_CONTROL_PUBREL:
                /* release associated PUBREC */
                msg = mqtt_mq_find(&client->mq, MQTT_CONTROL_PUBREC, &response.decoded.pubrel.packet_id);
                if (msg == NULL) {
                    client->error = MQTT_ERROR_ACK_OF_UNKNOWN;
                    return MQTT_ERROR_ACK_OF_UNKNOWN;
                }
                msg->state = MQTT_QUEUED_COMPLETE;
                /* update response time */
                client->typical_response_time = 0.875 * (client->typical_response_time) + 0.125 * (double) (time(NULL) - msg->time_sent);
                /* stage PUBCOMP */
                rv = __mqtt_pubcomp(client, response.decoded.pubrec.packet_id);
                if (rv != MQTT_OK) {
                    client->error = rv;
                    return rv;
                }
                break;
            case MQTT_CONTROL_PUBCOMP:
                /* release associated PUBREL */
                msg = mqtt_mq_find(&client->mq, MQTT_CONTROL_PUBREL, &response.decoded.pubcomp.packet_id);
                if (msg == NULL) {
                    client->error = MQTT_ERROR_ACK_OF_UNKNOWN;
                    return MQTT_ERROR_ACK_OF_UNKNOWN;
                }
                msg->state = MQTT_QUEUED_COMPLETE;
                /* update response time */
                client->typical_response_time = 0.875 * (client->typical_response_time) + 0.125 * (double) (time(NULL) - msg->time_sent);
                break;
            case MQTT_CONTROL_SUBACK:
                /* release associated SUBSCRIBE */
                msg = mqtt_mq_find(&client->mq, MQTT_CONTROL_SUBSCRIBE, &response.decoded.suback.packet_id);
                if (msg == NULL) {
                    client->error = MQTT_ERROR_ACK_OF_UNKNOWN;
                    return MQTT_ERROR_ACK_OF_UNKNOWN;
                }
                msg->state = MQTT_QUEUED_COMPLETE;
                /* update response time */
                client->typical_response_time = 0.875 * (client->typical_response_time) + 0.125 * (double) (time(NULL) - msg->time_sent);
                /* check that subscription was successful (not currently only one subscribe at a time) */
                if (response.decoded.suback.return_codes[0] == MQTT_SUBACK_FAILURE) {
                    client->error = MQTT_ERROR_SUBSCRIBE_FAILED;
                    return MQTT_ERROR_SUBSCRIBE_FAILED;
                }
                break;
            case MQTT_CONTROL_UNSUBACK:
                /* release associated UNSUBSCRIBE */
                msg = mqtt_mq_find(&client->mq, MQTT_CONTROL_UNSUBSCRIBE, &response.decoded.unsuback.packet_id);
                if (msg == NULL) {
                    client->error = MQTT_ERROR_ACK_OF_UNKNOWN;
                    return MQTT_ERROR_ACK_OF_UNKNOWN;
                }
                msg->state = MQTT_QUEUED_COMPLETE;
                /* update response time */
                client->typical_response_time = 0.875 * (client->typical_response_time) + 0.125 * (double) (time(NULL) - msg->time_sent);
                break;
            case MQTT_CONTROL_PINGRESP:
                /* release associated PINGREQ */
                msg = mqtt_mq_find(&client->mq, MQTT_CONTROL_PINGREQ, NULL);
                if (msg == NULL) {
                    client->error = MQTT_ERROR_ACK_OF_UNKNOWN;
                    return MQTT_ERROR_ACK_OF_UNKNOWN;
                }
                msg->state = MQTT_QUEUED_COMPLETE;
                /* update response time */
                client->typical_response_time = 0.875 * (client->typical_response_time) + 0.125 * (double) (time(NULL) - msg->time_sent);
                break;
            default:
                client->error = MQTT_ERROR_MALFORMED_RESPONSE;
                return MQTT_ERROR_MALFORMED_RESPONSE;
        }

        /* we've handled the response, now clean the buffer */
        void *dest = client->recv_buffer.mem_start;
        void *src  = client->recv_buffer.mem_start + consumed;
        size_t n = client->recv_buffer.curr - client->recv_buffer.mem_start - consumed;
        memmove(dest, src, n);
        client->recv_buffer.curr -= consumed;
        client->recv_buffer.curr_sz += consumed;
    }

    /* never hit (always return once there's nothing left. */
    return MQTT_OK;
}