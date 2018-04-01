#include <mqtt_mq.h>

void mqtt_message_queue_init(struct mqtt_message_queue *mq, void *buf, size_t bufsz) {
    /* initialize message queue */
    mq->mem_start = buf;
    mq->mem_end = buf + bufsz;
    mq->curr = (uint8_t *) buf;
    mq->queue_next = ((struct mqtt_queued_message*) mq->mem_end) - 1;
    mq->curr_sz = bufsz - 2*sizeof(struct mqtt_queued_message);
}

void mqtt_message_queue_clean(struct mqtt_message_queue *mq) {
    /* count how many MQTT_QUEUED_COMPLETE are at the start */
    struct mqtt_queued_message *tail = mq->queue_next + 1;
    struct mqtt_queued_message *head = ((struct mqtt_queued_message*) mq->mem_end) - 1;
    struct mqtt_queued_message *new_first;

    /* find first queued message that is not complete */
    for(new_first = head; new_first >= tail; --new_first) {
        if (new_first->state != MQTT_QUEUED_COMPLETE) {
            break;
        }
    }

    /* check if everything can be cleared */
    if (new_first < tail) {
        mq->curr = mq->mem_start;
        mq->queue_next = head;
        mq->curr_sz = (mq->mem_end - mq->mem_start) - 2*sizeof(struct mqtt_queued_message);
        return;
    }

    /* memmove all awaiting to mem_start */
    size_t n = mq->curr - new_first->start;
    memmove(mq->mem_start, new_first->start, n);
    mq->curr = mq->mem_start + n;

    /* memmove struct queued_messages */
    n = (new_first - mq->queue_next) * sizeof(struct mqtt_queued_message);

    memmove(mq->mem_end - n, tail, n);
    mq->queue_next =((struct mqtt_queued_message*) (mq->mem_end - n)) - 1;
    mq->curr_sz = (ssize_t) (((void*) mq->queue_next) - ((void*) mq->curr)) - sizeof(struct mqtt_queued_message); 
}

void mqtt_message_queue_register(struct mqtt_message_queue *mq,
                                    enum MQTTControlPacketType control_type,
                                    uint16_t packet_id,
                                    size_t nbytes) 
{
    mq->queue_next->start = mq->curr;
    mq->queue_next->size = nbytes;
    mq->queue_next->state = MQTT_QUEUED_UNSENT;
    mq->queue_next->packet_id = packet_id;
    mq->queue_next->control_type = control_type;

    mq->curr += nbytes;
    mq->curr_sz -= nbytes + sizeof(struct mqtt_queued_message);
    --(mq->queue_next);

    if (mq->curr_sz < 0) {
        mqtt_message_queue_clean(mq);
    }
}

ssize_t mqtt_message_queue_length(struct mqtt_message_queue *mq) {
    return (struct mqtt_queued_message*) mq->mem_end - mq->queue_next - 1;
}

struct mqtt_queued_message* mqtt_message_queue_item(struct mqtt_message_queue *mq, int index) {
    return ((struct mqtt_queued_message*) mq->mem_end) - 1 - index;
}