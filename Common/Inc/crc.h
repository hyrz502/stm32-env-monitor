#ifndef CRC_H
#define CRC_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>
#include <stddef.h>

uint16_t crc_modbus(const uint8_t*data,uint32_t len);
uint32_t crc_calc(const uint8_t* data,uint32_t len);


#ifdef __cplusplus
}
#endif

#endif
