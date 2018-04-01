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

/**
 * @brief Initialize a message queue.
 * 
 * @param[out] mq The message queue to initialize.
 * @param[in] buf The buffer for this message queue.
 * @param[in] bufsz The number of bytes in the buffer. 
 */
void mqtt_mq_init(struct mqtt_message_queue *mq, void *buf, size_t bufsz);

/**
 * @brief Clear as many messages from the front of the queue as possible.
 * 
 * @param mq The message queue.
 */
void mqtt_mq_clean(struct mqtt_message_queue *mq);

/**
 * @brief Register a message that was just added to the buffer.
 * 
 * @note This function should be called immediately following a call to a packer function
 *       that returned a positive value. The positive value (number of bytes packed) should
 *       be passed to this function.
 * 
 * @param mq The message queue.
 * @param[in] control_type The MQTTControlPacketType of the packed message.
 * @param[in] packet_id The packet id of the packet that was just packed (unused if not required).
 * @param[in] nbytes The number of bytes that were just packed.
 * 
 * @note This function will step mqtt_message_queue::curr and update mqtt_message_queue::curr_sz.
 */
void mqtt_mq_register(struct mqtt_message_queue *mq, 
                         enum MQTTControlPacketType control_type,
                         uint16_t packet_id,
                         size_t nbytes);

/**
 * @brief Returns the mqtt_queued_message at \p index.
 * 
 * @param mq_ptr A pointer to the message queue.
 * @param index The index of the message. 
 *
 * @returns The mqtt_queued_message at \p index.
 */
#define mqtt_mq_get(mq_ptr, index) (((struct mqtt_queued_message*) ((mq_ptr)->mem_end)) - 1 - index)

/**
 * @brief Returns the number of messages in the message queue, \p mq_ptr.
 */
#define mqtt_mq_length(mq_ptr) (((struct mqtt_queued_message*) ((mq_ptr)->mem_end)) - (mq_ptr)->queue_tail)

/**
 * @brief Used internally to recalculate the \c curr_sz.
 */
#define mqtt_mq_currsz(mq_ptr) (mq_ptr->curr >= (uint8_t*) ((mq_ptr)->queue_tail - 1)) ? 0 : ((uint8_t*) ((mq_ptr)->queue_tail - 1)) - (mq_ptr)->curr

