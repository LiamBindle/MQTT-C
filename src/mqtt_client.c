#include <time.h>
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

ssize_t mqtt_init(struct mqtt_client *client,
               int sockfd,
               uint8_t *sendbuf, size_t sendbufsz,
               uint8_t *recvbuf, size_t recvbufsz)
{
    if (client == NULL || sendbuf == NULL || recvbuf == NULL) {
        return MQTT_ERROR_NULLPTR;
    }
    client->socketfd = sockfd;
    mqtt_mq_init(&client->mq, sendbuf, sendbufsz);
    client->recvbuf = recvbuf;
    client->recvbufsz = recvbufsz;
    client->error = MQTT_ERROR_CLIENT_NOT_CONNECTED;
    client->response_timeout = 30;
    client->number_of_timeouts = 0;
    client->number_of_keep_alives = 0;
    client->typical_response_time = -1.0;
    return MQTT_OK;
}

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


ssize_t mqtt_connect(struct mqtt_client *client,
                     const char* client_id,
                     const char* will_topic,
                     const char* will_message,
                     const char* user_name,
                     const char* password,
                     uint8_t connect_flags,
                     uint16_t keep_alive)
{
    ssize_t rv;
    struct mqtt_queued_message *msg;
    
    /* try to pack the message */
    MQTT_CLIENT_TRY_PACK(rv, msg, client, 
        mqtt_pack_connection_request(
            client->mq.curr, client->mq.curr_sz,
            client_id, will_topic, will_message, 
            user_name, password, connect_flags, 
            keep_alive
        )
    );
    /* save the control type of the message */
    msg->control_type = MQTT_CONTROL_CONNECT;

    /* update the client's state */
    client->keep_alive = keep_alive;
    if (client->error == MQTT_ERROR_CLIENT_NOT_CONNECTED) {
        client->error = MQTT_OK;
    }
    return MQTT_OK;
}

ssize_t mqtt_publish(struct mqtt_client *client,
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

ssize_t mqtt_subscribe(struct mqtt_client *client,
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

ssize_t mqtt_unsubscribe(struct mqtt_client *client,
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

ssize_t mqtt_ping(struct mqtt_client *client) 
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

ssize_t mqtt_disconnect(struct mqtt_client *client) 
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

        /* determine the state to put the message in */
        switch (msg->control_type) {
        case MQTT_CONTROL_PUBACK:
        case MQTT_CONTROL_PUBCOMP:
        case MQTT_CONTROL_DISCONNECT:
            msg->state = MQTT_QUEUED_COMPLETE;
            break;
        case MQTT_CONTROL_CONNECT:
        case MQTT_CONTROL_PUBLISH:
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