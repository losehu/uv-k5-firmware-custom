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
uint32_t NOW_STEP=10;
uint32_t NOW_FREQ = 10760;//93MHZ
enum SI4732_MODE_T NOW_MODE = FM;//0FM
uint8_t display_flag=1;
void SI4732_Display() {
    memset(gStatusLine, 0, sizeof(gStatusLine));
    UI_DisplayClear();
    uint8_t String[19];
    if (NOW_MODE == FM) {

        sprintf(String, "%3d.%02d", NOW_FREQ / 100, NOW_FREQ % 100);
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
    }
    SYSTEM_DelayMs(50);
    display_flag=1;


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

        if (display_flag ) {
            display_flag=0;
            SI4732_Display();
        }

    }
}




void SI4732_PowerUp() {
    // 发送POWER_UP命令
    uint8_t cmd[3] = {0x01, 0x10, 0x05};
    SI4732_WriteBuffer(cmd, 3);
    SI4732_WAIT_STATUS(0x80);
    SYSTEM_DelayMs(500);

}
void SI4732_SwitchMode()
{

}
void SI4732_Init() {


    RST_HIGH;

    SI4732_PowerUp();
    SI4732_SetFreq(10760);
    uint8_t state = 0;
    while (state != 0x81) {
        uint8_t cmd3[1] = {0x14};
        SI4732_WriteBuffer(cmd3, 1);
        SI4732_ReadBuffer((uint8_t *) &state, 1);
    }
    SYSTEM_DelayMs(SI4732_DELAY_MS);

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
                break;

        }

    }

}


static void Key_FM(KeyboardState kbds) {
    switch (kbds.current) {
        case KEY_UP:
            SI4732_SetFreq(NOW_FREQ+NOW_STEP);


            break;
        case KEY_DOWN:
            SI4732_SetFreq(NOW_FREQ-NOW_STEP);
            break;
   
        default:
            break;
    }
}
