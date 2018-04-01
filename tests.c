#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include <mqtt.h>
#include <mqtt_mq.h>

int conf_client(const char* addr, const char* port, const struct addrinfo* hints, struct sockaddr_storage* sockaddr) {
    int sockfd = -1;
    int rv;
    struct addrinfo *p, *servinfo;
    char errbuf[128];

    /* get address information */
    rv = getaddrinfo(addr, port, hints, &servinfo);
    if(rv != 0) {
        fprintf(stderr, "error: %s: line %d: getaddrinfo: %s\n",
            __FILE__, __LINE__ - 3, gai_strerror(rv)
        );
        return -1;
    }

    /* open the first possible socket */
    for(p = servinfo; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) {
            sprintf(errbuf, "error: %s: line %d: socket: ", __FILE__, __LINE__ - 2);
            perror(errbuf);
            continue;
        }

        /* connect to server */
        rv = connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
        if(rv == -1) {
            sprintf(errbuf, "error: %s: line %d: connect: ", __FILE__, __LINE__ - 2);
            perror(errbuf);
            continue;
        }
        break;
    }  

    /* memcpy the configured socket info */
    if(sockaddr != NULL) memcpy(sockaddr, p->ai_addr, p->ai_addrlen);

    /* free servinfo */
    freeaddrinfo(servinfo);

    /* return the new socket fd */
    return sockfd;  
}

static void test_mqtt_fixed_header(void** state) {
    uint8_t correct_buf[1024];
    uint8_t buf[1024];
    struct mqtt_response response;
    struct mqtt_fixed_header *fixed_header = &response.fixed_header;
    ssize_t rv;

    /* sanity check with valid fixed_header */
    correct_buf[0] = (MQTT_CONTROL_CONNECT << 4) | 0;
    correct_buf[1] = 193u;
    correct_buf[2] = 2u;

    /* check that unpack is correct */
    rv = mqtt_unpack_fixed_header(&response, correct_buf, sizeof(correct_buf));
    assert_true(rv == 3);
    assert_true(fixed_header->control_type == MQTT_CONTROL_CONNECT);
    assert_true(fixed_header->control_flags == 0);
    assert_true(fixed_header->remaining_length == 321);

    /* check that unpack is correct */
    rv = mqtt_pack_fixed_header(buf, sizeof(buf), fixed_header);
    assert_true(rv == 3);
    assert_true(memcmp(correct_buf, buf, 3) == 0);


    /* check that invalid flags are caught */
    correct_buf[0] = (MQTT_CONTROL_CONNECT << 4) | 1;
    rv = mqtt_unpack_fixed_header(&response, correct_buf, sizeof(correct_buf));
    assert_true(rv == MQTT_ERROR_CONTROL_INVALID_FLAGS);

    /* check that valid flags are ok when there is a required bit */
    correct_buf[0] = (MQTT_CONTROL_PUBREL << 4) | 2;
    rv = mqtt_unpack_fixed_header(&response, correct_buf, sizeof(correct_buf));
    assert_true(rv == 3);

    /* check that invalid flags are ok when there is a required bit */
    correct_buf[0] = (MQTT_CONTROL_PUBREL << 4) | 3;
    rv = mqtt_unpack_fixed_header(&response, correct_buf, sizeof(correct_buf));
    assert_true(rv == MQTT_ERROR_CONTROL_INVALID_FLAGS);

    /* check that valid flags are ok when there are optional flags */
    correct_buf[0] = (MQTT_CONTROL_PUBLISH << 4) | 0xF;
    rv = mqtt_unpack_fixed_header(&response, correct_buf, sizeof(correct_buf));
    assert_true(rv == 3);
    
    correct_buf[0] = (MQTT_CONTROL_PUBLISH << 4) | 3;
    rv = mqtt_unpack_fixed_header(&response, correct_buf, sizeof(correct_buf));
    assert_true(rv == 3);


    /* check that remaining length is packed/unpacked correctly */
    correct_buf[0] = (MQTT_CONTROL_CONNECT << 4) | 0;
    correct_buf[1] = 64;

    /* check that unpack is correct */
    rv = mqtt_unpack_fixed_header(&response, correct_buf, sizeof(correct_buf));
    assert_true(rv == 2);
    assert_true(fixed_header->control_type == MQTT_CONTROL_CONNECT);
    assert_true(fixed_header->control_flags == 0);
    assert_true(fixed_header->remaining_length == 64);

    /* check that unpack is correct */
    rv = mqtt_pack_fixed_header(buf, sizeof(buf), fixed_header);
    assert_true(rv == 2);
    assert_true(memcmp(correct_buf, buf, 2) == 0);


    /* check that remaining length is packed/unpacked correctly */
    correct_buf[0] = (MQTT_CONTROL_CONNECT << 4) | 0;
    correct_buf[1] = 127;

    /* check that unpack is correct */
    rv = mqtt_unpack_fixed_header(&response, correct_buf, sizeof(correct_buf));
    assert_true(rv == 2);
    assert_true(fixed_header->control_type == MQTT_CONTROL_CONNECT);
    assert_true(fixed_header->control_flags == 0);
    assert_true(fixed_header->remaining_length == 127);

    /* check that unpack is correct */
    rv = mqtt_pack_fixed_header(buf, sizeof(buf), fixed_header);
    assert_true(rv == 2);
    assert_true(memcmp(correct_buf, buf, 2) == 0);


    /* check that remaining length is packed/unpacked correctly */
    correct_buf[0] = (MQTT_CONTROL_CONNECT << 4) | 0;
    correct_buf[1] = 128;
    correct_buf[2] = 1;

    /* check that unpack is correct */
    rv = mqtt_unpack_fixed_header(&response, correct_buf, sizeof(correct_buf));
    assert_true(rv == 3);
    assert_true(fixed_header->control_type == MQTT_CONTROL_CONNECT);
    assert_true(fixed_header->control_flags == 0);
    assert_true(fixed_header->remaining_length == 128);

    /* check that unpack is correct */
    rv = mqtt_pack_fixed_header(buf, sizeof(buf), fixed_header);
    assert_true(rv == 3);
    assert_true(memcmp(correct_buf, buf, 3) == 0);

    /* check bad inputs */
    assert_true( mqtt_pack_fixed_header(NULL, 5, fixed_header) == MQTT_ERROR_NULLPTR );
    assert_true( mqtt_pack_fixed_header(buf, 5, NULL) == MQTT_ERROR_NULLPTR );
    assert_true( mqtt_pack_fixed_header(buf, 2, fixed_header) == 0 );

    assert_true( mqtt_unpack_fixed_header(NULL, buf, 5) == MQTT_ERROR_NULLPTR );
    assert_true( mqtt_unpack_fixed_header(&response, NULL, 5) == MQTT_ERROR_NULLPTR );
    assert_true( mqtt_unpack_fixed_header(&response, buf, 2) == 0 );
}

static void test_mqtt_pack_connection_request(void** state) {
    uint8_t buf[256];
    ssize_t rv;
    const uint8_t correct_bytes[] = {
        (MQTT_CONTROL_DISCONNECT << 4) | 0, 16,
        0, 4, 'M', 'Q', 'T', 'T', MQTT_PROTOCOL_LEVEL, 0, 120u, 
        0, 4, 'l', 'i', 'a', 'm'
    };
    struct mqtt_response response;
    struct mqtt_fixed_header *fixed_header = &response.fixed_header;

    rv = mqtt_pack_connection_request(buf, sizeof(buf), "liam", NULL, NULL, NULL, NULL, 0, 120u);
    assert_true(rv == 18);

    /* check that fixed header is correct */
    rv = mqtt_unpack_fixed_header(&response, buf, rv);
    assert_true(fixed_header->control_type == MQTT_CONTROL_CONNECT);
    assert_true(fixed_header->remaining_length == 16);

    /* check that memory is correct */
    assert_true(memcmp(correct_bytes, buf, sizeof(correct_bytes)));
}

static void test_mqtt_pack_publish(void** state) {
    uint8_t buf[256];
    ssize_t rv;
    const uint8_t correct_bytes[] = {
        (MQTT_CONTROL_PUBLISH << 4) | MQTT_PUBLISH_RETAIN, 20,
        0, 6, 't', 'o', 'p', 'i', 'c', '1', 0, 23,
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
    };
    struct mqtt_response mqtt_response;
    struct mqtt_response_publish *response;
    response = &(mqtt_response.decoded.publish);
    
    
    rv = mqtt_pack_publish_request(buf, 256, "topic1", 23, "0123456789", 10, MQTT_PUBLISH_RETAIN);
    assert_true(rv == 22);
    assert_true(memcmp(buf, correct_bytes, 22) == 0);

    rv = mqtt_unpack_fixed_header(&mqtt_response, buf, 22);
    assert_true(rv == 2);
    rv = mqtt_unpack_publish_response(&mqtt_response, buf + 2);
    assert_true(response->qos_level == 0);
    assert_true(response->retain_flag == 1);
    assert_true(response->dup_flag == 0);
    assert_true(response->topic_name_size == 6);
    assert_true(memcmp(response->topic_name, "topic1", 6) == 0);
    assert_true(response->application_message_size == 10);
    assert_true(memcmp(response->application_message, "0123456789", 10) == 0);
}

static void test_mosquitto_connect_disconnect(void** state) {
    uint8_t buf[256];
    const char* addr = "test.mosquitto.org";
    const char* port = "1883";
    struct addrinfo hints = {0};
    struct sockaddr_storage sockaddr;
    struct mqtt_client client;
    ssize_t rv;
    struct mqtt_response mqtt_response;

    hints.ai_family = AF_INET;         /* use IPv4 */
    hints.ai_socktype = SOCK_STREAM;    /* TCP */
    client.socketfd = conf_client(addr, port, &hints, &sockaddr);
    assert_true(client.socketfd != -1);

    rv = mqtt_pack_connection_request(buf, sizeof(buf), "liam-123456", NULL, NULL, NULL, NULL, 0, 30);
    assert_true(rv > 0);
    assert_true(send(client.socketfd, buf, rv, 0) != -1);

    /* receive connack */
    assert_true(recv(client.socketfd, buf, sizeof(buf), 0) != -1);
    rv = mqtt_unpack_fixed_header(&mqtt_response, buf, sizeof(buf));
    assert_true(rv > 0);
    assert_true(mqtt_unpack_connack_response(&mqtt_response, buf + rv) > 0);
    assert_true(mqtt_response.decoded.connack.return_code == MQTT_CONNACK_ACCEPTED);

    /* disconnect */
    rv = mqtt_pack_disconnect(buf, sizeof(buf));
    assert_true(rv > 0);
    assert_true(send(client.socketfd, buf, rv, 0) != -1);

    /*close the socket */
    close(client.socketfd);
}

static void test_mqtt_unpack_connection_response(void** state) {
    uint8_t buf[] = {
        (MQTT_CONTROL_CONNACK << 4) | 0, 2,        
        0, MQTT_CONNACK_ACCEPTED
    };
    struct mqtt_response mqtt_response;
    ssize_t rv = mqtt_unpack_fixed_header(&mqtt_response, buf, sizeof(buf));
    assert_true(rv == 2);
    assert_true(mqtt_response.fixed_header.control_type == MQTT_CONTROL_CONNACK);

    /* unpack response */
    rv = mqtt_unpack_connack_response(&mqtt_response, buf+2);
    assert_true(rv == 2);
    assert_true(mqtt_response.decoded.connack.session_present_flag == 0);
    assert_true(mqtt_response.decoded.connack.return_code == MQTT_CONNACK_ACCEPTED);
}

static void test_mqtt_pubxxx(void** state) {
    uint8_t buf[256];
    ssize_t rv;
    struct mqtt_response response;
    uint8_t puback_correct_bytes[] = {
        MQTT_CONTROL_PUBACK << 4, 2,
        0, 213u
    };
    uint8_t pubrec_correct_bytes[] = {
        MQTT_CONTROL_PUBREC << 4, 2,
        0, 213u
    };
    uint8_t pubrel_correct_bytes[] = {
        MQTT_CONTROL_PUBREL << 4 | 2u, 2,
        0, 213u
    };
    uint8_t pubcomp_correct_bytes[] = {
        MQTT_CONTROL_PUBCOMP << 4, 2,
        0, 213u
    };

    /* puback */
    rv = mqtt_pack_pubxxx_request(buf, 256, MQTT_CONTROL_PUBACK, 213u);
    assert_true(rv == 4);
    assert_true(memcmp(puback_correct_bytes, buf, 4) == 0);

    rv = mqtt_unpack_fixed_header(&response, buf, 256);
    assert_true(rv == 2);
    assert_true(response.fixed_header.control_type == MQTT_CONTROL_PUBACK);
    rv = mqtt_unpack_pubxxx_response(&response, buf + 2);
    assert_true(rv == 2);
    assert_true(response.decoded.puback.packet_id == 213u);

    /* pubrec */
    rv = mqtt_pack_pubxxx_request(buf, 256, MQTT_CONTROL_PUBREC, 213u);
    assert_true(rv == 4);
    assert_true(memcmp(pubrec_correct_bytes, buf, 4) == 0);

    rv = mqtt_unpack_fixed_header(&response, buf, 256);
    assert_true(rv == 2);
    assert_true(response.fixed_header.control_type == MQTT_CONTROL_PUBREC);
    rv = mqtt_unpack_pubxxx_response(&response, buf + 2);
    assert_true(rv == 2);
    assert_true(response.decoded.pubrec.packet_id == 213u);

    /* pubrel */
    rv = mqtt_pack_pubxxx_request(buf, 256, MQTT_CONTROL_PUBREL, 213u);
    assert_true(rv == 4);
    assert_true(memcmp(pubrel_correct_bytes, buf, 4) == 0);

    rv = mqtt_unpack_fixed_header(&response, buf, 256);
    assert_true(rv == 2);
    assert_true(response.fixed_header.control_type == MQTT_CONTROL_PUBREL);
    rv = mqtt_unpack_pubxxx_response(&response, buf + 2);
    assert_true(rv == 2);
    assert_true(response.decoded.pubrel.packet_id == 213u);

    /* pubcomp */
    rv = mqtt_pack_pubxxx_request(buf, 256, MQTT_CONTROL_PUBCOMP, 213u);
    assert_true(rv == 4);
    assert_true(memcmp(pubcomp_correct_bytes, buf, 4) == 0);

    rv = mqtt_unpack_fixed_header(&response, buf, 256);
    assert_true(rv == 2);
    assert_true(response.fixed_header.control_type == MQTT_CONTROL_PUBCOMP);
    rv = mqtt_unpack_pubxxx_response(&response, buf + 2);
    assert_true(rv == 2);
    assert_true(response.decoded.pubcomp.packet_id == 213u);
}

static void test_mqtt_pack_subscribe(void** state) {
    uint8_t buf[256];
    ssize_t rv;
    const uint8_t correct[] = {
        MQTT_CONTROL_SUBSCRIBE << 4 | 2u, 23,
        0, 132u,
        0, 3, 'a', '/', 'b', 0u,
        0, 5, 'b', 'b', 'b', '/', 'x', 1u,
        0, 4, 'c', '/', 'd', 'd', 0u,
    };

    rv = mqtt_pack_subscribe_request(buf, 256, 132, "a/b", 0, "bbb/x", 1, "c/dd", 0, NULL);
    assert_true(rv == 25);
    assert_true(memcmp(buf, correct, 25) == 0);
}

static void test_mqtt_unpack_suback(void** state) {
    ssize_t rv;
    struct mqtt_response response;
    const uint8_t buf[] = {
        MQTT_CONTROL_SUBACK << 4, 5,
        0, 132u,
        MQTT_SUBACK_SUCCESS_MAX_QOS_0,
        MQTT_SUBACK_SUCCESS_MAX_QOS_1,
        MQTT_SUBACK_FAILURE
    };
    rv = mqtt_unpack_fixed_header(&response, buf, sizeof(buf));
    assert_true(rv == 2);
    assert_true(response.fixed_header.control_type == MQTT_CONTROL_SUBACK);
    rv = mqtt_unpack_suback_response(&response, buf + 2);
    assert_true(rv == 5);
    assert_true(response.decoded.suback.packet_id == 132u);
    assert_true(response.decoded.suback.num_return_codes == 3);
    assert_true(response.decoded.suback.return_codes[0] == MQTT_SUBACK_SUCCESS_MAX_QOS_0);
    assert_true(response.decoded.suback.return_codes[1] == MQTT_SUBACK_SUCCESS_MAX_QOS_1);
    assert_true(response.decoded.suback.return_codes[2] == MQTT_SUBACK_FAILURE);
}

static void test_mqtt_pack_unsubscribe(void** state) {
    uint8_t buf[256];
    ssize_t rv;
    const uint8_t correct[] = {
        MQTT_CONTROL_UNSUBSCRIBE << 4 | 2u, 20,
        0, 132u,
        0, 3, 'a', '/', 'b',
        0, 5, 'b', 'b', 'b', '/', 'x',
        0, 4, 'c', '/', 'd', 'd',
    };

    rv = mqtt_pack_unsubscribe_request(buf, 256, 132, "a/b", "bbb/x", "c/dd", NULL);
    assert_true(rv == 22);
    assert_true(memcmp(buf, correct, sizeof(correct)) == 0);
}

static void test_mqtt_unpack_unsuback(void** state) {
    uint8_t buf[] = {
        MQTT_CONTROL_UNSUBACK << 4, 2,
        0, 213u
    };
    ssize_t rv;
    struct mqtt_response response;

    rv = mqtt_unpack_fixed_header(&response, buf, 4);
    assert_true(rv == 2);
    assert_true(response.fixed_header.control_type == MQTT_CONTROL_UNSUBACK);
    rv = mqtt_unpack_unsuback_response(&response, buf + 2);
    assert_true(rv == 2);
    assert_true(response.decoded.unsuback.packet_id == 213u);
}

static void test_mqtt_pack_disconnect(void** state) {
    uint8_t buf[2];
    assert_true(mqtt_pack_disconnect(buf, 2) == 2);   
}

static void test_mqtt_pack_ping(void** state) {
    uint8_t buf[2];
    struct mqtt_response response;
    struct mqtt_fixed_header *fixed_header = &response.fixed_header;
    assert_true(mqtt_pack_ping_request(buf, 2) == 2);   
    assert_true(mqtt_unpack_fixed_header(&response, buf, 2) == 2);
    assert_true(fixed_header->control_type == MQTT_CONTROL_PINGREQ);
    assert_true(fixed_header->remaining_length == 0);
}

static void test_mqtt_connect_and_ping(void** state) {
    uint8_t buf[256];
    const char* addr = "test.mosquitto.org";
    const char* port = "1883";
    struct addrinfo hints = {0};
    struct sockaddr_storage sockaddr;
    struct mqtt_client client;
    ssize_t rv;
    struct mqtt_response mqtt_response;

    hints.ai_family = AF_INET;         /* use IPv4 */
    hints.ai_socktype = SOCK_STREAM;    /* TCP */
    client.socketfd = conf_client(addr, port, &hints, &sockaddr);
    assert_true(client.socketfd != -1);

    rv = mqtt_pack_connection_request(buf, sizeof(buf), "this-is-me", NULL, NULL, NULL, NULL, 0, 30);
    assert_true(rv > 0);
    assert_true(send(client.socketfd, buf, rv, 0) != -1);

    /* receive connack */
    assert_true(recv(client.socketfd, buf, sizeof(buf), 0) != -1);
    rv = mqtt_unpack_fixed_header(&mqtt_response, buf, sizeof(buf));
    assert_true(rv > 0);
    assert_true(mqtt_unpack_connack_response(&mqtt_response, buf + rv) > 0);
    assert_true(mqtt_response.decoded.connack.return_code == MQTT_CONNACK_ACCEPTED);

    /* send ping request */
    rv = mqtt_pack_ping_request(buf, sizeof(buf));
    assert_true(rv > 0);
    assert_true(send(client.socketfd, buf, rv, 0) != -1);

    /* receive ping response */
    assert_true(recv(client.socketfd, buf, sizeof(buf), 0) != -1);
    rv = mqtt_unpack_fixed_header(&mqtt_response, buf, sizeof(buf));
    assert_true(rv > 0);
    assert_true(mqtt_response.fixed_header.control_type == MQTT_CONTROL_PINGRESP);

    /* disconnect */
    rv = mqtt_pack_disconnect(buf, sizeof(buf));
    assert_true(rv > 0);
    assert_true(send(client.socketfd, buf, rv, 0) != -1);

    /*close the socket */
    close(client.socketfd);
}

static void test_message_queue(void **unused) {
    char mem[32 + 4*sizeof(struct mqtt_queued_message)];
    ssize_t tmp;

    struct mqtt_message_queue mq;
    mqtt_message_queue_init(&mq, mem, sizeof(mem));
    assert_true(mq.mem_start == (void*) mem);
    assert_true(mq.curr == (uint8_t*)mem);
    assert_true(mq.mem_end == (void*) mem + sizeof(mem));
    assert_true((void*)mq.queue_next == mq.mem_end - sizeof(struct mqtt_queued_message));
    assert_true(mq.curr_sz == (void*) mq.queue_next - mq.mem_start - (ssize_t) sizeof(struct mqtt_queued_message));
    tmp = mq.curr_sz;
    assert_true(mqtt_message_queue_length(&mq) == 0);

    /* register a message */
    memset(mq.curr, 'a', 16);
    mqtt_message_queue_register(&mq, MQTT_CONTROL_CONNECT, 123, 16);
    assert_true(mq.curr == mq.mem_start + 16);
    assert_true(mq.curr_sz == tmp - 16 - (ssize_t) sizeof(struct mqtt_queued_message));
    assert_true((void*)mq.queue_next == mq.mem_end - 2*sizeof(struct mqtt_queued_message));
    assert_true(mqtt_message_queue_length(&mq) == 1);
    
    /* check that indexing is correct */
    assert_true(mqtt_message_queue_item(&mq, 0)->packet_id == 123);
    assert_true(mqtt_message_queue_item(&mq, 0)->size == 16);
    assert_true(mqtt_message_queue_item(&mq, 0)->control_type == MQTT_CONTROL_CONNECT);
    assert_true(mqtt_message_queue_item(&mq, 0)->start == (uint8_t*) mq.mem_start);
    assert_true(mqtt_message_queue_item(&mq, 0)->state == MQTT_QUEUED_UNSENT);

    memset(mq.curr, 'b', 8);
    mqtt_message_queue_register(&mq, MQTT_CONTROL_CONNECT, 234, 8);
    assert_true(mqtt_message_queue_item(&mq, 0)->packet_id == 123);
    assert_true(mqtt_message_queue_item(&mq, 0)->size == 16);
    assert_true(mqtt_message_queue_item(&mq, 0)->control_type == MQTT_CONTROL_CONNECT);
    assert_true(mqtt_message_queue_item(&mq, 0)->start == (uint8_t*) mq.mem_start);
    assert_true(mqtt_message_queue_item(&mq, 0)->state == MQTT_QUEUED_UNSENT);


    assert_true(mqtt_message_queue_item(&mq, 1)->packet_id == 234);
    assert_true(mqtt_message_queue_item(&mq, 1)->size == 8);
    assert_true(mqtt_message_queue_item(&mq, 1)->control_type == MQTT_CONTROL_CONNECT);
    assert_true(mqtt_message_queue_item(&mq, 1)->start == (uint8_t*) mq.mem_start + 16);
    assert_true(mqtt_message_queue_item(&mq, 1)->state == MQTT_QUEUED_UNSENT);

    /* try to clean the first */
    mqtt_message_queue_item(&mq, 0)->state = MQTT_QUEUED_COMPLETE;
    assert_true(mqtt_message_queue_length(&mq) == 2);
    for(int i = 0; i < 16; ++i) {
        assert_true(mem[i] == 'a');
    }
    mqtt_message_queue_clean(&mq);
    assert_true(mqtt_message_queue_length(&mq) == 1);
    for(int i = 0; i < 8; ++i) {
        assert_true(mem[i] == 'b');
    }
    assert_true(mq.curr == (uint8_t*) mem + 8);
    assert_true(mq.curr_sz == sizeof(mem) - 8 - 3*sizeof(struct mqtt_queued_message));
    assert_true(mq.queue_next == (struct mqtt_queued_message*) mq.mem_end - 2);

    /* check that queued_message is correct */
    assert_true(mqtt_message_queue_item(&mq, 0)->packet_id == 234);
    assert_true(mqtt_message_queue_item(&mq, 0)->size == 8);
    assert_true(mqtt_message_queue_item(&mq, 0)->control_type == MQTT_CONTROL_CONNECT);
    assert_true(mqtt_message_queue_item(&mq, 0)->start == (uint8_t*) mq.mem_start + 16);
    assert_true(mqtt_message_queue_item(&mq, 0)->state == MQTT_QUEUED_UNSENT);

    /* entirely clear, repeatedly */
    mqtt_message_queue_item(&mq, 0)->state = MQTT_QUEUED_COMPLETE;
    mqtt_message_queue_clean(&mq);
    assert_true(mqtt_message_queue_length(&mq) == 0);
    assert_true(mq.mem_start == (void*) mem);
    assert_true(mq.curr == (uint8_t*)mem);
    assert_true(mq.mem_end == (void*) mem + sizeof(mem));
    assert_true((void*)mq.queue_next == mq.mem_end - sizeof(struct mqtt_queued_message));
    assert_true(mq.curr_sz == (void*) mq.queue_next - mq.mem_start - (ssize_t) sizeof(struct mqtt_queued_message));
    mqtt_message_queue_clean(&mq);
    assert_true(mqtt_message_queue_length(&mq) == 0);
    assert_true(mq.mem_start == (void*) mem);
    assert_true(mq.curr == (uint8_t*)mem);
    assert_true(mq.mem_end == (void*) mem + sizeof(mem));
    assert_true((void*)mq.queue_next == mq.mem_end - sizeof(struct mqtt_queued_message));
    assert_true(mq.curr_sz == (void*) mq.queue_next - mq.mem_start - (ssize_t) sizeof(struct mqtt_queued_message));
    mqtt_message_queue_clean(&mq);
    assert_true(mqtt_message_queue_length(&mq) == 0);
    assert_true(mq.mem_start == (void*) mem);
    assert_true(mq.curr == (uint8_t*)mem);
    assert_true(mq.mem_end == (void*) mem + sizeof(mem));
    assert_true((void*)mq.queue_next == mq.mem_end - sizeof(struct mqtt_queued_message));
    assert_true(mq.curr_sz == (void*) mq.queue_next - mq.mem_start - (ssize_t) sizeof(struct mqtt_queued_message));

    /* fill up perfectly */
    assert_true(mq.curr_sz == 32 + 2*sizeof(struct mqtt_queued_message));
    memset(mq.curr, 'a', 16);
    mqtt_message_queue_register(&mq, MQTT_CONTROL_PINGREQ, 100, 16);
    assert_true(mq.curr_sz == 16 + 1*sizeof(struct mqtt_queued_message));
    memset(mq.curr, 'b', 8);
    mqtt_message_queue_register(&mq, MQTT_CONTROL_PUBACK, 200, 8);
    assert_true(mq.curr_sz == 8);
    memset(mq.curr, 'c', 8);
    mqtt_message_queue_register(&mq, MQTT_CONTROL_CONNECT, 300, 8);
    assert_true(mq.curr_sz == 0);
    assert_true(mqtt_message_queue_length(&mq) == 3);

    mqtt_message_queue_item(&mq, 1)->state = MQTT_QUEUED_COMPLETE;
    mqtt_message_queue_item(&mq, 2)->state = MQTT_QUEUED_COMPLETE;
    mqtt_message_queue_clean(&mq);
    assert_true(mqtt_message_queue_length(&mq) == 3);
    mqtt_message_queue_item(&mq, 2)->state = MQTT_QUEUED_AWAITING_ACK;
    mqtt_message_queue_item(&mq, 0)->state = MQTT_QUEUED_COMPLETE;
    mqtt_message_queue_clean(&mq);
    assert_true(mqtt_message_queue_length(&mq) == 1);
    for(int i = 0; i < 8; ++i) {
        assert_true(mem[i] == 'c');
    }
    mqtt_message_queue_item(&mq, 0)->state = MQTT_QUEUED_COMPLETE;
    mqtt_message_queue_clean(&mq);
    assert_true(mqtt_message_queue_length(&mq) == 0);
    assert_true(mq.mem_start == (void*) mem);
    assert_true(mq.curr == (uint8_t*)mem);
    assert_true(mq.mem_end == (void*) mem + sizeof(mem));
    assert_true((void*)mq.queue_next == mq.mem_end - sizeof(struct mqtt_queued_message));
    assert_true(mq.curr_sz == (void*) mq.queue_next - mq.mem_start - (ssize_t) sizeof(struct mqtt_queued_message));

    /**/
    memset(mq.curr, 'x', 16);
    mqtt_message_queue_register(&mq, MQTT_CONTROL_PINGREQ, 123, 16);
    printf("%ld\n", mq.curr_sz);
    memset(mq.curr, 'y', mq.curr_sz);
    mqtt_message_queue_register(&mq, MQTT_CONTROL_PUBACK, 234, mq.curr_sz);
    for(int i = 0; i < mqtt_message_queue_item(&mq, 0)->size; ++i) {
        assert_true(mqtt_message_queue_item(&mq, 0)->start[i] == 'x');
    }
    for(int i = 0; i < mqtt_message_queue_item(&mq, 1)->size; ++i) {
        assert_true(mqtt_message_queue_item(&mq, 1)->start[i] == 'y');
    }
    

}

int main(void)
{
    const struct CMUnitTest tests[] = {
        /*cmocka_unit_test(test_mqtt_fixed_header),
        cmocka_unit_test(test_mqtt_pack_connection_request),
        cmocka_unit_test(test_mqtt_unpack_connection_response),
        cmocka_unit_test(test_mqtt_pack_disconnect),
        cmocka_unit_test(test_mosquitto_connect_disconnect),
        cmocka_unit_test(test_mqtt_pack_publish),
        cmocka_unit_test(test_mqtt_pubxxx),
        cmocka_unit_test(test_mqtt_pack_subscribe),
        cmocka_unit_test(test_mqtt_unpack_suback),
        cmocka_unit_test(test_mqtt_pack_unsubscribe),
        cmocka_unit_test(test_mqtt_unpack_unsuback),
        cmocka_unit_test(test_mqtt_pack_ping),
        cmocka_unit_test(test_mqtt_connect_and_ping),*/
        cmocka_unit_test(test_message_queue),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}