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
#ifdef ENABLE_4732


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
    UART_Init();

    memset(gStatusLine, 0, sizeof(gStatusLine));
    memset(gFrameBuffer, 0, sizeof(gFrameBuffer));
    ST7565_BlitStatusLine();
    ST7565_BlitFullScreen();




        GPIO_SetBit(&GPIOB->DATA, GPIOB_PIN_BACKLIGHT);
        UI_PrintStringSmall("Boot OK", 0, 127, 2);
        UI_PrintStringSmall("Release Key", 0, 127, 6);
        ST7565_BlitStatusLine();
        ST7565_BlitFullScreen();


    const char *line0 = "ISS (ZARYA)";
    const char *line1 = "1 25544U 98067A   25026.23381182  .00024447  00000+0  42619-3 0  9998";
    const char *line2 = "2 25544  51.6395 290.1622 0002159 133.7357  10.9595 15.50580823493146";
    // 解析TLE数据
tle_data data;  // 直接在栈上创建一个tle_data
            struct tm utc ;
      utc.tm_isdst=0;
            utc.tm_yday=26;
            utc.tm_wday=1;
            utc.tm_year=125;
            utc.tm_mon =0;
            utc.tm_mday=27;
            utc.tm_hour=8;
            utc.tm_min=28;
            utc.tm_sec=42;
        memset(gFrameBuffer,0,sizeof(gFrameBuffer));
    ST7565_BlitFullScreen();
 show_uint32(456,3) ;
     SYSTEM_DelayMs(3000);

    while (1) {

//125 0 27 7 39 3Look: 57.454126 -18.469256
//Pos: 51.546400, 125.533706
//
//125 0 27 7 39 4Look: 57.403762 -18.499665
//Pos: 51.553088, 125.632517
//
//125 0 27 7 39 5Look: 57.353492 -18.530080
//Pos: 51.559686, 125.731359

    if (tle_parse(line0, line1, line2, &data) == 0) {  // 修改为传递指针
    } else {
        lat_lon observer = { 50, 50, 50 }; // 设定观察者的经纬度和海拔

            jd target = to_jd(utc.tm_year + 1900, utc.tm_mon + 1, utc.tm_mday, utc.tm_hour, utc.tm_min, utc.tm_sec);

            look_result look = eci_to_look(&data, observer, target);

int line=0;
    memset(gFrameBuffer[line],0,256);
    char str[20] = {0};
    sprintf(str, "%d.%d", (int)(look.azimuth), (int)(look.azimuth*100)%100);
    UI_PrintStringSmall(str, 0, 127, line);
        sprintf(str, "%d.%d", (int)(look.altitude), (int)(-look.altitude*100)%100);
    UI_PrintStringSmall(str, 1, 127, line);
    ST7565_BlitFullScreen();
    SYSTEM_DelayMs(1000);
 utc.tm_sec+=1;
 if(utc.tm_sec >= 60) {
   utc.tm_sec=0;
 utc.tm_min+=1;

 }
 if(utc.tm_min >= 60) {
   utc.tm_min=0;
   utc.tm_hour+=1;
 }
    }
    }
}

