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
    /** @brief The start of the allocated memory space */
    void *mem_start;

    /** @brief The end of the allocated memory space */
    void *mem_end;

    /** @brief The current position in the buffer to start packing at. */
    uint8_t *curr;

    /** @brief The number of bytes available (for packing) in the buffer. */
    ssize_t curr_sz;

    /** 
     * @brief A pointer to the struct mqtt_message_queue to store for the \em next message to 
     *        be queued in the buffer.
     * 
     * @note Queue elements are stored in reverse order. This means that the tail of the queue
     *       is stored at \c {queue_next + 1}.
     */
    struct mqtt_queued_message *queue_next;
};

void mqtt_message_queue_init(struct mqtt_message_queue *mq, void *buf, size_t bufsz);

void mqtt_message_queue_clean(struct mqtt_message_queue *mq);

void mqtt_message_queue_register(struct mqtt_message_queue *mq,
                                    enum MQTTControlPacketType control_type,
                                    uint16_t packet_id,
                                    size_t nbytes);


ssize_t mqtt_message_queue_length(struct mqtt_message_queue *mq);

struct mqtt_queued_message* mqtt_message_queue_item(struct mqtt_message_queue *mq, int index);
