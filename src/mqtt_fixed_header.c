#include <mqtt.h>

ssize_t mqtt_unpack_fixed_header(struct mqtt_fixed_header *fixed_header, const uint8_t *buf, size_t bufsz) {
    const uint8_t *start = buf;
    
    /* check for null pointers or empty buffer */
    if (fixed_header == NULL || buf == NULL || bufsz == 0) {
        return -1;
    }

    /* parse control type and flags */
    fixed_header->control_type  = *buf >> 4;
    fixed_header->control_flags = *buf & 0x0F;

    /* parse remaining size */
    fixed_header->remaining_length = 0;

    int lshift = 0;
    do {
        /* consume byte and assert at least 1 byte left */
        --bufsz;
        ++buf;
        if (bufsz == 0) return -1;

        /* parse next byte*/
        fixed_header->remaining_length += (*buf & 0x7F) << lshift;
        lshift += 7;
    } while(*buf & 0x80); /* while continue bit is set */ 

    /* consume last byte */
    --bufsz;
    ++buf;

    /* return how many bytes were consumed */
    return buf - start;
}

ssize_t mqtt_pack_fixed_header(uint8_t *buf, size_t bufsz, const struct mqtt_fixed_header *fixed_header) {
    const uint8_t *start = buf;
    
    /* check for null pointers or empty buffer */
    if (fixed_header == NULL || buf == NULL || bufsz == 0) {
        return -1;
    }

    /* pack control type and flags */
    *buf =  (((uint8_t) fixed_header->control_type) << 4) & 0xF0;
    *buf |= ((uint8_t) fixed_header->control_flags)       & 0x0F;

    uint32_t remaining_length = fixed_header->remaining_length;
    do {
        /* consume byte and assert at least 1 byte left */
        --bufsz;
        ++buf;
        if (bufsz == 0) return -1;
        
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