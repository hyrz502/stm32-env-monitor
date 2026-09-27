#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>
#include "err.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct 
{
    uint8_t *buf;
    uint32_t size;
    uint32_t mask;
    volatile uint32_t head;
    volatile uint32_t tail;
}ring_buffer_t;

err_t rb_init(ring_buffer_t *rb,uint8_t*storage,uint32_t size);
err_t rb_write(ring_buffer_t *rb,uint8_t Byte);
err_t rb_write_block(ring_buffer_t* rb,uint8_t *buf,uint32_t len);
err_t rb_read(ring_buffer_t* rb,uint8_t* out);
uint32_t rb_read_block(ring_buffer_t* rb,uint8_t* out,uint32_t len);
uint32_t rb_available(ring_buffer_t* rb);
uint32_t rb_free(ring_buffer_t* rb);
void rb_reset(ring_buffer_t* rb);

#ifdef __cplusplus
}
#endif
#endif
