//
// Created by RUPC on 2024/3/10.
//

#ifndef UV_K5_FIRMWARE_CHINESE_4732_H
#define UV_K5_FIRMWARE_CHINESE_4732_H
#include <stdbool.h>
#include <stdint.h>
#include "driver/bk1080-regs.h"


#define RST_HIGH       GPIO_ClearBit(&GPIOB->DATA, GPIOB_PIN_BK1080)
#define RST_LOW         GPIO_SetBit(&GPIOB->DATA, GPIOB_PIN_BK1080)
int SI4732_WriteBuffer(uint8_t *buff,uint8_t size) ;
uint8_t SI4732_Read(bool bFinal) ;
void SI4732_ReadBuffer(uint8_t *Value,uint8_t size) ;


#endif //UV_K5_FIRMWARE_CHINESE_4732_H
