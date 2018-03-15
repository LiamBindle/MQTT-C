#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <setjmp.h>
#include <cmocka.h>
#include <arpa/inet.h> //< htons, ntohs

#include <mqtt.h>

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
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}