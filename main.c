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
#include "driver/uart.h"
#include "string.h"
#include <stdio.h>
#include "ui/helper.h"
#include <string.h>
#include "driver/bk4819.h"
#include "font.h"
#include "ui/ui.h"
#include <stdint.h>
#include <string.h>
#include "font.h"
#include <stdio.h>     // NULL
#include "app/mdc1200.h"
#include "app/uart.h"
#include "string.h"
#include "app/messenger.h"
#ifdef ENABLE_DOPPLER
#include "app/doppler.h"
#endif
#ifdef ENABLE_AM_FIX
#include "am_fix.h"
#endif

#include "bsp/dp32g030/rtc.h"

#ifdef ENABLE_TIMER
#include "bsp/dp32g030/uart.h"
#include "bsp/dp32g030/timer.h"
#endif

#include "audio.h"
#include "board.h"
#include "misc.h"
#include "radio.h"
#include "settings.h"
#include "version.h"
#include "app/app.h"
#include "app/dtmf.h"
#include "bsp/dp32g030/gpio.h"
#include "bsp/dp32g030/syscon.h"
#include "driver/backlight.h"
#include "driver/bk4819.h"
#include "driver/gpio.h"
#include "driver/system.h"
#include "driver/systick.h"

#ifdef ENABLE_UART
#include "driver/uart.h"
#endif

#include "helper/battery.h"
#include "helper/boot.h"

#include "ui/lock.h"
#include "ui/welcome.h"
#include "ui/menu.h"
#include "driver/eeprom.h"
#include "driver/st7565.h"

void _putchar(__attribute__((unused)) char c) {

#ifdef ENABLE_UART
    UART_Send((uint8_t *)&c, 1);
#endif

}

#ifdef ENABLE_RTC
void show_uint32(uint32_t num,uint8_t  line)
{
    char str[6] = {0};

    str[0] = (num / 100000) + '0';
    str[1] = (num / 10000) + '0';
    str[2] = (num / 1000) % 10 + '0';
    str[3] = (num / 100) % 10 + '0';
    str[4] = (num / 10) % 10 + '0';
    str[5] = (num % 10) + '0';
    str[6] = '\0'; // 添加字符串结束符
    UI_PrintStringSmall(str, 0, 127, line);
    ST7565_BlitFullScreen();
}
// 定义位掩码
#define SEC_TENS_RO_MASK 0x70 // 0b1110000
#define SEC_ONES_RO_MASK 0x0F // 0b0001111
#define MIN_TENS_RO_MASK 0x7000 // 0b0111 0000 0000 0000
#define MIN_ONES_RO_MASK 0xf00 // 0b000 1111 0000 0000

// 从寄存器值中提取十位数和个位数并计算十进制秒数
uint32_t calculate_decimal_seconds(uint32_t register_value) {
    // 提取十位数和个位数
    uint32_t SEC_TENS_RO = (register_value & SEC_TENS_RO_MASK) >> 4;
    uint32_t SEC_ONES_RO = register_value & SEC_ONES_RO_MASK;

    // 计算十进制秒数
    uint32_t decimal_seconds = SEC_TENS_RO * 10 + SEC_ONES_RO;

    return decimal_seconds;
}
uint32_t calculate_decimal_min(uint32_t register_value) {
    // 提取十位数和个位数
    uint32_t SEC_TENS_RO = (register_value & MIN_TENS_RO_MASK) >> 12;
    uint32_t SEC_ONES_RO = (register_value & MIN_ONES_RO_MASK)>>8;

    // 计算十进制秒数
    uint32_t decimal_seconds = SEC_TENS_RO * 10 + SEC_ONES_RO;

    return decimal_seconds;
}
#endif

void Main(void) {
    //BOOT_Mode_t  BootMode;

    // Enable clock gating of blocks we need
    SYSCON_DEV_CLK_GATE = 0
                          | SYSCON_DEV_CLK_GATE_GPIOA_BITS_ENABLE
                          | SYSCON_DEV_CLK_GATE_GPIOB_BITS_ENABLE
                          | SYSCON_DEV_CLK_GATE_GPIOC_BITS_ENABLE
                          | SYSCON_DEV_CLK_GATE_UART1_BITS_ENABLE
                          | SYSCON_DEV_CLK_GATE_SPI0_BITS_ENABLE
                          | SYSCON_DEV_CLK_GATE_SARADC_BITS_ENABLE
                          | SYSCON_DEV_CLK_GATE_CRC_BITS_ENABLE
                          | SYSCON_DEV_CLK_GATE_AES_BITS_ENABLE
                          | SYSCON_DEV_CLK_GATE_PWM_PLUS0_BITS_ENABLE
                          | (1 << 12)
                          | (1 << 22);

    SYSTICK_Init();
#ifdef ENABLE_RTC

    BOARD_PORTCON_Init();
    BOARD_GPIO_Init();
    ST7565_Init();
    memset(gStatusLine, 0, sizeof(gStatusLine));
    UI_DisplayClear();
    ST7565_BlitStatusLine();  // blank status line
    ST7565_BlitFullScreen();
    char str[20] = {0}; // 分配一个足够大的字符串数组来存储转换后的字符串
    RTC_INIT();

    while (1) {
         show_uint32((RTC_IF>>8)&0X01,0);
        show_uint32(RTC_VALID&0X01,1);

        show_uint32(calculate_decimal_seconds(RTC_TSTR),2);
        show_uint32(calculate_decimal_min(RTC_TSTR),3);


    }
#endif
#ifdef ENABLE_TIMER


    BOARD_PORTCON_Init();
    BOARD_GPIO_Init();
    ST7565_Init();
    TIM0_INIT();
    memset(gStatusLine, 0, sizeof(gStatusLine));
    UI_DisplayClear();
    ST7565_BlitStatusLine();  // blank status line
    ST7565_BlitFullScreen();
    char str[20]={0}; // 分配一个足够大的字符串数组来存储转换后的字符串
    while(1)
    {
        char str[6];
        show_uint32(TIM0_CNT,0);
        show_uint32(TIMERBASE0_LOW_CNT,1);
        show_uint32(TIMERBASE0_HIGH_CNT,2);
        show_uint32(TIMERBASE0_IF,3);
        show_uint32(TIMERBASE0_IE,4);
    }
#endif
    BOARD_Init();

    boot_counter_10ms = 250;   // 2.5 sec
#ifdef ENABLE_UART
    UART_Init();
#endif
    SETTINGS_InitEEPROM();


    memset(gDTMF_String, '-', sizeof(gDTMF_String));
    gDTMF_String[sizeof(gDTMF_String) - 1] = 0;

    BK4819_Init();

    BOARD_ADC_GetBatteryInfo(&gBatteryCurrentVoltage, &gBatteryCurrent);


    SETTINGS_InitEEPROM();


    SETTINGS_LoadCalibration();
#ifdef ENABLE_MESSENGER
    MSG_Init();
#endif
#ifdef ENABLE_MDC1200
    MDC1200_init();
#endif


    RADIO_ConfigureChannel(0, VFO_CONFIGURE_RELOAD);
    RADIO_ConfigureChannel(1, VFO_CONFIGURE_RELOAD);


    RADIO_SelectVfos();

    RADIO_SetupRegisters(true);

    for (uint32_t i = 0; i < ARRAY_SIZE(gBatteryVoltages); i++) {
        BOARD_ADC_GetBatteryInfo(&gBatteryVoltages[i], &gBatteryCurrent);
    }
    BATTERY_GetReadings(false);

#ifdef ENABLE_AM_FIX
    AM_fix_init();
#endif

#if ENABLE_CHINESE_FULL == 0
    gMenuListCount = 52;
#else
    gMenuListCount=53;
#endif

    gKeyReading0 = KEY_INVALID;
    gKeyReading1 = KEY_INVALID;
    gDebounceCounter = 0;
    UI_DisplayWelcome();


    boot_counter_10ms = 250;

    while (boot_counter_10ms > 0 || (KEYBOARD_Poll() != KEY_INVALID)) {

        if (KEYBOARD_Poll() == KEY_EXIT
#if ENABLE_CHINESE_FULL == 4
            ||gEeprom.POWER_ON_DISPLAY_MODE == POWER_ON_DISPLAY_MODE_NONE
#endif
                ) {    // halt boot beeps
            boot_counter_10ms = 0;
            break;
        }
#ifdef ENABLE_BOOT_BEEPS

        if ((boot_counter_10ms % 25) == 0)
                    AUDIO_PlayBeep(BEEP_880HZ_40MS_OPTIONAL);
#endif

    }


#ifdef ENABLE_PWRON_PASSWORD
    if (gEeprom.POWER_ON_PASSWORD < 1000000)
    {
        bIsInLockScreen = true;
        UI_DisplayLock();
        bIsInLockScreen = false;
    }
#endif

    //	BOOT_ProcessMode();
    GUI_SelectNextDisplay(DISPLAY_MAIN);

    GPIO_ClearBit(&GPIOA->DATA, GPIOA_PIN_VOICE_0);

    gUpdateStatus = true;

#ifdef ENABLE_VOICE
    {
        uint8_t Channel;

        AUDIO_SetVoiceID(0, VOICE_ID_WELCOME);

        Channel = gEeprom.ScreenChannel[gEeprom.TX_VFO];
        if (IS_MR_CHANNEL(Channel))
        {
            AUDIO_SetVoiceID(1, VOICE_ID_CHANNEL_MODE);
            AUDIO_SetDigitVoice(2, Channel + 1);
        }
        else if (IS_FREQ_CHANNEL(Channel))
            AUDIO_SetVoiceID(1, VOICE_ID_FREQUENCY_MODE);

        AUDIO_PlaySingleVoice(0);
    }
#endif

#ifdef ENABLE_NOAA
    RADIO_ConfigureNOAA();
#endif


    while (1) {

        APP_Update();
        if (gNextTimeslice) {
            APP_TimeSlice10ms();
        }
        if (gNextTimeslice_500ms) {
            APP_TimeSlice500ms();
        }
    }
}
