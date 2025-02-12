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
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "tle/eci.h"
#include "bsp/dp32g030/gpio.h"
#include "driver/gpio.h"
#include "app/si.h"

#include "driver/i2c.h"
#include "driver/system.h"
#include "frequencies.h"
#include "misc.h"
#include "app/doppler.h"
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
#include "time.h"
#include "app/doppler.h"
#include "bsp/dp32g030/rtc.h"
#include "bsp/dp32g030/uart.h"
#include "bsp/dp32g030/timer.h"
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
#include "bsp/dp32g030/pwmplus.h"
#include "driver/uart.h"
#include "app/spectrum.h"
#include "helper/battery.h"
#include "helper/boot.h"
#include "ui/lock.h"
#include "ui/welcome.h"
#include "ui/menu.h"
#include "driver/eeprom.h"
#include "driver/st7565.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "tle/eci.h"
void _putchar(__attribute__((unused)) char c) {

#ifdef ENABLE_UART
    UART_Send((uint8_t *) &c, 1);
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
                          | SYSCON_DEV_CLK_GATE_PWM_PLUS0_BITS_ENABLE
                          | (1 << 12)
#ifndef ENABLE_RTC
                          | (1 << 22)
#endif

                          ;

    SYSTICK_Init();
    BOARD_Init();
    UART_Init();
    BK4819_Init();

    // BOARD_ADC_GetBatteryInfo(&gBatteryCurrentVoltage, &gBatteryCurrent);


    SETTINGS_InitEEPROM();


    SETTINGS_LoadCalibration();

#ifndef ENABLE_RTC

    RTC_INIT();
#endif

    // INIT_DOPPLER_DATA();
    // RADIO_ConfigureChannel(0, VFO_CONFIGURE_RELOAD);
    // RADIO_ConfigureChannel(1, VFO_CONFIGURE_RELOAD);
    // RADIO_SelectVfos();
    RADIO_SetupRegisters(true);

    // for (uint32_t i = 0; i < ARRAY_SIZE(gBatteryVoltages); i++)
    //     BOARD_ADC_GetBatteryInfo(&gBatteryVoltages[i], &gBatteryCurrent);

    // BATTERY_GetReadings(false);
    TIM0_INIT();
    memset(gStatusLine,0,sizeof(gStatusLine));
    memset(gFrameBuffer,0,sizeof(gFrameBuffer));
    ST7565_BlitStatusLine();  // blank status line
    ST7565_BlitFullScreen();
    PWM_PLUS0_CH0_COMP = 255<<2;
    GPIO_ClearBit(&GPIOA->DATA, GPIOA_PIN_VOICE_0);

    //  if(tle_check()==0)
    // {

    //     show_uint32(888,0);

    //     // if(sate_info.num>45 | sate_info.num==0) {
    //     //     ERROR_DISPLAY();
    //     //     NVIC_SystemReset();
    //     // }
    // }
    // show_uint32(456,0);
  TLE_Main();

    // show_uint32((uint32_t)((int)a*1000),1);

    while (1) {


// for (int i = 0; i < 24; i+=num)
// {
//        EEPROM_ReadBuffer(0x2c00+i,byte_array+i,num);

// }
    //    EEPROM_ReadBuffer(0x2c00,byte_array,24);


    // memcpy(&a,byte_array, 8);
    // char aaa[25];
    // floatToString(a,aaa);
    // UI_PrintStringSmall(aaa,0,127,5);

    // ST7565_BlitFullScreen();
// #define R 30
// #define center_X 37
// #define center_Y 32
//          for (int y = center_Y - 3 * R; y <= center_Y + 3 * R; y++) {  // y方向按3倍步长计算
//         int dy = (y - center_Y) / 3;  // 还原到圆的标准坐标系
//         int dx_squared = R * R - dy * dy;  // 计算 (x - center_X)^2
//         if (dx_squared >= 0) {
//             int dx = (int)sqrt(dx_squared) * 2;  // 计算dx并按2倍进行扩展
//             DrawPoint(center_X + dx, y);
//             DrawPoint(center_X - dx, y);
//         }
//     }

        // ST7565_BlitStatusLine();  // blank status line
        // ST7565_BlitFullScreen();
        // SYSTEM_DelayMs(20);


    }

}

//char c[170]={0};
//char d[170]={0};
//
//char line0[10]="1233";
//char line1[70]="1 25544U 98067A   25026.23381182  .00024447  00000+0  42619-3 0  9998";
//char line2[70]="2 25544  51.6395 290.1622 0002159 133.7357  10.9595 15.50580823493146";
//memcpy(c,line0,9);memcpy(c+9,line1,69);memcpy(c+9+69,line2,69);
//c[9+69+69]=0xc4;
//c[9+69+69+1]=9;
//c[9+69+69+2]=0xc4;
//c[9+69+69+3]=9;
//
//c[9+69+69+4]=0xc0;
//c[9+69+69+5]=0x20;
//c[9+69+69+6]=0x90;
//c[9+69+69+7]=2;
//
//c[9+69+69+8]=0xc0;
//c[9+69+69+9]=0x20;
//c[9+69+69+10]=0x90;
//c[9+69+69+11]=2;
////    memcpy(c+9+69+69,2500,2);
////    memcpy(c+9+69+69+2,2500,2);
////    memcpy(c+9+69+69+2+2,43000000,4);
////    memcpy(c+9+69+69+2+2+4,43000000,4);
//
//int cnts=0;
//int mmmm=160;
//for(int num=0;num<45;num++){
//    uint32_t base_add  =0x1E200+num*mmmm;
//    //    uint32_t name_add  =base_add;//
//    //    uint32_t line1_add =base_add+9;//line1
//    //    uint32_t line2_add =base_add+9+69;//line2
//    //    uint32_t TT_add    =base_add+9+69+69;//发射亚音
//    //    uint32_t RT_add    =base_add+9+69+69+2;//接收亚音
//    //    uint32_t TF_add    =base_add+9+69+69+2+2;//发射频率
//    //    uint32_t RF_add    =base_add+9+69+69+2+2+4;//接收频率
//    int www=32;
//    for(int i=0;i<160;i+=www){
//        EEPROM_WriteBuffer(base_add+i, c+i,www) ;
//    }
//
//
//    //    EEPROM_WriteBuffer(base_add,"123456789",9) ;
//    //    EEPROM_WriteBuffer(line1_add,line1,70) ;
//    //    EEPROM_WriteBuffer(line2_add,line2,70) ;
//    //    EEPROM_WriteBuffer(TT_add, 2500,2);
//    //    EEPROM_WriteBuffer(RT_add,2500,2);
//    //    EEPROM_WriteBuffer(TF_add, 43000000,4);
//    //    EEPROM_WriteBuffer(RF_add,43000000,4);
//}



//
////            show_uint32(now_sat.RX_FREQ,5);
//        memset(gFrameBuffer[5],0,0);
//        UI_PrintStringSmall(now_sat.RX_FREQ, 0, 127, 5);
//        ST7565_BlitFullScreen();
//                show_uint32(now_sat.TX_TONE,5);


//        uint8_t my_time[6]={25,1,27,8,28,42};
//        look_result look;
//        lat_lon sub_point;
//        lat_lon observer = { 50, 50, 50 };
//int cnt=0;
//        show_uint32(cal_sat(line0,line1,line2,observer,&look,&sub_point,my_time),0);
//        for(int i=0;i<69;i++)
//          {
//          if(line2[i]!=now_sat.line2[i])
//            {
//            cnt++;
//            }
//          }
//        show_uint32(cnt,1);