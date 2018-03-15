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
    uint8_t correct_buf[5];
    uint8_t buf[5];
    struct mqtt_fixed_header fixed_header;
    ssize_t rv;

    /* sanity check with valid fixed_header */
    correct_buf[0] = (MQTT_CONTROL_CONNECT << 4) | 0;
    correct_buf[1] = 193u;
    correct_buf[2] = 2u;

    /* check that unpack is correct */
    rv = mqtt_unpack_fixed_header(&fixed_header, correct_buf, sizeof(correct_buf));
    assert_true(rv == 3);
    assert_true(fixed_header.control_type == MQTT_CONTROL_CONNECT);
    assert_true(fixed_header.control_flags == 0);
    assert_true(fixed_header.remaining_length == 321);

    /* check that unpack is correct */
    rv = mqtt_pack_fixed_header(buf, sizeof(buf), &fixed_header);
    assert_true(rv == 3);
    assert_true(memcmp(correct_buf, buf, 3) == 0);


    /* check that invalid flags are caught */
    correct_buf[0] = (MQTT_CONTROL_CONNECT << 4) | 1;
    rv = mqtt_unpack_fixed_header(&fixed_header, correct_buf, sizeof(correct_buf));
    assert_true(rv == MQTT_ERROR_CONTROL_INVALID_FLAGS);

    /* check that valid flags are ok when there is a required bit */
    correct_buf[0] = (MQTT_CONTROL_PUBREL << 4) | 2;
    rv = mqtt_unpack_fixed_header(&fixed_header, correct_buf, sizeof(correct_buf));
    assert_true(rv == 3);

    /* check that invalid flags are ok when there is a required bit */
    correct_buf[0] = (MQTT_CONTROL_PUBREL << 4) | 3;
    rv = mqtt_unpack_fixed_header(&fixed_header, correct_buf, sizeof(correct_buf));
    assert_true(rv == MQTT_ERROR_CONTROL_INVALID_FLAGS);

    /* check that valid flags are ok when there are optional flags */
    correct_buf[0] = (MQTT_CONTROL_PUBLISH << 4) | 0xF;
    rv = mqtt_unpack_fixed_header(&fixed_header, correct_buf, sizeof(correct_buf));
    assert_true(rv == 3);
    
    correct_buf[0] = (MQTT_CONTROL_PUBLISH << 4) | 3;
    rv = mqtt_unpack_fixed_header(&fixed_header, correct_buf, sizeof(correct_buf));
    assert_true(rv == 3);


    /* check that remaining length is packed/unpacked correctly */
    correct_buf[0] = (MQTT_CONTROL_CONNECT << 4) | 0;
    correct_buf[1] = 64;

    /* check that unpack is correct */
    rv = mqtt_unpack_fixed_header(&fixed_header, correct_buf, sizeof(correct_buf));
    assert_true(rv == 2);
    assert_true(fixed_header.control_type == MQTT_CONTROL_CONNECT);
    assert_true(fixed_header.control_flags == 0);
    assert_true(fixed_header.remaining_length == 64);

    /* check that unpack is correct */
    rv = mqtt_pack_fixed_header(buf, sizeof(buf), &fixed_header);
    assert_true(rv == 2);
    assert_true(memcmp(correct_buf, buf, 2) == 0);


    /* check that remaining length is packed/unpacked correctly */
    correct_buf[0] = (MQTT_CONTROL_CONNECT << 4) | 0;
    correct_buf[1] = 127;

    /* check that unpack is correct */
    rv = mqtt_unpack_fixed_header(&fixed_header, correct_buf, sizeof(correct_buf));
    assert_true(rv == 2);
    assert_true(fixed_header.control_type == MQTT_CONTROL_CONNECT);
    assert_true(fixed_header.control_flags == 0);
    assert_true(fixed_header.remaining_length == 127);

    /* check that unpack is correct */
    rv = mqtt_pack_fixed_header(buf, sizeof(buf), &fixed_header);
    assert_true(rv == 2);
    assert_true(memcmp(correct_buf, buf, 2) == 0);


    /* check that remaining length is packed/unpacked correctly */
    correct_buf[0] = (MQTT_CONTROL_CONNECT << 4) | 0;
    correct_buf[1] = 128;
    correct_buf[2] = 1;

    /* check that unpack is correct */
    rv = mqtt_unpack_fixed_header(&fixed_header, correct_buf, sizeof(correct_buf));
    assert_true(rv == 3);
    assert_true(fixed_header.control_type == MQTT_CONTROL_CONNECT);
    assert_true(fixed_header.control_flags == 0);
    assert_true(fixed_header.remaining_length == 128);

    /* check that unpack is correct */
    rv = mqtt_pack_fixed_header(buf, sizeof(buf), &fixed_header);
    assert_true(rv == 3);
    assert_true(memcmp(correct_buf, buf, 3) == 0);

    /* check bad inputs */
    assert_true( mqtt_pack_fixed_header(NULL, 5, &fixed_header) == MQTT_ERROR_NULLPTR );
    assert_true( mqtt_pack_fixed_header(buf, 5, NULL) == MQTT_ERROR_NULLPTR );
    assert_true( mqtt_pack_fixed_header(buf, 2, &fixed_header) == 0 );

    assert_true( mqtt_unpack_fixed_header(NULL, buf, 5) == MQTT_ERROR_NULLPTR );
    assert_true( mqtt_unpack_fixed_header(&fixed_header, NULL, 5) == MQTT_ERROR_NULLPTR );
    assert_true( mqtt_unpack_fixed_header(&fixed_header, buf, 2) == 0 );
}

static void test_mqtt_pack_connection_request(void** state) {
    uint8_t buf[256];
    ssize_t rv;
    const uint8_t correct_bytes[] = {
        (MQTT_CONTROL_DISCONNECT << 4) | 0, 16,
        0, 4, 'M', 'Q', 'T', 'T', MQTT_PROTOCOL_LEVEL, 0, 120u, 
        0, 4, 'l', 'i', 'a', 'm'
    };
    struct mqtt_connection_request request = {0};
    struct mqtt_fixed_header fixed_header;
    request.client_id = "liam";
    request.keep_alive = 120;
    
    rv = mqtt_pack_connection_request(buf, sizeof(buf), &request);
    assert_true(rv == 18);

    /* check that fixed header is correct */
    rv = mqtt_unpack_fixed_header(&fixed_header, buf, rv);
    assert_true(fixed_header.control_type == MQTT_CONTROL_CONNECT);
    assert_true(fixed_header.remaining_length == 16);

    /* check that memory is correct */
    assert_true(memcmp(correct_bytes, buf, sizeof(correct_bytes)));
}

static void test_mosquitto_connect_disconnect(void** state) {
    uint8_t buf[256];
    const char* addr = "test.mosquitto.org";
    const char* port = "1883";
    struct addrinfo hints = {0};
    struct sockaddr_storage sockaddr;
    struct mqtt_client client;
    ssize_t rv;
    struct mqtt_connection_request request = {0};
    struct mqtt_fixed_header fixed_header;
    struct mqtt_connection_response response;

    hints.ai_family = AF_INET;         /* use IPv4 */
    hints.ai_socktype = SOCK_STREAM;    /* TCP */
    client.socketfd = conf_client(addr, port, &hints, &sockaddr);
    assert_true(client.socketfd != -1);

    /* send connection request */
    request.client_id = "liam-123456";
    request.keep_alive = 30;
    rv = mqtt_pack_connection_request(buf, sizeof(buf), &request);
    assert_true(rv > 0);
    assert_true(send(client.socketfd, buf, rv, 0) != -1);

    /* receive connack */
    assert_true(recv(client.socketfd, buf, sizeof(buf), 0) != -1);
    rv = mqtt_unpack_fixed_header(&fixed_header, buf, sizeof(buf));
    assert_true(rv > 0);
    assert_true(mqtt_unpack_connection_response(&response, &fixed_header, buf + rv, sizeof(buf)) > 0);
    assert_true(response.connect_return_code == MQTT_CONNACK_ACCEPTED);

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
    struct mqtt_fixed_header fixed_header;
    struct mqtt_connection_response response;
    ssize_t rv = mqtt_unpack_fixed_header(&fixed_header, buf, sizeof(buf));
    assert_true(rv == 2);
    assert_true(fixed_header.control_type == MQTT_CONTROL_CONNACK);

    /* unpack response */
    rv = mqtt_unpack_connection_response(&response, &fixed_header, buf+2, sizeof(buf)-2);
    assert_true(rv == 2);
    assert_true(response.session_present_flag == 0);
    assert_true(response.connect_return_code == MQTT_CONNACK_ACCEPTED);
}

static void test_mqtt_pack_disconnect(void** state) {
    uint8_t buf[2];
    assert_true(mqtt_pack_disconnect(buf, 2) == 2);   
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_mqtt_fixed_header),
        cmocka_unit_test(test_mqtt_pack_connection_request),
        cmocka_unit_test(test_mqtt_unpack_connection_response),
        cmocka_unit_test(test_mqtt_pack_disconnect),
        cmocka_unit_test(test_mosquitto_connect_disconnect),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}