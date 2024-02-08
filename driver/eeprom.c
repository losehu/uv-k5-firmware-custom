/* Copyright 2023 Dual Tachyon
 * https://github.com/DualTachyon
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

#include <stddef.h>
#include <string.h>

#include "driver/eeprom.h"
#include "driver/i2c.h"
#include "driver/system.h"
#include "assert.h"

#ifdef ENABLE_FLASHLIGHT
#include "app/flashlight.h"
#endif

#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "app/action.h"

#ifdef ENABLE_AIRCOPY
#include "app/aircopy.h"
#endif

#include "app/app.h"
#include "app/chFrScanner.h"
#include "app/dtmf.h"
#include "driver/uart.h"

#ifdef ENABLE_FMRADIO
#include "app/fm.h"
#endif

#include "app/generic.h"
#include "app/main.h"
#include "app/menu.h"
#include "app/scanner.h"

#ifdef ENABLE_UART
#include "app/uart.h"
#endif

#include "ARMCM0.h"
#include "audio.h"
#include "board.h"
#include "bsp/dp32g030/gpio.h"
#include "driver/backlight.h"

#ifdef ENABLE_FMRADIO
#include "driver/bk1080.h"
#endif

#include "driver/bk4819.h"
#include "driver/gpio.h"
#include "driver/keyboard.h"
#include "driver/st7565.h"
#include "driver/system.h"
#include "am_fix.h"
//#include "external/printf/printf.h"
#include "frequencies.h"
#include "functions.h"
#include "helper/battery.h"
#include "misc.h"
#include "radio.h"
#include "settings.h"

#if defined(ENABLE_OVERLAY)
#include "sram-overlay.h"
#endif
#ifdef ENABLE_MESSENGER
#include "app/messenger.h"
#endif
#ifdef ENABLE_DOPPLER
#include "app/doppler.h"
#endif

#include "ui/battery.h"
#include "ui/inputbox.h"
#include "ui/main.h"
#include "ui/menu.h"
#include "ui/status.h"
#include "ui/ui.h"

#ifdef ENABLE_MESSENGER_NOTIFICATION
bool gPlayMSGRing = false;
uint8_t gPlayMSGRingCount = 0;
#endif

void EEPROM_ReadBuffer(uint32_t Address, void *pBuffer, uint8_t Size) {

    __disable_irq();
    I2C_Start();

    uint8_t IIC_ADD = 0xA0 | ((Address / 0x10000) << 1);
#if ENABLE_EEPROM_TYPE == 1
    if (Address >= 0x40000)
        {IIC_ADD = 0xA8 | (((Address - 0x40000) / 0x10000) << 1);
Address-=0x40000;

        }
#elif ENABLE_EEPROM_4M == 2
    if (Address >= 0x20000)
        {IIC_ADD = 0xA4 | (((Address - 0x20000) / 0x10000) << 1);
Address-=0x20000;
        }
#endif
    I2C_Write(IIC_ADD);
    I2C_Write((Address >> 8) & 0xFF);
    I2C_Write((Address >> 0) & 0xFF);

    I2C_Start();

    I2C_Write(IIC_ADD + 1);

    I2C_ReadBuffer(pBuffer, Size);

    I2C_Stop();
    __enable_irq();

}

void EEPROM_WriteBuffer(uint32_t Address, const void *pBuffer, uint8_t WRITE_SIZE) {

    if (pBuffer == NULL)
        return;
    uint8_t buffer[128];
    EEPROM_ReadBuffer(Address, buffer, WRITE_SIZE);
    if (memcmp(pBuffer, buffer, WRITE_SIZE) != 0) {
        uint8_t IIC_ADD = 0xA0 | ((Address / 0x10000) << 1);
        I2C_Start();
#if ENABLE_EEPROM_TYPE == 1
        if(Address>=0x40000)
            IIC_ADD = 0xA8 | (((Address - 0x40000) / 0x10000) << 1);
#elif ENABLE_EEPROM_TYPE == 2
        if(Address>=0x20000)
            IIC_ADD = 0xA4 | (((Address - 0x20000) / 0x10000) << 1);
#endif
        I2C_Write(IIC_ADD);

        I2C_Write((Address >> 8) & 0xFF);
        I2C_Write((Address) & 0xFF);
        I2C_WriteBuffer(pBuffer, WRITE_SIZE);
        I2C_Stop();
    }
    SYSTEM_DelayMs(8);

}


void EEPROM_Buffer_MORE(uint32_t Address, void *pBuffer, uint16_t WRITE_SIZE, uint8_t TYPE) {
    uint16_t SUM_WRITE = 0;
    while (WRITE_SIZE) {
        uint16_t NOW_WRITE_SIZE = (WRITE_SIZE < 128 ? WRITE_SIZE : 128) - (Address & 0x7f);
        WRITE_SIZE -= NOW_WRITE_SIZE;
        if (TYPE) EEPROM_WriteBuffer(Address, pBuffer + SUM_WRITE, NOW_WRITE_SIZE);
        else EEPROM_ReadBuffer(Address, pBuffer + SUM_WRITE, NOW_WRITE_SIZE);

        SUM_WRITE += NOW_WRITE_SIZE;
        Address += NOW_WRITE_SIZE;
    }
}