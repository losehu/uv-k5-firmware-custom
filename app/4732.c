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
#define AM_FREQ 15410
#define FM_FREQ 10760
uint32_t NOW_FREQ = AM_FREQ;// 9300 93MHZ 10000 10MHZ
enum SI4732_MODE_T NOW_MODE = AM;//0FM
uint8_t display_flag = 1;
uint8_t NOW_RSSI = 0;
uint8_t NOW_RSN = 0;
int c = 1;
int a=19;int b=20;

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
    sprintf(String, "%02d %02d %d %d", NOW_RSSI, NOW_RSN, a,b);
    UI_PrintStringSmall(String, 0, 127, 5);

//    ST7565_BlitStatusLine();  // blank status line
    ST7565_BlitFullScreen();
}

int d = 0;

void SI4732_SetFreq(uint32_t freq) {
    waitToSend();
    if (NOW_MODE == FM) {
        if (freq < 6400 || freq > 10800) return;
        NOW_FREQ = freq;
        uint8_t cmd[5] = {0x20, 0x00, (freq & 0xff00) >> 8, freq & 0x00ff, d};//设置频率
        SI4732_WriteBuffer(cmd, 5);

    } else if (NOW_MODE == AM) {


        if (freq < 149 || freq > 23000) return;

        NOW_FREQ = freq;
//        if(freq<1000) freq+=45;
//        else if(freq>1000) freq-=45;
//int a= 100;//560;
        uint8_t cmd[6] = {0x40, 0x00, (freq & 0xff00) >> 8, freq & 0x00ff, c >> 8, c & 0xff};//设置频率
        SI4732_WriteBuffer(cmd, 6);

//11.66 59 25
//115 25
//50 5~20
//6.15 69 14
//45 22
    }
    waitToSend();

    SYSTEM_DelayMs(30);
//    SYSTEM_DelayMs(500);
    RSQ_GET();

    display_flag = 1;


}

void SI4732_WAIT_STATUS(uint8_t state) {

    uint8_t tmp = 0;
    SI4732_ReadBuffer((uint8_t *) &tmp, 1);
    while (tmp != state) {
        SI4732_ReadBuffer((uint8_t *) &tmp, 1);
        show_uint32(tmp, 4);
    }
    display_flag=1;
}

void SI4732_Main() {
    SI4732_Init();
    BACKLIGHT_TurnOn();
    int cnt = 0;
//    int cnt = 1000000/2+1;
    while (1) {
        //设置频率

//        SI4732_SetFreq(10760);

//        if (cnt == 0) {
//            SI4732_SetFreq(9300);
//        } else if (cnt == 1000000/2) {
//            SI4732_SetFreq(10760);
//        }
//        cnt++;
//        if (cnt == 1000000)cnt = 0;


        if (cnt == 1350) {
            RSQ_GET();
            cnt = 0;
        }
        if (cnt % 5 == 0) {
            HandleUserInput();
        }

        cnt++;
        if (display_flag) {
            display_flag = 0;
            SI4732_Display();
        }
//                uint8_t cmd[2] = {0x42, 0x01};
//                if(NOW_MODE==FM)cmd[0]=0x22;
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
        SYSTEM_DelayMs(1);


    }
}


void SI4732_PowerUp() {
    waitToSend();
    uint8_t cmd[3] = {0x01, 0x10, 0x05};
    if (NOW_MODE == AM) {
        cmd[1] = 0x11;
    }
    SI4732_WriteBuffer(cmd, 3);
    SYSTEM_DelayMs(10);//10

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

int now = 0;

void RSQ_GET() {
    //AM_RSQ_STATUS
//    GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
    waitToSend();
    if (NOW_MODE == AM) {
        uint8_t cmd[2] = {0x43, 0x00};
        SI4732_WriteBuffer(cmd, 2);
    } else if (NOW_MODE == FM) {
        uint8_t cmd[2] = {0x23, 0x00};
        SI4732_WriteBuffer(cmd, 2);
    }


    waitToSend();
    uint8_t cmd_read[6] = {0};
    SI4732_ReadBuffer(cmd_read, 6);
    NOW_RSSI = cmd_read[4];
    NOW_RSN = cmd_read[5];
    display_flag = 1;
//    GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);

}
void sendProperty(uint16_t propertyNumber, uint16_t parameter)
{
    waitToSend();

    uint8_t tmp[6]={0x12,0,propertyNumber>>8,propertyNumber&0xff,parameter>>8,parameter&0xff};
    SI4732_WriteBuffer(tmp,6);
    SYSTEM_DelayMs(550);

}

void setVolume(uint8_t volume)
{
#define RX_VOLUME 0x4000
if(volume<0)volume=0;
    if(volume>63)volume=63;
    sendProperty(RX_VOLUME, volume);

}
void waitToSend()
{
    uint8_t tmp=0;
    do
    {
        SYSTICK_DelayUs(300);
        SI4732_ReadBuffer((uint8_t *)&tmp,1);
    } while (!(tmp & 0x80)||tmp&0x40);
}
void setAvcAmMaxGain(uint8_t gain)
{
    if (gain < 12 || gain > 90)
        return;
    sendProperty(0x3103, gain * 340);
}
void  AM_FRONTEND_AGC_CONTROL(uint8_t MIN_GAIN_INDEX,uint8_t ATTN_BACKUP)
{
  uint16_t num=MIN_GAIN_INDEX<<8|ATTN_BACKUP;
    sendProperty(0x3705, num);
}
void setAmSoftMuteMaxAttenuation(uint8_t smattn )
{
    sendProperty(0x3302, smattn);
}
void setBandwidth(uint8_t AMCHFLT, uint8_t AMPLFLT)
{
    waitToSend();


    uint8_t tmp[6]={0x12,0,0x31,0x02,AMCHFLT,AMPLFLT};
    SI4732_WriteBuffer(tmp,6);


    waitToSend();

}
void SI4732_Init() {
    SYSTEM_DelayMs(SI4732_DELAY_MS);


    SI4732_PowerDown();
    SYSTEM_DelayMs(SI4732_DELAY_MS);

    GPIO_ClearBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);

    RST_LOW;
    SYSTEM_DelayMs(10);
    RST_HIGH;
    SYSTEM_DelayMs(10);



    SI4732_PowerUp();

    GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
    setVolume(63); // Default volume level.



    if (NOW_MODE == AM) {
        SYSTEM_DelayMs(250);
        setAvcAmMaxGain(48);
        SYSTEM_DelayMs(500);

////      AM_DEEMPHASIS
//        uint8_t cmd1[6] = {0x12, 0x00, 0x31, 0x00, 0x00, 0x01};
//        SI4732_WriteBuffer(cmd1, 6);
//        SI4732_WAIT_STATUS(0x80);
//        SYSTEM_DelayMs(100);

// AM_AGC_OVERRIDE
waitToSend();
        uint8_t cmd2[6] = {0x48, 0x01, 0};
        SI4732_WriteBuffer(cmd2, 3);
        waitToSend();

        setAmSoftMuteMaxAttenuation(8);


        setBandwidth(2,1);

        setAvcAmMaxGain(38);

////AM_FRONTEND_AGC_CONTROL
//        uint8_t cmd5[6] = {0x12, 0x00, 0x37, 0x05, 19, 20};
//        SI4732_WriteBuffer(cmd5, 6);
//        SYSTEM_DelayMs(100);
        AM_FRONTEND_AGC_CONTROL(10,12);


//// AM_AUTOMATIC_VOLUME_CONTROL_MAX_GAIN
//        uint8_t cmd8[6] = {0x12, 0x00,0x31, 0x03, 16320>>8,16320&0xff};
//        SI4732_WriteBuffer(cmd8, 6);
//        SI4732_WAIT_STATUS(0x80);
//        SYSTEM_DelayMs(100);

//        uint8_t cmd4[6] = {0x12, 0x00, 0x33, 0x02, 0x00, 0};
//        SI4732_WriteBuffer(cmd4, 6);
//        SI4732_WAIT_STATUS(0x80);
//        SYSTEM_DelayMs(100);

//    uint8_t cmd4[6] = {0x12, 0x00, 0x31,0x03,0x78,0x00};
//    SI4732_WriteBuffer(cmd4, 6);
//    SI4732_WAIT_STATUS(0x80);
//    SYSTEM_DelayMs(100);
    }else
    {
        // FM_AGC_OVERRIDE
        waitToSend();
        uint8_t cmd2[6] = {0x28, 0x01, 0};
        SI4732_WriteBuffer(cmd2, 3);
        SYSTEM_DelayMs(100);

////AM_FRONTEND_AGC_CONTROL
//        uint8_t cmd5[6] = {0x12, 0x00, 0x37, 0x05, 0, 0};
//        SI4732_WriteBuffer(cmd5, 6);
//        SYSTEM_DelayMs(100);

    }
//    //  AM_CHANNEL_FILTER
//    uint8_t cmd1[6] = {0x12, 0x00, 0x31,0x02,0x01,0x00};
//    SI4732_WriteBuffer(cmd1, 6);
//    SI4732_WAIT_STATUS(0x80);
//    SYSTEM_DelayMs(500);
//
////音量
//    uint8_t cmd_vol[6] = {0x12, 0x00, 0x40, 0x00, 0x00, 0x3f};
//    SI4732_WriteBuffer(cmd_vol, 6);
//    SI4732_WAIT_STATUS(0x80);
//    uint8_t b;
//    SI4732_ReadBuffer((uint8_t *) &b, 1);

//    while (b != 128) {
//        SI4732_ReadBuffer((uint8_t *) &b, 1);
//        show_uint32((uint32_t) b, 0);
//    }

//    SYSTEM_DelayMs(100);

////声道
//    uint8_t cmd_hear[6] = {0x12, 0x00, 0x40, 0x01, 0x00, 0x00};
//    SI4732_WriteBuffer(cmd_hear, 6);
//    SI4732_ReadBuffer((uint8_t *) &b, 1);
//    while (b != 128) {
//        SI4732_ReadBuffer((uint8_t *) &b, 1);
//        show_uint32((uint32_t) b, 0);
//    }
//    SYSTEM_DelayMs(100);


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
//    SYSTEM_DelayMs(200);

    SYSTEM_DelayMs(100);

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
//            c+=20;
//            d += 1;
            SI4732_SetFreq(NOW_FREQ +  NOW_STEP);


            break;
        case KEY_DOWN:
//            c-=20;
//            d -= 1;
            SI4732_SetFreq(NOW_FREQ - NOW_STEP);
            break;

        case KEY_STAR:
            NOW_STEP /= 10;
            if (NOW_STEP == 0)NOW_STEP = 1000;
            break;

        case KEY_F:
            NOW_MODE = 1 - NOW_MODE;
            if (NOW_MODE == FM)NOW_FREQ = FM_FREQ;
            else NOW_FREQ = AM_FREQ;
            SI4732_Init();
            break;
        case KEY_1:
            a++;
            AM_FRONTEND_AGC_CONTROL(a,b);
            break;

        case KEY_7:
            a--;
            AM_FRONTEND_AGC_CONTROL(a,b);
            break;

        case KEY_2:
            b++;
            AM_FRONTEND_AGC_CONTROL(a,b);
            break;

        case KEY_8:
            b--;
            AM_FRONTEND_AGC_CONTROL(a,b);
            break;


        default:
            break;
    }
}
