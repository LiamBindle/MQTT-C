#include <mqtt.h>

enum MQTTQueuedMessageState {
    MQTT_QUEUED_UNSENT,
    MQTT_QUEUED_AWAITING_ACK,
    MQTT_QUEUED_COMPLETE
};

struct mqtt_queued_message {
    uint8_t *start;
    size_t size;

    enum MQTTQueuedMessageState state;
    time_t timeout;

    enum MQTTControlPacketType control_type;
    uint16_t packet_id;
};

struct mqtt_message_queue {
    void *mem_start;
    void *mem_end;

    uint8_t *curr;
    size_t curr_sz;
    
    struct mqtt_queued_message *queue_tail;
};

void mqtt_mq_init(struct mqtt_message_queue *mq, void *buf, size_t bufsz);
void mqtt_mq_clean(struct mqtt_message_queue *mq);
void mqtt_mq_register(struct mqtt_message_queue *mq, 
                         enum MQTTControlPacketType control_type,
                         uint16_t packet_id,
                         size_t nbytes);
#define mqtt_mq_get(mq_ptr, index) (((struct mqtt_queued_message*) ((mq_ptr)->mem_end)) - 1 - index)
#define mqtt_mq_length(mq_ptr) (((struct mqtt_queued_message*) ((mq_ptr)->mem_end)) - (mq_ptr)->queue_tail)
#define mqtt_mq_currsz(mq_ptr) (mq_ptr->curr >= (uint8_t*) ((mq_ptr)->queue_tail - 1)) ? 0 : ((uint8_t*) ((mq_ptr)->queue_tail - 1)) - (mq_ptr)->curr

