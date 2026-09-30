#include "app_data.h"

static ring_buffer_t s_rb;
ring_buffer_t* rb = &s_rb;

uint8_t shortage[BUFFER_MAXSIZE];
