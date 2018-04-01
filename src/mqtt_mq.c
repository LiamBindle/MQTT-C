#include <mqtt.h>


void mqtt_mq_init(struct mqtt_message_queue *mq, void *buf, size_t bufsz) 
{
    mq->mem_start = buf;
    mq->mem_end = buf + bufsz;
    mq->curr = buf;
    mq->queue_tail = mq->mem_end;
    mq->curr_sz = mqtt_mq_currsz(mq);
}

struct mqtt_queued_message* mqtt_mq_register(struct mqtt_message_queue *mq, size_t nbytes)
{
    /* make queued message header */
    --(mq->queue_tail);
    mq->queue_tail->start = mq->curr;
    mq->queue_tail->size = nbytes;
    mq->queue_tail->state = MQTT_QUEUED_UNSENT;

    /* move curr and recalculate curr_sz */
    mq->curr += nbytes;
    mq->curr_sz = mqtt_mq_currsz(mq);

    return mq->queue_tail;
}

void mqtt_mq_clean(struct mqtt_message_queue *mq) {
    struct mqtt_queued_message *new_head;

    for(new_head = mqtt_mq_get(mq, 0); new_head >= mq->queue_tail; --new_head) {
        if (new_head->state != MQTT_QUEUED_COMPLETE) break;
    }
    
    /* check if everything can be removed */
    if (new_head < mq->queue_tail) {
        mq->curr = mq->mem_start;
        mq->queue_tail = mq->mem_end;
        mq->curr_sz = mqtt_mq_currsz(mq);
        return;
    } else if (new_head == mqtt_mq_get(mq, 0)) {
        /* do nothing */
        return;
    }

    /* move buffered data */
    size_t n = mq->curr - new_head->start;
    size_t removing = new_head->start - (uint8_t*) mq->mem_start;
    memmove(mq->mem_start, new_head->start, n);
    mq->curr = mq->mem_start + n;

    /* move queue */
    ssize_t new_tail_idx = new_head - mq->queue_tail;
    memmove(mqtt_mq_get(mq, new_tail_idx), mq->queue_tail, sizeof(struct mqtt_queued_message) * (new_tail_idx + 1));
    mq->queue_tail = mqtt_mq_get(mq, new_tail_idx);

    /* bump back start's */
    for(ssize_t i = 0; i < new_tail_idx + 1; ++i) {
        mqtt_mq_get(mq, i)->start -= removing;
    }

    /* get curr_sz */
    mq->curr_sz = mqtt_mq_currsz(mq);
}
