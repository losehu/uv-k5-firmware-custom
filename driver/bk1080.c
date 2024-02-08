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

#include "bsp/dp32g030/gpio.h"
#include "bk1080.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/system.h"
#include "frequencies.h"
#include "misc.h"

//#define CHAN_SPACING     0u  // 200kHz
//#define CHAN_SPACING     1u  // 100kHz
#define   CHAN_SPACING     2u  // 50kHz

#define VOLUME             15u

#define SEEK_THRESHOLD     10u

const freq_band_table_t FM_RADIO_FREQ_BAND_TABLE[] =
        {
                {875, 1080},   // 87.5 ~ 108 MHz
                {760, 1080},   // 76   ~ 108 MHz
                {760, 900},   // 76   ~  90 MHz
                {640, 760}    // 64   ~  76 MHz
        };

static const uint16_t BK1080_RegisterTable[] =
        {
                0x0008,                 // 0x00
                0x1080,                 // 0x01   chip ID
                (1u << 9) | (1u << 0),  // 0x02   0x0201  0000001000000001
                0x0000,                 // 0x03
                0x40C0,                 // 0x04   0100000011000000
                (SEEK_THRESHOLD << 8) | (0u << 6) | (CHAN_SPACING << 4) |
                (VOLUME << 0), // 0x0A1F,  // 0x05  00001010 00 01 1111
                0x002E,                 // 0x06   0000000000101110
                0x02FF,                 // 0x07   0000001011111111
                0x5B11,                 // 0x08   0101101100010001
                0x0000,                 // 0x09
                0x411E,                 // 0x0A   0100000100011110
                0x0000,                 // 0x0B
                0xCE00,                 // 0x0C   1100111000000000
                0x0000,                 // 0x0D
                0x0000,                 // 0x0E
                0x1000,                 // 0x0F   1000000000000000
                0x3197,                 // 0x10   0011000110010111
                0x0000,                 // 0x11
                0x13FF,                 // 0x12   0001001111111111
                0x9852,                 // 0x13   1001100001010010
                0x0000,                 // 0x14
                0x0000,                 // 0x15
                0x0008,                 // 0x16
                0x0000,                 // 0x17
                0x51E1,                 // 0x18   0101000111100001
                0xA8BC,                 // 0x19   1010100010111100
                0x2645,                 // 0x1A   0010011001000101
                0x00E4,                 // 0x1B   0000000011100100
                0x1CD8,                 // 0x1C   0001110011011000
                0x3A50,                 // 0x1D   0011101001010000
                0xEAE0,                 // 0x1E   1110101011100000
                0x3000,                 // 0x1F   0011000000000000
                0x0200,                 // 0x20   0010000000000000
                0x0000                  // 0x21
        };

uint16_t BK1080_BaseFrequency;
uint16_t BK1080_FrequencyDeviation;

bool is_init;
uint16_t BK1080_freq_lower;
uint16_t BK1080_freq_upper;
uint16_t BK1080_freq_base;
int16_t BK1080_freq_offset;

void BK1080_Init(const uint16_t frequency, const bool initialise) {
    unsigned int i;

    // determine the lower and upper frequency limits when multiple bands are used

    if (!is_init) {
        BK1080_freq_base = 0;
        BK1080_freq_offset = 0;

        BK1080_freq_lower = 0xffff;
        BK1080_freq_upper = 0;

        for (i = 0; i < ARRAY_SIZE(FM_RADIO_FREQ_BAND_TABLE); i++) {
            const uint16_t lower = FM_RADIO_FREQ_BAND_TABLE[i].lower;
            const uint16_t upper = FM_RADIO_FREQ_BAND_TABLE[i].upper;

            if (BK1080_freq_lower > lower)
                BK1080_freq_lower = lower;

            if (BK1080_freq_upper < upper)
                BK1080_freq_upper = upper;
        }
    }

    if (initialise) {    // init and enable the chip

        GPIO_ClearBit(&GPIOB->DATA, GPIOB_PIN_BK1080);

        if (!is_init) {
            for (i = 0; i < ARRAY_SIZE(BK1080_RegisterTable); i++)
                BK1080_WriteRegister(i, BK1080_RegisterTable[i]);

            SYSTEM_DelayMs(250);

            BK1080_WriteRegister(BK1080_REG_25_INTERNAL, 0xA83C);  // 1010 1000 0011 1100
            BK1080_WriteRegister(BK1080_REG_25_INTERNAL, 0xA8BC);  // 1010 1000 1011 1100

            SYSTEM_DelayMs(60);

            is_init = true;
        } else {
            BK1080_WriteRegister(BK1080_REG_02_POWER_CONFIGURATION, (1u << 9) | (1u << 0));
        }

        BK1080_WriteRegister(BK1080_REG_05_SYSTEM_CONFIGURATION2, 0x0A5F);  // 0000 1010 0101 1111

        BK1080_SetFrequency(frequency);
    } else {    // disable the chip

        BK1080_WriteRegister(BK1080_REG_02_POWER_CONFIGURATION,
                             (1u << 9) | (1u << 6) | (1u << 0)); // 0x0241); // 0000 0010 0100 0001
        GPIO_SetBit(&GPIOB->DATA, GPIOB_PIN_BK1080);
    }
}

uint16_t BK1080_ReadRegister(BK1080_Register_t Register) {
    uint8_t Value[2];
    I2C_Start();
    I2C_Write(0x80);
    I2C_Write((Register << 1) | I2C_READ);
    I2C_ReadBuffer(Value, sizeof(Value));
    I2C_Stop();
    return (Value[0] << 8) | Value[1];
}

void BK1080_WriteRegister(BK1080_Register_t Register, uint16_t Value) {
    I2C_Start();
    I2C_Write(0x80);
    I2C_Write((Register << 1) | I2C_WRITE);
    Value = ((Value >> 8) & 0xFF) | ((Value & 0xFF) << 8);
    I2C_WriteBuffer(&Value, sizeof(Value));
    I2C_Stop();
}

void BK1080_Mute(const bool Mute) {
    BK1080_WriteRegister(BK1080_REG_02_POWER_CONFIGURATION, (1u << 9) | (1u << 0) | (Mute ? 1u << 14 : 0u));
}

void BK1080_SetFrequency(uint16_t Frequency) {
    int channel;
    uint16_t band = 0;

    // determine which band to use
    for (band = 0; band < ARRAY_SIZE(FM_RADIO_FREQ_BAND_TABLE); band++)
        if (Frequency >= FM_RADIO_FREQ_BAND_TABLE[band].lower && Frequency < FM_RADIO_FREQ_BAND_TABLE[band].upper)
            break;

    if (band >= ARRAY_SIZE(FM_RADIO_FREQ_BAND_TABLE)) {
        Frequency = BK1080_freq_lower;
    }

//	channel =  (int)Frequency - FM_RADIO_FREQ_BAND_TABLE[band].lower;       // 100kHz channel spacing
    channel = ((int) Frequency - FM_RADIO_FREQ_BAND_TABLE[band].lower) * 2;  // 50kHz channel spacing
    channel = (channel < 0) ? 0 : (channel > 1023) ? 1023 : channel;

    BK1080_WriteRegister(BK1080_REG_05_SYSTEM_CONFIGURATION2,
                         (SEEK_THRESHOLD << 8) | (band << 6) | (CHAN_SPACING << 4) | (VOLUME << 0));

    BK1080_WriteRegister(BK1080_REG_03_CHANNEL, (uint16_t) channel);
//	SYSTEM_DelayMs(1);
    BK1080_WriteRegister(BK1080_REG_03_CHANNEL, (uint16_t) channel | (1u << 15));
}

int16_t BK1080_get_freq_offset(const uint16_t Frequency) {
    BK1080_freq_base = Frequency;
    BK1080_freq_offset = (int16_t) BK1080_ReadRegister(BK1080_REG_07) / 16;
    return BK1080_freq_offset;
}

void BK1080_GetFrequencyDeviation(uint16_t Frequency) {
    BK1080_BaseFrequency = Frequency;
    BK1080_FrequencyDeviation = BK1080_ReadRegister(BK1080_REG_07) / 16;
}