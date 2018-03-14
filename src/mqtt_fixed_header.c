#include <mqtt.h>
#include <mqtt_rules.h>

struct {
    const uint8_t type_is_valid[16];
    const uint8_t mandatory_flags[16];
    const uint8_t mandatory_flags_mask[16];
} mqtt_control_rules = {
    {   /* boolean value, true if type is valid */
        0x00, /* MQTT_CONTROL_RESERVED */
        0x01, /* MQTT_CONTROL_CONNECT */
        0x01, /* MQTT_CONTROL_CONNACK */
        0x01, /* MQTT_CONTROL_PUBLISH */
        0x01, /* MQTT_CONTROL_PUBACK */
        0x01, /* MQTT_CONTROL_PUBREC */
        0x01, /* MQTT_CONTROL_PUBREL */
        0x01, /* MQTT_CONTROL_PUBCOMP */
        0x01, /* MQTT_CONTROL_SUBSCRIBE */
        0x01, /* MQTT_CONTROL_SUBACK */
        0x01, /* MQTT_CONTROL_UNSUBSCRIBE */
        0x01, /* MQTT_CONTROL_UNSUBACK */
        0x01, /* MQTT_CONTROL_PINGREQ */
        0x01, /* MQTT_CONTROL_PINGRESP */
        0x01, /* MQTT_CONTROL_DISCONNECT */
        0x00  /* MQTT_CONTROL_RESERVED */
    },
    {   /* flags that must be set for the associated control type */
        0x00, /* MQTT_CONTROL_RESERVED */
        0x00, /* MQTT_CONTROL_CONNECT */
        0x00, /* MQTT_CONTROL_CONNACK */
        0x00, /* MQTT_CONTROL_PUBLISH */
        0x00, /* MQTT_CONTROL_PUBACK */
        0x00, /* MQTT_CONTROL_PUBREC */
        0x02, /* MQTT_CONTROL_PUBREL */
        0x00, /* MQTT_CONTROL_PUBCOMP */
        0x02, /* MQTT_CONTROL_SUBSCRIBE */
        0x00, /* MQTT_CONTROL_SUBACK */
        0x02, /* MQTT_CONTROL_UNSUBSCRIBE */
        0x00, /* MQTT_CONTROL_UNSUBACK */
        0x00, /* MQTT_CONTROL_PINGREQ */
        0x00, /* MQTT_CONTROL_PINGRESP */
        0x00, /* MQTT_CONTROL_DISCONNECT */
        0x00  /* MQTT_CONTROL_RESERVED */
    },
    {   /* mask of flags that must be specific values for the associated control type*/
        0x00, /* MQTT_CONTROL_RESERVED */
        0x0F, /* MQTT_CONTROL_CONNECT */
        0x0F, /* MQTT_CONTROL_CONNACK */
        0x00, /* MQTT_CONTROL_PUBLISH */
        0x0F, /* MQTT_CONTROL_PUBACK */
        0x0F, /* MQTT_CONTROL_PUBREC */
        0x0F, /* MQTT_CONTROL_PUBREL */
        0x0F, /* MQTT_CONTROL_PUBCOMP */
        0x0F, /* MQTT_CONTROL_SUBSCRIBE */
        0x0F, /* MQTT_CONTROL_SUBACK */
        0x0F, /* MQTT_CONTROL_UNSUBSCRIBE */
        0x0F, /* MQTT_CONTROL_UNSUBACK */
        0x0F, /* MQTT_CONTROL_PINGREQ */
        0x0F, /* MQTT_CONTROL_PINGRESP */
        0x0F, /* MQTT_CONTROL_DISCONNECT */
        0x00  /* MQTT_CONTROL_RESERVED */
    }
};

ssize_t mqtt_validate_fixed_header(const struct mqtt_fixed_header *fixed_header) {
    uint8_t control_type;
    uint8_t control_flags;
    uint8_t mandatory_flags;
    uint8_t mandatory_flags_mask;

    /* get value and rules */
    control_type = fixed_header->control_type;
    control_flags = fixed_header->control_flags;
    mandatory_flags = mqtt_control_rules.mandatory_flags[control_type];
    mandatory_flags_mask = mqtt_control_rules.mandatory_flags_mask[control_type];

    /* check for valid type */
    if (!mqtt_control_rules.type_is_valid[control_type]) {
        return MQTT_ERROR_CONTROL_FORBIDDEN_TYPE;
    }
    
    /* check that flags are appropriate */
    if(MQTT_BITFIELD_RULE_VIOLOATION(control_flags, mandatory_flags, mandatory_flags_mask)) {
        return MQTT_ERROR_CONTROL_INVALID_FLAGS;
    }

    return 0;
}

ssize_t mqtt_unpack_fixed_header(struct mqtt_fixed_header *fixed_header, const uint8_t *buf, size_t bufsz) {
    const uint8_t *start = buf;
    int lshift;
    ssize_t errcode;
    
    /* check for null pointers or empty buffer */
    if (fixed_header == NULL || buf == NULL) {
        return MQTT_ERROR_NULLPTR;
    }

    /* check that bufsz is not zero */
    if (bufsz == 0) return 0;

    /* parse control type and flags */
    fixed_header->control_type  = *buf >> 4;
    fixed_header->control_flags = *buf & 0x0F;

    /* parse remaining size */
    fixed_header->remaining_length = 0;

    lshift = 0;
    do {
        /* consume byte and assert at least 1 byte left */
        --bufsz;
        ++buf;
        if (bufsz == 0) return 0;

        /* parse next byte*/
        fixed_header->remaining_length += (*buf & 0x7F) << lshift;
        lshift += 7;
    } while(*buf & 0x80); /* while continue bit is set */ 

    /* consume last byte */
    --bufsz;
    ++buf;

    /* check that the fixed header is valid */
    errcode = mqtt_validate_fixed_header(fixed_header);
    if (errcode) {
        return errcode;
    }

    /* return how many bytes were consumed */
    return buf - start;
}

ssize_t mqtt_pack_fixed_header(uint8_t *buf, size_t bufsz, const struct mqtt_fixed_header *fixed_header) {
    const uint8_t *start = buf;
    ssize_t errcode;
    uint32_t remaining_length;
    
    /* check for null pointers or empty buffer */
    if (fixed_header == NULL || buf == NULL) {
        return MQTT_ERROR_NULLPTR;
    }

    /* check that the fixed header is valid */
    errcode = mqtt_validate_fixed_header(fixed_header);
    if (errcode) {
        return errcode;
    }

    /* check that bufsz is not zero */
    if (bufsz == 0) return 0;

    /* pack control type and flags */
    *buf =  (((uint8_t) fixed_header->control_type) << 4) & 0xF0;
    *buf |= ((uint8_t) fixed_header->control_flags)       & 0x0F;

    remaining_length = fixed_header->remaining_length;
    do {
        /* consume byte and assert at least 1 byte left */
        --bufsz;
        ++buf;
        if (bufsz == 0) return 0;
        
        /* pack next byte */
        *buf  = remaining_length & 0x7F;
        if(remaining_length > 127) *buf |= 0x80;
        remaining_length = remaining_length >> 7;
    } while(*buf & 0x80);
    
    /* consume last byte */
    --bufsz;
    ++buf;

    /* return how many bytes were consumed */
    return buf - start;
}