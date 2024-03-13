//
// Created by RUPC on 2024/3/10.
//
#include "4732.h"
#include "bsp/dp32g030/gpio.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/system.h"
#include "frequencies.h"
#include "misc.h"
#include "ui/helper.h"
#include <string.h>

#include "app/fm.h"
#include "driver/st7565.h"
#include "external/printf/printf.h"
#include "misc.h"
#include "settings.h"
#include "ui/fmradio.h"
#include "ui/helper.h"
#include "ui/inputbox.h"
#include "ui/ui.h"
#include "chinese.h"

#include "functions.h"
#include "stdbool.h"
#include "app/spectrum.h"
#include "am_fix.h"
#include "audio.h"
#include "misc.h"
#include "driver/eeprom.h"
#include "driver/backlight.h"
#include "frequencies.h"
#include "ui/helper.h"
#include "ui/main.h"
#include "driver/backlight.h"

KeyboardState kbds = {KEY_INVALID, KEY_INVALID, 0};
uint32_t NOW_STEP = 10;
uint32_t NOW_FREQ = 12120;// 9300 93MHZ 10000 10MHZ
enum SI4732_MODE_T NOW_MODE = AM;//0FM
uint8_t display_flag = 1;

void SI4732_Display() {
    memset(gStatusLine, 0, sizeof(gStatusLine));
    UI_DisplayClear();
    uint8_t String[19];
    if (NOW_MODE == FM) {

        sprintf(String, "%3d.%02d", NOW_FREQ / 100, NOW_FREQ % 100);
        UI_DisplayFrequency(String, 25, 1, false);
    } else if (NOW_MODE == AM) {
        sprintf(String, "%3d.%03d", NOW_FREQ / 1000, NOW_FREQ % 1000);
        UI_DisplayFrequency(String, 25, 1, false);
    }

    ST7565_BlitStatusLine();  // blank status line
    ST7565_BlitFullScreen();
}

void SI4732_SetFreq(uint32_t freq) {
    if (NOW_MODE == FM) {
        if (freq < 6400 || freq > 10800) return;
        NOW_FREQ = freq;
        uint8_t cmd[5] = {0x20, 0x00, (freq & 0xff00) >> 8, freq & 0x00ff, 0x00};//设置频率
        SI4732_WriteBuffer(cmd, 5);
        SI4732_WAIT_STATUS(0X80);

    } else if (NOW_MODE == AM) {


        if (freq < 149 || freq > 23000) return;
        NOW_FREQ = freq;
        uint8_t cmd[6] = {0x40, 0x00, (freq & 0xff00) >> 8, freq & 0x00ff, 0x00, 0x00};//设置频率
        SI4732_WriteBuffer(cmd, 6);
        SI4732_WAIT_STATUS(0X80);
    }
    SYSTEM_DelayMs(50);

    display_flag = 1;


}

void SI4732_WAIT_STATUS(uint8_t state) {

    uint8_t tmp = 0;
    SI4732_ReadBuffer((uint8_t *) &tmp, 1);
    while (tmp != state) {
        SI4732_ReadBuffer((uint8_t *) &tmp, 1);
    }
}

void SI4732_Main() {
    SI4732_Init();
    BACKLIGHT_TurnOn();

//    int cnt = 1000000/2+1;
    while (1) {
        //设置频率
        HandleUserInput();
//        SI4732_SetFreq(10760);

//        if (cnt == 0) {
//            SI4732_SetFreq(9300);
//        } else if (cnt == 1000000/2) {
//            SI4732_SetFreq(10760);
//        }
//        cnt++;
//        if (cnt == 1000000)cnt = 0;

        if (display_flag) {
            display_flag = 0;
            SI4732_Display();
        }

    }
}


void SI4732_PowerUp() {
    uint8_t cmd[3] = {0x01, 0x10, 0x05};
    if (NOW_MODE == AM) {
        cmd[1] = 0x11;
    }
    SI4732_WriteBuffer(cmd, 3);
    SI4732_WAIT_STATUS(0x80);
    SYSTEM_DelayMs(500);

}

void SI4732_PowerDown() {
    GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
    uint8_t cmd[1] = {0x11};
    SI4732_WriteBuffer(cmd, 1);
    SI4732_WAIT_STATUS(0x80);
    SYSTICK_Delay250ns(1);
    RST_LOW;
}

void SI4732_SwitchMode() {
    SI4732_PowerDown();

}

void SI4732_GET_INT_STATUS() {
    uint8_t state = 0;
    while (state != 0x81) {
        uint8_t cmd3[1] = {0x14};
        SI4732_WriteBuffer(cmd3, 1);
        SI4732_ReadBuffer((uint8_t *) &state, 1);
    }
    SYSTEM_DelayMs(SI4732_DELAY_MS);
}


void SI4732_Init() {


    SI4732_PowerDown();

    RST_HIGH;
    SYSTEM_DelayMs(SI4732_DELAY_MS);

    SI4732_PowerUp();
    SYSTEM_DelayMs(SI4732_DELAY_MS);



//    uint8_t cmd1[6] = {0x12, 0x00, 0x31,0x02,0x00,0x00};
//    SI4732_WriteBuffer(cmd1, 6);
//    SI4732_WAIT_STATUS(0x80);
//    SYSTEM_DelayMs(500);




    SI4732_SetFreq(NOW_FREQ);


    SI4732_GET_INT_STATUS();

//        uint8_t cmd[2] = {0x42, 0x01};
//        SI4732_WriteBuffer(cmd, 2);
//        SI4732_WAIT_STATUS(0X80);
//        uint8_t cmd_read[8]={0};
//        SI4732_ReadBuffer(cmd_read,8);
//        show_hex(cmd_read[0],0);
//        show_hex(cmd_read[1],1);
//        show_hex(cmd_read[2],2);
//        show_hex(cmd_read[3],3);
//        show_hex(cmd_read[4],4);
//        show_hex(cmd_read[5],5);
//        show_hex(cmd_read[6],6);
    GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);

}

void SI4732_ReadBuffer(uint8_t *Value, uint8_t size) {
    I2C_Start();
    I2C_Write(0x23);
    I2C_ReadBuffer(Value, size);
    I2C_Stop();
}

void SI4732_WriteBuffer(uint8_t *buff, uint8_t size) {
    I2C_Start();
    I2C_Write(0x22);
    I2C_WriteBuffer(buff, size);
    I2C_Stop();
}


static void HandleUserInput() {
    kbds.prev = kbds.current;
    kbds.current = GetKey();
    if (kbds.current == KEY_INVALID) {
        kbds.counter = 0;
    }

    if (kbds.current != KEY_INVALID && kbds.current == kbds.prev) {
        if (kbds.counter < 16)
            kbds.counter++;
        else
            kbds.counter -= 3;
        SYSTEM_DelayMs(20);
    } else {
        kbds.counter = 0;
    }


    if (kbds.counter == 2 || kbds.counter == 16) {
        switch (NOW_MODE) {
            case FM:
                Key_FM(kbds);
                break;
            case AM:
                Key_FM(kbds);
                break;

        }

    }

}


static void Key_FM(KeyboardState kbds) {
    switch (kbds.current) {
        case KEY_UP:
            SI4732_SetFreq(NOW_FREQ + NOW_STEP);


            break;
        case KEY_DOWN:
            SI4732_SetFreq(NOW_FREQ - NOW_STEP);
            break;

        case KEY_STAR:
            NOW_STEP /= 10;
            if (NOW_STEP == 0)NOW_STEP = 1000;
            break;

        case KEY_F:
            NOW_MODE = 1 - NOW_MODE;
            if (NOW_MODE == FM)NOW_FREQ = 10760;
            else NOW_FREQ = 10000;
            SI4732_Init();
            break;

        default:
            break;
    }
}
