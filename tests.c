#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <setjmp.h>
#include <cmocka.h>

#include <mqtt.h>

static void test_mqtt_fixed_header(void** state) {
    const uint8_t sample_fixed_header[] = {
        (11u << 4) | 6u, 193u, 2
    };
    uint8_t buf[3];
    struct mqtt_fixed_header fixed_header;

    /* sanity check */
    ssize_t rv = mqtt_unpack_fixed_header(&fixed_header, sample_fixed_header, 3);
    assert_true(rv == 3);
    assert_true(fixed_header.control_type == MQTT_CONTROL_UNSUBACK);
    assert_true(fixed_header.control_flags == 6);
    assert_true(fixed_header.remaining_length == 321);

    rv = mqtt_pack_fixed_header(buf, 3, &fixed_header);
    assert_true(rv == 3);
    assert_true(memcmp(buf, sample_fixed_header, 3) == 0);

    uint8_t test2[] = {
        (MQTT_CONTROL_SUBACK << 4) | 7u, 64, 0
    };

    rv = mqtt_unpack_fixed_header(&fixed_header, test2, 3);
    assert_true(rv == 2);
    assert_true(fixed_header.control_type == MQTT_CONTROL_SUBACK);
    assert_true(fixed_header.control_flags == 7);
    assert_true(fixed_header.remaining_length == 64);

    rv = mqtt_pack_fixed_header(buf, 3, &fixed_header);
    assert_true(rv == 2);
    assert_true(memcmp(buf, test2, 2) == 0);

    test2[1] = 127;

    rv = mqtt_unpack_fixed_header(&fixed_header, test2, 3);
    assert_true(rv == 2);
    assert_true(fixed_header.control_type == MQTT_CONTROL_SUBACK);
    assert_true(fixed_header.control_flags == 7);
    assert_true(fixed_header.remaining_length == 127);

    rv = mqtt_pack_fixed_header(buf, 3, &fixed_header);
    assert_true(rv == 2);
    assert_true(memcmp(buf, test2, 2) == 0);

    test2[1] = 128;
    test2[2] = 1;

    rv = mqtt_unpack_fixed_header(&fixed_header, test2, 3);
    assert_true(rv == 3);
    assert_true(fixed_header.control_type == MQTT_CONTROL_SUBACK);
    assert_true(fixed_header.control_flags == 7);
    assert_true(fixed_header.remaining_length == 128);

    rv = mqtt_pack_fixed_header(buf, 3, &fixed_header);
    assert_true(rv == 3);
    assert_true(memcmp(buf, test2, 3) == 0);

    /* check errors */
    assert_true(mqtt_pack_fixed_header(NULL, 10, &fixed_header) == -1);
    assert_true(mqtt_pack_fixed_header(buf, 0, &fixed_header) == -1);
    assert_true(mqtt_pack_fixed_header(buf, 10, NULL) == -1);
    assert_true(mqtt_pack_fixed_header(buf, 2, &fixed_header) == -1);
    
    assert_true(mqtt_unpack_fixed_header(NULL, buf, 10) == -1);
    assert_true(mqtt_unpack_fixed_header(&fixed_header, NULL, 10) == -1);
    assert_true(mqtt_unpack_fixed_header(&fixed_header, buf, 0) == -1);
    assert_true(mqtt_unpack_fixed_header(&fixed_header, buf, 2) == -1);
    assert_true(mqtt_unpack_fixed_header(&fixed_header, buf, 3) == 3);
}

int main(void)
{
    const struct CMUnitTest tests[] =
    {
        cmocka_unit_test(test_mqtt_fixed_header),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}