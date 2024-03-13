//
// Created by RUPC on 2024/3/10.
//

#ifndef UV_K5_FIRMWARE_CHINESE_4732_H
#define UV_K5_FIRMWARE_CHINESE_4732_H
#include <stdbool.h>
#include <stdint.h>
#include "driver/bk1080-regs.h"
#include "functions.h"
#include "stdbool.h"

#ifdef ENABLE_DOPPLER

#include "app/doppler.h"
#include "bsp/dp32g030/rtc.h"

#endif

#include "app/spectrum.h"
#include "am_fix.h"
#include "audio.h"
#include "misc.h"


//#define ENABLE_DOPPLER
#ifdef ENABLE_SCAN_RANGES
#include "chFrScanner.h"
#endif

#include "driver/eeprom.h"

#include "driver/backlight.h"
#include "frequencies.h"
#include "ui/helper.h"
#include "ui/main.h"
#define SI4732_DELAY_MS 200
#define RST_HIGH       GPIO_ClearBit(&GPIOB->DATA, GPIOB_PIN_BK1080)
#define RST_LOW         GPIO_SetBit(&GPIOB->DATA, GPIOB_PIN_BK1080)
void SI4732_WriteBuffer(uint8_t *buff,uint8_t size) ;
void SI4732_ReadBuffer(uint8_t *Value,uint8_t size) ;
void SI4732_Init();
void SI4732_Main() ;
void SI4732_WAIT_STATUS(uint8_t state);
static void Key_FM(KeyboardState kbd) ;
static void HandleUserInput() ;

enum SI4732_MODE_T{
    FM,AM
};
////“Ù¡ø
//    uint8_t cmd_vol[7] = {0x22, 0x12, 0x00, 0x40, 0x00, 0x00, 0x3f};
//    if (SI4732_WriteBuffer(cmd_vol, 7) < 0)
//        show_uint32(8888, 0);
//    uint8_t b = SI4732_Read(1);
//    while (b != 128) {
//        b = SI4732_Read(1);
//        show_uint32((uint32_t) b, 0);
//    }
//    SYSTEM_DelayMs(100);

////…˘µ¿
//    uint8_t cmd_hear[7] = {0x22, 0x12, 0x00, 0x40, 0x01, 0x00, 0x00};
//    SI4732_WriteBuffer(cmd_hear, 7);
//    uint8_t b = SI4732_Read(1);
//    while (b != 128) {
//        b = SI4732_Read(1);
//        show_uint32((uint32_t) b, 0);
//    }
//    SYSTEM_DelayMs(100);

#endif //UV_K5_FIRMWARE_CHINESE_4732_H
