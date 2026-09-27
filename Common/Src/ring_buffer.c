#include "ring_buffer.h"

static bool s_is_power_of_two(uint32_t v)
{
    return ((v!=0U)&&((v&(v-1U))==0U));
}

err_t rb_init(ring_buffer_t *rb,uint8_t*storage,uint32_t size)
{
    if(rb==NULL||storage==NULL||(!(s_is_power_of_two(size))))
    {
        return ERR_PARAM;
    }
    rb->buf=storage;
    rb->size=size;
    rb->mask=size-1;
    rb->head=0U;
    rb->tail=0U;
    return ERR_OK;
}

err_t rb_write(ring_buffer_t *rb,uint8_t Byte)
{
    if(rb==NULL)
    {
        return ERR_PARAM;
    }
    uint32_t next= (rb->head+1U) & rb->mask;
    if(next==rb->tail)
    {
        return ERR_NOMEM;
    }
    rb->buf[rb->head]=Byte;
    rb->head=next;
    return ERR_OK;
}
err_t rb_write_block(ring_buffer_t* rb,uint8_t *buf,uint32_t len)
{
    if(rb==NULL||buf==NULL)
    {
        return ERR_PARAM;
    }
    for(uint32_t i=0;i<len;i++)
    {
        if(rb_write(rb,buf[i])!=ERR_OK)
        {
            return ERR_NOMEM;
        }
    }
    return ERR_OK;
}
err_t rb_read(ring_buffer_t* rb,uint8_t* out)
{
    if(rb==NULL||out==NULL)
    {
        return ERR_PARAM;
    }
    if(rb->head==rb->tail)
    {
        return ERR_NOMEM;   /*缓冲区空*/
    }
    *out=rb->buf[rb->tail];
    rb->tail=(rb->tail+1U)&rb->mask;
    return ERR_OK;
}
uint32_t rb_read_block(ring_buffer_t* rb,uint8_t* out,uint32_t len)
{
    if(rb==NULL||out==NULL)
    {
        return 0U;
    }
    uint32_t cnt=0U;
    while(cnt<len&&rb_read(rb,out)==ERR_OK)
    {
        cnt++;
    }
    return cnt;
}
uint32_t rb_available(ring_buffer_t* rb)
{
    if(rb==NULL)
    {
        return 0U;
    }
    return (rb->head-rb->tail)&rb->mask;
}
uint32_t rb_free(ring_buffer_t* rb)
{
    if(rb==NULL)
    {
        return 0U;
    }
    return (rb->tail-rb->head-1U)&rb->mask;
}
void rb_reset(ring_buffer_t* rb)
{
    if(rb==NULL)
    {
        return;
    }
    rb->head=0U;
    rb->tail=0U;
}
