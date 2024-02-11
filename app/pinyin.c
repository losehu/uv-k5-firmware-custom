//
// Created by RUPC on 2024/2/9.
//
#include "app/pinyin.h"
#include "string.h"
#include "driver/eeprom.h"
#include "ui/helper.h"
#include "driver/uart.h"
#include "bitmaps.h"
#include "driver/st7565.h"
#include "driver/gpio.h"

#include "driver/keyboard.h"
#include "functions.h"
#include "stdbool.h"
#include "ui/menu.h"

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

//char edit_original[17]; // a copy of the text before editing so that we can easily test for changes/difference
//char edit[17];
//int edit_index;

char input[22];
uint8_t  sum_pxl=0;

KeyboardState kbd1 = {KEY_INVALID, KEY_INVALID, 0};
uint8_t PINYIN_MODE = 0;
uint8_t INPUT_MODE = 0;

uint8_t pinyin_cmp(uint8_t *a, uint8_t *b) {
    for (int i = 0; i < 6; i++) {
        if (a[i] > b[i]) return 1;
        else if (a[i] < b[i]) return 0;
    }
    return 2;
}

uint8_t pinyin_search(uint8_t *target, uint8_t size, uint32_t *add) {
    int low = 0;
    int high = 398;
    uint8_t target_all[6];
    memset(target_all, ' ', 6);
    memcpy(target_all, target, size);
    while (low <= high) {
        int mid = low + (high - low) / 2;
        uint8_t tmp[11];
        EEPROM_ReadBuffer(0x20000 + mid * 16, tmp, 11);


        uint8_t cmp = pinyin_cmp(tmp, target_all);

        if (cmp == 2) {
            *add = tmp[7] | (tmp[8] << 8) | (tmp[9] << 16) | (tmp[10] << 24);
            return tmp[6];
        } else if (cmp == 0) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return 255; // 如果未找到目标元素，返回-1
}

void SHOW_ARRAY(uint8_t way)// 0up 1 down
{

    if (way)
        memcpy(&gFrameBuffer[6] + 123, BITMAP_ARRAY_DOWN, 5);
    else
        memcpy(&gFrameBuffer[5] + 123, BITMAP_ARRAY_UP, 5);

}

static void IN_PINYIN(uint8_t key) {

}
uint8_t EN_SELECT=0;
char * EN_LIST;

static void IN_EN(uint8_t key) {

    switch (key) {
        case KEY_1...KEY_9:
            if(edit_index<MAX_EDIT_INDEX) {
                edit[edit_index++] = key + '0';
                sum_pxl+=7;
            }
            break;
        case KEY_STAR:
            if(edit_index) {
                sum_pxl-=7;
                edit_index--;
            }
            break;
        case KEY_UP:
//            if(edit[edit_index]>)
            break;
        case KEY_DOWN:
            break;
    }
    edit[edit_index] = '\0';

}

static void IN_NUM(uint8_t key) {
    switch (key) {


    }


}

static void OnKeyIN(uint8_t key) {
    switch (key) {
        case KEY_0 :
            INPUT_MODE = 1- INPUT_MODE;
            break;

        default:
            if (INPUT_MODE == 1)IN_PINYIN(key);
            else if (INPUT_MODE == 0) IN_EN(key);
            break;
    }


}

static void OnKeyChoose(uint8_t key) {
    switch (key) {
        case KEY_3:
            break;
        case KEY_9:
            break;
        case KEY_1:
            break;
        case KEY_7:
            break;
        case KEY_2:
            break;
        case KEY_8:
            break;
        case KEY_UP:
            break;
        case KEY_DOWN:
            break;
        case KEY_SIDE1:
            break;
        case KEY_STAR:
            break;
        case KEY_F:
            break;
        case KEY_5:
            break;
        case KEY_0:
            break;
        case KEY_6:
            break;
        case KEY_4:
            break;
        case KEY_SIDE2:
            break;
        case KEY_PTT:
            break;
        case KEY_MENU:
            break;
        case KEY_EXIT:
            break;
        default:
            break;
    }
}

bool HandleUserInput() {
    kbd1.prev = kbd1.current;
    kbd1.current = GetKey();


    if (kbd1.current != KEY_INVALID && kbd1.current == kbd1.prev) {
        if (kbd1.counter < 16)
            kbd1.counter++;
        else
            kbd1.counter -= 3;
        SYSTEM_DelayMs(20);
    } else {
        kbd1.counter = 0;
    }


    if (kbd1.counter == 2 || kbd1.counter == 16) {
        switch (PINYIN_MODE) {
            case 0: //输入拼音
                OnKeyIN(kbd1.current);
                break;
            case 1://检索汉字
                OnKeyChoose(kbd1.current);
                break;

        }
    }
    return true;
}

void SHOW_INPUT() {
    UI_PrintStringSmall(input, 0, 0, 4);

}

void SHOW_CHOOSE() {

}

void SHOW_SCREEN() {
    UI_DisplayClear();
    char string[17] = {0};
    strcpy(string, edit);
    show_move_flag = 1;
    UI_PrintStringSmall(string, 0, 127, 1);
    SHOW_INPUT();
    if (PINYIN_MODE) SHOW_CHOOSE();
    if (INPUT_MODE == 1)memcpy(&gFrameBuffer[0][120], BITMAP_CN, 7);
    else  UI_PrintStringSmall("A", 120, 0, 0);

    ST7565_BlitFullScreen();

}

void PINYIN() {
//    uint32_t add;
//    uint8_t name[6]="gen";
//   uint8_t num= pinyin_search(name,3,&add);
//    show_uint32(num,0);
//    show_uint32(add,1);
//    char a[119]={0};
//    EEPROM_ReadBuffer(add,a,num*2);
//
//    UI_PrintStringSmall("1你2她3我4脏5其6把",0,0,5);


//    while (1) {
//        HandleUserInput();
//        SHOW_SCREEN();
//    }

}