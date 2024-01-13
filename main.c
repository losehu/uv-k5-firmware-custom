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

#include "ui/ui.h"
#include <stdint.h>
#include <string.h>
#include "font.h"
#include <stdio.h>     // NULL
#include "app/mdc1200.h"
#include "app/uart.h"
#include "string.h"

#ifdef ENABLE_AM_FIX
#include "am_fix.h"
#endif
#ifdef ENABLE_TIMER
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
                          | SYSCON_DEV_CLK_GATE_PWM_PLUS0_BITS_ENABLE;

    SYSTICK_Init();
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


    SETTINGS_WriteBuildOptions();
    SETTINGS_LoadCalibration();
#ifdef ENABLE_MDC1200
    MDC1200_init();
#endif


    RADIO_ConfigureChannel(0, VFO_CONFIGURE_RELOAD);
    RADIO_ConfigureChannel(1, VFO_CONFIGURE_RELOAD);
    //0x2000~1e31c中文字库
    // 1e31e开机字符1长度  1e31f开机字符2长度
    // 0x1e320~0x1e332 开机字符1
    // 0x1e333~0x1e345 开机字符2

#if ENABLE_CHINESE_FULL > 0&&ENABLE_CHINESE_FULL < 4
    BACKLIGHT_TurnOn();
       UI_PrintStringSmall("FREE FIRMWARE!", 0, 127, 0);
       UI_PrintStringSmall("QQ QUN: 707856906!", 0, 127, 1);
               ST7565_BlitStatusLine();  // blank status line
        ST7565_BlitFullScreen();
    uint32_t ALL_SIZE = 40960;
        uint32_t start_add = 0x2000;
        const uint8_t *p_font;
    #if ENABLE_CHINESE_FULL==1
        ALL_SIZE = 40960;
        start_add = 0x2000;
        p_font = gFontChinese_out1;

       UI_PrintStringSmall("FONT 1 / 3", 0, 127, 2);

        ST7565_BlitStatusLine();  // blank status line
        ST7565_BlitFullScreen();
    #elif ENABLE_CHINESE_FULL == 2
        ALL_SIZE=40960;
        start_add= 0x2000+40960;
        p_font=gFontChinese_out2;
       UI_PrintStringSmall("FONT 2 / 3", 0, 127, 2);

        ST7565_BlitStatusLine();  // blank status line
        ST7565_BlitFullScreen();
    #elif  ENABLE_CHINESE_FULL==3
        ALL_SIZE=33564;
        start_add=  0x2000+40960*2;
        p_font=gFontChinese_out3;
       UI_PrintStringSmall("FONT 3 / 3", 0, 127, 2);

        ST7565_BlitStatusLine();  // blank status line
        ST7565_BlitFullScreen();
    #endif
    for (uint32_t i = 0; i < ALL_SIZE; i += 128) {
            if (i != (uint32_t) (ALL_SIZE / 128) * 128) {
                WRITE_SIZE = 128;
                EEPROM_WriteBuffer(start_add + i, p_font + i, WRITE_SIZE);
                //check
                uint8_t B[128];
                EEPROM_ReadBuffer(start_add + i, B, 128);
                if (memcmp(B, p_font + i, 128) != 0) {

                    UI_PrintStringSmall("FAILED!", 0, 127, 4);

                    ST7565_BlitStatusLine();  // blank status line
                    ST7565_BlitFullScreen();
                    UART_Send("FAILED!\n", 7);
                    UART_Send(B,128);
                    while (1);
                } else {
                    char str[10] = {0};
                    float a = ((float) (i + 128) / (float) ALL_SIZE) * (float) 100;
                    if (a < 100) {
                        str[0] = (int) a / 10 + '0';
                        str[1] = (int) a % 10 + '0';
                        str[2] = '.';
                        str[3] = (int) (10 * a) % 10 + '0';
                        str[4] = '\n';
                        str[5] = '\0';


                    } else {
                        strcpy(str, "100!\n");
                    }
                    UI_PrintStringSmall(str, 0, 127, 3);
                    ST7565_BlitStatusLine();  // blank status line
                    ST7565_BlitFullScreen();
                    UART_Send(str, 5);
                }

            } else {
                uint8_t tmp[128] = {0};
                WRITE_SIZE = ALL_SIZE - (uint32_t) (ALL_SIZE / 128) * 128;
                memcpy(tmp, p_font + i, WRITE_SIZE);
                EEPROM_WriteBuffer(start_add + i, tmp, WRITE_SIZE);
                //check
                uint8_t B[128];
                memset(B, 0, sizeof(B));
                EEPROM_ReadBuffer(start_add + i, B, WRITE_SIZE);
                if (memcmp(B, p_font + i, WRITE_SIZE) != 0) {
                    UI_PrintStringSmall("FAILED!", 0, 127, 3);
                    ST7565_BlitStatusLine();  // blank status line
                    ST7565_BlitFullScreen();
                    UART_Send("FAILED!\n", 7);
                    while (1);
                } else {
                    char str[10] = {0};
                    float a = ((float) (i + 128) / (float) ALL_SIZE) * (float) 100;
                    if (a < 100) {
                        str[0] = (int) a / 10 + '0';
                        str[1] = (int) a % 10 + '0';
                        str[2] = '.';
                        str[3] = (int) (10 * a) % 10 + '0';
                        str[4] = '\n';
                        str[5] = '\0';

                    } else {
                        strcpy(str, "100!\n");
                    }
                    UI_PrintStringSmall(str, 0, 127, 3);
                    ST7565_BlitStatusLine();  // blank status line
                    ST7565_BlitFullScreen();
                        UART_Send(str, 5);
                }

            }
        }
        uint8_t check_start[6],check_end[6];
            #if ENABLE_CHINESE_FULL==1
                uint8_t check1_start[6]={0X40,0X40,0X40,0X40,0X40,0X40};
                uint8_t check1_end[6]={0XFE,0X92,0XFE,0X08,0XF4,0X97};
                memcpy(check_start,check1_start,6);
                memcpy(check_end,check1_end,6);

            #elif ENABLE_CHINESE_FULL==2
                uint8_t check2_start[6]={0X94,0XF4,0X04,0XFC,0X78,0XF8};
                uint8_t check2_end[6]={0XAF,0XEA,0X08,0X78,0XFA,0XAA};
                memcpy(check_start,check2_start,6);
                memcpy(check_end,check2_end,6);
            #elif ENABLE_CHINESE_FULL==3
                uint8_t check3_start[6]={0XAB,0XFA,0X4A,0XF5,0X4F,0X45};
                uint8_t check3_end[6]={0X6A,0XE2,0X6A,0XAF,0X63,0X0A};
                memcpy(check_start,check3_start,6);
                memcpy(check_end,check3_end,6);
            #endif
        uint8_t check[8];
        EEPROM_ReadBuffer(start_add , check, 6);
        if(memcmp(check,check_start,6)!=0)
        {
            UI_PrintStringSmall("CHECK FAIL!", 0, 127, 3);
            ST7565_BlitStatusLine();  // blank status line
            ST7565_BlitFullScreen();
            UART_Send("CHECK FAIL!\n", 12);
            while(1);
        }
        EEPROM_ReadBuffer(start_add+ALL_SIZE-6 , check, 6);
        if(memcmp(check,check_end,6)!=0)
        {
            UI_PrintStringSmall("CHECK FAIL!", 0, 127, 3);
            ST7565_BlitStatusLine();  // blank status line
            ST7565_BlitFullScreen();
            UART_Send("CHECK FAIL!\n", 12);
            while(1);
        }
    UI_PrintStringSmall("CHECK OK!", 0, 127, 4);
    ST7565_BlitStatusLine();  // blank status line
    ST7565_BlitFullScreen();
    UART_Send("CHECK OK!\n", 10);

    while (1) {}

#endif


    RADIO_SelectVfos();

    RADIO_SetupRegisters(true);

    for (uint32_t i = 0; i < ARRAY_SIZE(gBatteryVoltages); i++) {
        BOARD_ADC_GetBatteryInfo(&gBatteryVoltages[i], &gBatteryCurrent);
    }
    BATTERY_GetReadings(false);


#ifdef ENABLE_AM_FIX
    AM_fix_init();
#endif

    //BootMode = BOOT_GetMode();

//	if (BootMode == BOOT_MODE_F_LOCK)
//    gMenuListCount = 46;

    gMenuListCount = 0;
    while (MenuList[gMenuListCount].name[0] != '\0') gMenuListCount++;
//#if ENBALE_CHINESE_FULL==0
//    gMenuListCount=52;
//#else
//    gMenuListCount=53;
//#endif
    // wait for user to release all butts before moving on
//	if (/*!GPIO_CheckBit(&GPIOC->DATA, GPIOC_PIN_PTT) ||*/
//	     KEYBOARD_Poll() != KEY_INVALID ||
//		 BootMode != BOOT_MODE_NORMAL)
//	{	// keys are pressed
//		UI_DisplayReleaseKeys(BootMode);
//    while (KEYBOARD_Poll() != KEY_INVALID)  // 500ms
//    {
//    }
    gKeyReading0 = KEY_INVALID;
    gKeyReading1 = KEY_INVALID;
    gDebounceCounter = 0;
//	}

//    TIM0_INIT();
//
//    memset(gStatusLine, 0, sizeof(gStatusLine));
//    UI_DisplayClear();
//    ST7565_BlitStatusLine();  // blank status line
//    ST7565_BlitFullScreen();
//    char str[20]={0}; // 分配一个足够大的字符串数组来存储转换后的字符串
//
//    while(1)
//{
//
//    str[0]=TIMERBASE0_LOW_CNT/100+'0';
//    str[1]=TIMERBASE0_LOW_CNT/10%10+'0';
//    str[2]=TIMERBASE0_LOW_CNT%10+'0';
//       UI_PrintStringSmall(str, 0, 127, 3);
//    ST7565_BlitFullScreen();
//
//
//}
//    memset(gStatusLine, 0, sizeof(gStatusLine));
//    UI_DisplayClear();
//    while(1)
//    {
//        char A[128];        char B[128];
//
//        memset(A,'A',sizeof (A));//0x20000 128K 0X40000 256K
//        uint32_t ADD;
//
//#if ENABLE_EEPROM_TYPE==1
//        ADD=0x50000;
//#elif ENABLE_EEPROM_TYPE==2
//        ADD=0x30000;
//#endif
//        EEPROM_WriteBuffer(ADD, A, 128);
//        EEPROM_ReadBuffer(ADD, B, 128) ;
//        if (memcmp(A,B,128)==0)
//        {
//#if ENABLE_EEPROM_TYPE==1
//            UI_PrintStringSmall("Double 2Mb Eeprom", 0, 127, 2);
//            UI_PrintStringSmall("Install OK!", 0, 127, 3);
//
//#elif ENABLE_EEPROM_TYPE==2
//            UI_PrintStringSmall("Double 1Mb Eeprom", 0, 127, 2);
//            UI_PrintStringSmall("Install OK!", 0, 127, 3);
//#endif
//            ST7565_BlitStatusLine();  // blank status line
//            ST7565_BlitFullScreen();
//        } else
//        {
//#if ENABLE_EEPROM_TYPE==1
//            UI_PrintStringSmall("Double 2Mb Eeprom", 0, 127, 2);
//            UI_PrintStringSmall("Install Failed!", 0, 127, 3);
//
//#elif ENABLE_EEPROM_TYPE==2
//            UI_PrintStringSmall("Double 1Mb Eeprom", 0, 127, 2);
//            UI_PrintStringSmall("Install Failed!", 0, 127, 3);
//#endif
//            ST7565_BlitStatusLine();  // blank status line
//            ST7565_BlitFullScreen();
//        }
//
//    }
    UI_DisplayWelcome();


    boot_counter_10ms = 250;

    while (boot_counter_10ms > 0||(KEYBOARD_Poll() != KEY_INVALID)) {

        if (KEYBOARD_Poll() == KEY_EXIT
            #if ENABLE_CHINESE_FULL==4
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
//    int start_add=0x1EFFD;
//    WRITE_SIZE = 11;
//    uint32_t add=0;
//    uint8_t data_write[8];
//    memset(data_write,'Z',sizeof (data_write));
//    EEPROM_WriteBuffer(start_add + add,data_write , WRITE_SIZE);
//    uint8_t B[128];
//    EEPROM_ReadBuffer(start_add + add, B, WRITE_SIZE);
//    if (memcmp(B, data_write, WRITE_SIZE) != 0) {
//
//        UI_PrintStringSmall("FAILED!", 0, 127, 4);
//
//        ST7565_BlitStatusLine();  // blank status line
//        ST7565_BlitFullScreen();
//        UART_Send("FAILED!\n", 7);
//        UART_Send(B, WRITE_SIZE);
//    }else
//    {
//        UI_PrintStringSmall("OK", 0, 127, 3);
//
//        ST7565_BlitStatusLine();  // blank status line
//        ST7565_BlitFullScreen();
//        UART_Send("OK!\n", 7);
//        UART_Send(B, WRITE_SIZE);
//    }
//    while (1);

    while (1) {


//        BK4819_EnterTxMute();
//        BK4819_SetAF(BK4819_AF_MUTE);
//
//        BK4819_WriteRegister(BK4819_REG_70, BK4819_REG_70_ENABLE_TONE1 | (66u << BK4819_REG_70_SHIFT_TONE1_TUNING_GAIN));
//
//        BK4819_EnableTXLink();
//        SYSTEM_DelayMs(50);
//
//
//        BK4819_WriteRegister(BK4819_REG_71, (((uint32_t)500 * 1353245u) + (1u << 16)) >> 17);
//
//        BK4819_ExitTxMute();
//        SYSTEM_DelayMs(800);
//        BK4819_EnterTxMute();
//
//        BK4819_WriteRegister(BK4819_REG_70, 0x0000);
//        BK4819_WriteRegister(BK4819_REG_30, 0xC1FE);   // 1 1 0000 0 1 1111 1 1 1 0





        APP_Update();
        if (gNextTimeslice) {
            APP_TimeSlice10ms();
        }
        if (gNextTimeslice_500ms) {
            APP_TimeSlice500ms();
        }
    }
}
