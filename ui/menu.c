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
#include "driver/eeprom.h"
#include <string.h>
#include <stdlib.h>  // abs()
#include "bitmaps.h"
#include "driver/uart.h"
#include "app/dtmf.h"
#include "app/menu.h"
#include "board.h"
#include "dcs.h"
#include "driver/backlight.h"
#include "driver/bk4819.h"
#include "driver/st7565.h"
#include "external/printf/printf.h"
#include "frequencies.h"
#include "helper/battery.h"
#include "misc.h"
#include "settings.h"
#include "ui/helper.h"
#include "ui/inputbox.h"
#include "ui/menu.h"
#include "ui/ui.h"
#include "chinese.h"

void insertNewline(char a[], int index, int len) {

    if (index < 0 || index >= len || len >= 63) {
        return;
    }
    for (int i = len; i >= index; i--) {
        a[i + 1] = a[i];
    }
    a[index] = '\n';
    a[len + 1] = '\0'; // Null-terminate the string
}

const t_menu_item MenuList[] =
        {
//   text,     voice ID,                               menu ID
                {/*"Step",*/   VOICE_ID_FREQUENCY_STEP, MENU_STEP, 步进频率},
                {/*"RxDCS",*/  VOICE_ID_DCS, MENU_R_DCS, 接收数字亚音}, // was "R_DCS"
                {/*"RxCTCS",*/ VOICE_ID_CTCSS, MENU_R_CTCS, 接收模拟亚音}, // was "R_CTCS"
                {/*"TxDCS",*/  VOICE_ID_DCS, MENU_T_DCS, 发送数字亚音}, // was "T_DCS"
                {/*"TxCTCS",*/ VOICE_ID_CTCSS, MENU_T_CTCS, 发送模拟亚音}, // was "T_CTCS"
                {/*"TxODir",*/ VOICE_ID_TX_OFFSET_FREQUENCY_DIRECTION, MENU_SFT_D, 频差方向}, // was "SFT_D"
                {/*"TxOffs",*/ VOICE_ID_TX_OFFSET_FREQUENCY, MENU_OFFSET, 频差频率}, // was "OFFSET"
#ifdef ENABLE_CUSTOM_SIDEFUNCTIONS

                {/*"W/N",*/    VOICE_ID_CHANNEL_BANDWIDTH,             MENU_W_N           ,宽窄带},
#endif

                {/*"Scramb",*/ VOICE_ID_SCRAMBLER_ON, MENU_SCR, 加密}, // was "SCR"
                {/*"BusyCL",*/ VOICE_ID_BUSY_LOCKOUT, MENU_BCL, 遇忙禁发}, // was "BCL"
                {/*"Compnd",*/ VOICE_ID_INVALID, MENU_COMPAND, 压扩},
                {/*"ChSave",*/ VOICE_ID_MEMORY_CHANNEL, MENU_MEM_CH, 存置信道}, // was "MEM-CH"
                {/*"ChDele",*/ VOICE_ID_DELETE_CHANNEL, MENU_DEL_CH, 删除信道}, // was "DEL-CH"
                {/*"ChName",*/ VOICE_ID_INVALID, MENU_MEM_NAME, 命名信道},
                {/*"SList",*/  VOICE_ID_INVALID, MENU_S_LIST, 信道扫描列表},
                {/*"SList1",*/ VOICE_ID_INVALID, MENU_SLIST1, 扫描列表1},
                {/*"SList2",*/ VOICE_ID_INVALID, MENU_SLIST2, 扫描列表2},
                {/*"ScnRev",*/ VOICE_ID_INVALID, MENU_SC_REV, 搜索恢复模式},
                {/*"TxTOut",*/ VOICE_ID_TRANSMIT_OVER_TIME, MENU_TOT, 发送超时}, // was "TOT"
                {/*"BatSav",*/ VOICE_ID_SAVE_MODE, MENU_SAVE, 省电模式}, // was "SAVE"
                {/*"Mic",*/    VOICE_ID_INVALID, MENU_MIC, 麦克风增益},
                {/*"ChDisp",*/ VOICE_ID_INVALID, MENU_MDF, 信道显示模式}, // was "MDF"
#if ENABLE_CHINESE_FULL == 4
                {/*"POnMsg",*/ VOICE_ID_INVALID,                       MENU_PONMSG        ,开机显示},
#endif
                {/*"BackLt",*/ VOICE_ID_INVALID, MENU_ABR, 自动背光}, // was "ABR"
                {/*"BLMax",*/  VOICE_ID_INVALID, MENU_ABR_MAX, 背光亮度},
                {/*"MDCID",*/  VOICE_ID_INVALID, MENU_MDC_ID, MDC_ID},

                {/*"Roger",*/  VOICE_ID_INVALID, MENU_ROGER, 首尾音},

                {/*"STE",*/    VOICE_ID_INVALID, MENU_STE, 尾音消除},
                {/*"RP STE",*/ VOICE_ID_INVALID, MENU_RP_STE, 过中继尾音消除},
                {/*"1 Call",*/ VOICE_ID_INVALID, MENU_1_CALL, 按键即呼},

#ifdef ENABLE_CUSTOM_SIDEFUNCTIONS
                {/*"F1Shrt",*/ VOICE_ID_INVALID,                       MENU_F1SHRT        ,侧键1短按},
                {/*"F1Long",*/ VOICE_ID_INVALID,                       MENU_F1LONG        ,侧键1长按},
                {/*"F2Shrt",*/ VOICE_ID_INVALID,                       MENU_F2SHRT        ,侧键2短按},
                {/*"F2Long",*/ VOICE_ID_INVALID,                       MENU_F2LONG        ,侧键2长按},
                {/*"M Long",*/ VOICE_ID_INVALID,                       MENU_MLONG         ,M键长按},
#endif

#ifdef ENABLE_DTMF_CALLING

                {/*"ANI ID",*/ VOICE_ID_ANI_CODE,                      MENU_ANI_ID        ,DTMF_ID},
#endif
                {/*"UPCode",*/ VOICE_ID_INVALID, MENU_UPCODE, DTMF上线码},
                {/*"DWCode",*/ VOICE_ID_INVALID, MENU_DWCODE, DTMF下线码},
                {/*"PTT ID",*/ VOICE_ID_INVALID, MENU_PTT_ID, DTMF发送},
                {/*"D ST",*/   VOICE_ID_INVALID, MENU_D_ST, DTMF侧音},
#ifdef ENABLE_DTMF_CALLING

                {/*"D Resp",*/ VOICE_ID_INVALID,                       MENU_D_RSP         ,DTMF响应},
                {/*"D Hold",*/ VOICE_ID_INVALID,                       MENU_D_HOLD        ,DTMF复位},
#endif
                {/*"D Prel",*/ VOICE_ID_INVALID, MENU_D_PRE, DTMF预载波},
#ifdef ENABLE_DTMF_CALLING
#ifdef ENABLE_CUSTOM_SIDEFUNCTIONS

                {/*"D Decd",*/ VOICE_ID_INVALID,                       MENU_D_DCD         ,DTMF解码},
#endif
                {/*"D List",*/ VOICE_ID_INVALID,                       MENU_D_LIST        ,DTMF联系人},
#endif
                {/*"D Live",*/ VOICE_ID_INVALID, MENU_D_LIVE_DEC, DTMF显示}, // live DTMF decoder
#ifdef ENABLE_AM_FIX//1
                {/*"AM Fix",*/ VOICE_ID_INVALID,                       MENU_AM_FIX        ,AM自动增益},
#endif
#ifdef ENABLE_AM_FIX_TEST1//0
                {/*"AM FT1",*/ VOICE_ID_INVALID,                       MENU_AM_FIX_TEST1  ,""},
#endif

                {/*"RxMode",*/ VOICE_ID_DUAL_STANDBY, MENU_TDR, 收发模式},
                {/*"Sql",*/    VOICE_ID_SQUELCH, MENU_SQL, 静噪等级},

                // hidden menu items from here on
                // enabled if pressing both the PTT and upper side button at power-on
                {/*"F Lock",*/ VOICE_ID_INVALID, MENU_F_LOCK, 频段解锁},
//                {/*"Tx 200",*/ VOICE_ID_INVALID,                       MENU_200TX         ,两百M发射}, // was "200TX"
//                {/*"Tx 350",*/ VOICE_ID_INVALID,                       MENU_350TX         ,三百五十M发射}, // was "350TX"
//                {/*"Tx 500",*/ VOICE_ID_INVALID,                       MENU_500TX         ,五百M发射}, // was "500TX"
//                {/*"350 En",*/ VOICE_ID_INVALID,                       MENU_350EN         ,三百五十M接收}, // was "350EN"
#ifdef ENABLE_F_CAL_MENU//0
                {/*"FrCali",*/ VOICE_ID_INVALID,                       MENU_F_CALI        ,""}, // reference xtal calibration
#endif
                {/*"BatCal",*/ VOICE_ID_INVALID, MENU_BATCAL, 电池调压}, // battery voltage calibration
                {/*"BatTyp",*/ VOICE_ID_INVALID, MENU_BATTYP, 电池大小}, // battery type 1600/2200mAh
                {/*"Reset",*/  VOICE_ID_INITIALISATION, MENU_RESET,
                               参数复位}, // might be better to move this to the hidden menu items ?

                {/*"",*/       VOICE_ID_INVALID, 0xff, "\x00"}  // end of list - DO NOT delete or move this this
        };

#ifdef ENABLE_CUSTOM_SIDEFUNCTIONS
#if ENABLE_CHINESE_FULL==0 || defined(ENABLE_ENGLISH)
#ifdef ENABLE_ENGLISH
const char gSubMenu_W_N[][7] =//7

#else
const char gSubMenu_W_N[][3] =//7

#endif
#else
        const char gSubMenu_W_N[][5] =//7
#endif
        {
//                "WIDE",
//                "NARROW"
                宽带,
               窄带
        };
#endif
#if ENABLE_CHINESE_FULL == 4
const char gSubMenu_PONMSG[][5]={
        关闭,
        图片,
        信息
};
#endif
#if ENABLE_CHINESE_FULL != 4 || defined(ENABLE_ENGLISH)
#ifdef ENABLE_ENGLISH
const char gSubMenu_SFT_D[][4] =

#else
const char gSubMenu_SFT_D[][10] =//4
#endif
#else
        const char gSubMenu_SFT_D[][16] =//4
#endif
        {
//                "OFF",
//                "+",
//                "-"
                发送等于接收,
                发送等于接收加偏移,
                发送等于接收减偏移

        };


#if ENABLE_CHINESE_FULL != 4 || defined(ENABLE_ENGLISH)
#ifdef ENABLE_ENGLISH
const char gSubMenu_OFF_ON[][4] =

#else
const char gSubMenu_OFF_ON[][3] =//4
#endif

#else
        const char gSubMenu_OFF_ON[][5] =//4
#endif
        {
//                "OFF",
//                "ON"
                关闭,
                开启
        };
#if ENABLE_CHINESE_FULL != 4 || defined(ENABLE_ENGLISH)

const char gSubMenu_SAVE[][4] =//4
#else
        const char gSubMenu_SAVE[][6] =//4
#endif
        {
//                "OFF",
//                "1:1",
//                "1:2",
//                "1:3",
//                "1:4"

                关闭,
                一级,
                二级,
                三级,
                四级

        };
#if ENABLE_CHINESE_FULL != 4 || defined(ENABLE_ENGLISH)
const char gSubMenu_TOT[][7] = //7
#else
        const char gSubMenu_TOT[][6] = //7
#endif
        {
//                "30 sec",
//                "1 min",
//                "2 min",
//                "3 min",
//                "4 min",
//                "5 min",
//                "6 min",
//                "7 min",
//                "8 min",
//                "9 min",
//                "15 min"

                三十秒,
                一分,
                两分,
                三分,
                四分,
                五分,
                六分,
                七分,
                八分,
                九分,
                十五分

        };

const char *const gSubMenu_RXMode[] =
        {

//                "MAIN\nONLY",        // TX and RX on main only
//                "DUAL RX\nRESPOND", // Watch both and respond
//                "CROSS\nBAND",        // TX on main, RX on secondary
//                "MAIN TX\nDUAL RX"    // always TX on main, but RX on both
                主信道接收发射,        // TX and RX on main only
                双信道接收, // Watch both and respond
                主信道发射副信道接收,        // TX on main, RX on secondary
                主信道发射双信道接收    // always TX on main, but RX on both

        };

#ifdef ENABLE_VOICE
const char gSubMenu_VOICE[][4] =
{
    "OFF",
    "CHI",
    "ENG"
};
#endif
#if ENABLE_CHINESE_FULL != 4 || defined(ENABLE_ENGLISH)
#ifdef ENABLE_ENGLISH
const char gSubMenu_SC_REV[][8] =//8

#else
const char gSubMenu_SC_REV[][10] =//8
#endif

#else
        const char gSubMenu_SC_REV[][18] =//8
#endif
        {
//                "TIMEOUT",
//                "CARRIER",
//                "STOP"
                遇信号5秒后搜索,
                信号停止后搜索,
                遇信号后停止搜索

        };

const char *const gSubMenu_MDF[] =
        {
//                "FREQ",
//                "CHANNEL\nNUMBER",
//                "NAME",
//                "NAME\n+\nFREQ"
                频率,
                信道号,
                名称,
                名称加频率
        };

#ifdef ENABLE_ALARM
const char gSubMenu_AL_MOD[][5] =
{
    "SITE",
    "TONE"
};
#endif
#ifdef ENABLE_DTMF_CALLING
#if ENABLE_CHINESE_FULL!=4 || defined(ENABLE_ENGLISH)

#ifdef ENABLE_ENGLISH
const char gSubMenu_D_RSP[][11] =//11

#else
const char gSubMenu_D_RSP[][10] =//11
#endif
#else
const char gSubMenu_D_RSP[][18] =//11
#endif
        {
//                "DO\nNOTHING",
//                "RING",
//                "REPLY",
//                "BOTH"
                不响应,
                本地响铃,
                回复响应,
               本地响铃回复响应
        };
#endif

const char *const gSubMenu_PTT_ID[] =
        {
//                "OFF",
//                "UP CODE",
//                "DOWN CODE",
//                "UP+DOWN\nCODE",
//                "APOLLO\nQUINDAR"
                不发送,
                上线码,
                下线码,
                上线加下线码,
                Quindar码
        };


#if ENABLE_CHINESE_FULL != 4 || defined(ENABLE_ENGLISH)


#ifdef ENABLE_ENGLISH
const char gSubMenu_ROGER[][15] =

#else
const char gSubMenu_ROGER[][13] =
#endif
#else
        const char gSubMenu_ROGER[][15] =
#endif
        {
//                "OFF",
//                "ROGER",
//                "MDC"

                关闭,
                ROGER尾音,
                MDC尾音,
                MDC首音,
                MDC首尾音,
                MDC首音加ROGER
        };
#if ENABLE_CHINESE_FULL != 4 || defined(ENABLE_ENGLISH)

#ifdef ENABLE_ENGLISH
const char gSubMenu_RESET[][4] =//4

#else
const char gSubMenu_RESET[][6] =//4
#endif

#else
        const char gSubMenu_RESET[][11] =//4
#endif
        {
//                "VFO",
//                "ALL"
                除信道参数,
                全部参数
        };

const char *const gSubMenu_F_LOCK[] =
        {
                "DEFAULT+\n137-174\n400-470",
                "FCC HAM\n144-148\n420-450",
                "CE HAM\n144-146\n430-440",
                "GB HAM\n144-148\n430-440",

//                "DISABLE\nALL",
//                "UNLOCK\nALL",
                禁用全部,
                解锁全部,
        };
#if ENABLE_CHINESE_FULL != 4 || defined(ENABLE_ENGLISH)

#ifdef ENABLE_ENGLISH
const char gSubMenu_BACKLIGHT[][7] =//7

#else
const char gSubMenu_BACKLIGHT[][5] =//7
#endif
#else
        const char gSubMenu_BACKLIGHT[][6] =//7
#endif
        {
//                "OFF",
//                "5 sec",
//                "10 sec",
//                "20 sec",
//                "1 min",
//                "2 min",
//                "4 min",
//                "ON"
                关闭,
                五秒,
                十秒,
                二十秒,
                一分,
                两分,
                四分,
                开启

        };
#if ENABLE_CHINESE_FULL != 4 || defined(ENABLE_ENGLISH)


#ifdef ENABLE_ENGLISH
const char gSubMenu_RX_TX[][6] =//6

#else
const char gSubMenu_RX_TX[][7] =//6
#endif
#else
        const char gSubMenu_RX_TX[][12] =//6
#endif
        {
//                "OFF",
//                "TX",
//                "RX",
//                "TX/RX"
                关闭,
                发送时,
                接收时,
                发送接收时
        };

#ifdef ENABLE_AM_FIX_TEST1
const char gSubMenu_AM_fix_test1[][8] =
{
    "LNA-S 0",
    "LNA-S 1",
    "LNA-S 2",
    "LNA-S 3"
};
#endif


const char gSubMenu_BATTYP[][8] =
        {
                "1600mAh",
                "2200mAh"
        };

const char gSubMenu_SCRAMBLER[][7] =
        {
//                "OFF",
                关闭,

                "2600Hz",
                "2700Hz",
                "2800Hz",
                "2900Hz",
                "3000Hz",
                "3100Hz",
                "3200Hz",
                "3300Hz",
                "3400Hz",
                "3500Hz"
        };

#ifdef ENABLE_CUSTOM_SIDEFUNCTIONS
const t_sidefunction SIDEFUNCTIONS[] =
        {
               {关闭, ACTION_OPT_NONE},
#ifdef ENABLE_FLASHLIGHT
               {手电, ACTION_OPT_FLASHLIGHT},
#endif
               {切换发射功率, ACTION_OPT_POWER},
               {监听, ACTION_OPT_MONITOR},
               {扫描, ACTION_OPT_SCAN},
#ifdef ENABLE_VOX
               {声控发射,				ACTION_OPT_VOX},
#endif
#ifdef ENABLE_ALARM
                {"ALARM",			ACTION_OPT_ALARM},
#endif
#ifdef ENABLE_FMRADIO
               {FM收音机,		ACTION_OPT_FM},
#endif
#ifdef ENABLE_TX1750
                {"1750HZ",			ACTION_OPT_1750},
#endif
               {锁定按键, ACTION_OPT_KEYLOCK},
               {切换信道, ACTION_OPT_A_B},
               {切换信道模式, ACTION_OPT_VFO_MR},
               {切换调制模式, ACTION_OPT_SWITCH_DEMODUL},
               {DTMF解码, ACTION_OPT_D_DCD},
               {切换宽窄带, ACTION_OPT_WIDTH},
#ifdef ENABLE_SIDEFUNCTIONS_SEND
               {主信道发射, ACTION_OPT_SEND_CURRENT},
               {副信道发射, ACTION_OPT_SEND_OTHER},
#endif
#ifdef ENABLE_BLMIN_TMP_OFF
                {"BLMIN\nTMP OFF",  ACTION_OPT_BLMIN_TMP_OFF}, 		//BackLight Minimum Temporay OFF
#endif
        };
const t_sidefunction *gSubMenu_SIDEFUNCTIONS = SIDEFUNCTIONS;
const uint8_t gSubMenu_SIDEFUNCTIONS_size = ARRAY_SIZE(SIDEFUNCTIONS);
#endif

bool gIsInSubMenu;
uint8_t gMenuCursor;

int UI_MENU_GetCurrentMenuId() {

    if (gMenuCursor < ARRAY_SIZE(MenuList))
        return MenuList[gMenuCursor].menu_id;
    else
        return MenuList[ARRAY_SIZE(MenuList) - 1].menu_id;
}

uint8_t UI_MENU_GetMenuIdx(uint8_t id) {
    for (uint8_t i = 0; i < ARRAY_SIZE(MenuList); i++)
        if (MenuList[i].menu_id == id)
            return i;
    return 0;
}

int32_t gSubMenuSelection;

// edit box
char edit_original[17]; // a copy of the text before editing so that we can easily test for changes/difference
char edit[17];
int edit_index;


void UI_DisplayMenu(void) {
    const unsigned int menu_list_width = 6; // max no. of characters on the menu list (left side)
    const unsigned int menu_item_x1 = (8 * menu_list_width);//+ 2;
    const unsigned int menu_item_x2 = LCD_WIDTH - 1;
    unsigned int i;
    char String[64];  // bigger cuz we can now do multi-line in one string (use '\n' char)
#ifdef ENABLE_DTMF_CALLING
    char               Contact[16];
#endif

    // clear the screen buffer
    UI_DisplayClear();

#if 1
    // original menu layout



    // invert the current menu list item pixels反转当前菜单项的像素值 ：


    // draw vertical separating dotted line绘制垂直分隔的点线 ：
//    for (i = 0; i < 7; i++)
//        gFrameBuffer[i][(8 * menu_list_width) + 1] = 0xAA;


    // draw the little sub-menu triangle marker绘制子菜单三角标志：
    //const void *BITMAP_CurrentIndicator = BITMAP_MARKER;

    if (gIsInSubMenu)
        memmove(gFrameBuffer[2] + 41, BITMAP_VFO_Default, sizeof(BITMAP_VFO_Default));
#ifndef ENABLE_MDC1200
    uint8_t add = 1;

    if (gMenuCursor + 1 >= 26)
        add = 0;

    sprintf(String, "%2u/%u", add + gMenuCursor, gMenuListCount - 1);

#else
    sprintf(String, "%2u/%u", 1 + gMenuCursor, gMenuListCount);
#endif

#ifdef ENABLE_PINYIN //拼音取消显示
    if (!(UI_MENU_GetCurrentMenuId() == MENU_MEM_NAME && gIsInSubMenu && gIsInSubMenu && edit_index >= 0))
#endif
    UI_PrintStringSmall(String, 2, 0, 6);
#ifdef ENABLE_PINYIN//拼音取消显示
    if (!(UI_MENU_GetCurrentMenuId() == MENU_MEM_NAME && gIsInSubMenu && edit_index >= 0))
#endif


#ifdef ENABLE_ENGLISH
    {
    uint8_t size_menu = strlen(MenuList[gMenuCursor].name)*7;
    UI_PrintStringSmall(MenuList[gMenuCursor].name, size_menu < 48 ? (48 - size_menu) / 2 : 0, 0, 0);
    }
#else
    {
        UI_ShowChineseMenu();
    }
#endif

#else
    {	// new menu layout .. experimental & unfinished

        const int menu_index = gMenuCursor;  // current selected menu item
        i = 1;

        if (!gIsInSubMenu)
        {
            while (i < 2)
            {	// leading menu items - small text
                const int k = menu_index + i - 2;
                if (k < 0)
                    UI_PrintStringSmall(MenuList[gMenuListCount + k].name, 0, 0, i);  // wrap-a-round
                else
                if (k >= 0 && k < (int)gMenuListCount)
                    UI_PrintStringSmall(MenuList[k].name, 0, 0, i);
                i++;
            }

            // current menu item - keep big n fat
            if (menu_index >= 0 && menu_index < (int)gMenuListCount)
                UI_PrintStringSmall(MenuList[menu_index].name, 0, 0, 2);
            i++;

            while (i < 4)
            {	// trailing menu item - small text
                const int k = menu_index + i - 2;
                if (k >= 0 && k < (int)gMenuListCount)
                    UI_PrintStringSmall(MenuList[k].name, 0, 0, 1 + i);
                else
                if (k >= (int)gMenuListCount)
                    UI_PrintStringSmall(MenuList[gMenuListCount - k].name, 0, 0, 1 + i);  // wrap-a-round
                i++;
            }

            // draw the menu index number/count
            sprintf(String, "%2u.%u", 1 + gMenuCursor, gMenuListCount);
            UI_PrintStringSmall(String, 2, 0, 6);
        }
        else
        if (menu_index >= 0 && menu_index < (int)gMenuListCount)
        {	// current menu item

            UI_PrintStringSmall(MenuList[menu_index].name, 0, 0, 0);
//			UI_PrintStringSmall(String, 0, 0, 0);
        }
    }
#endif

    // **************

    memset(String, 0, sizeof(String));

    bool already_printed = false;
/* Brightness is set to max in some entries of this menu. Return it to the configured brightness
	   level the "next" time we enter here.I.e., when we move from one menu to another.
	   It also has to be set back to max when pressing the Exit key. */

    BACKLIGHT_TurnOn();
    switch (UI_MENU_GetCurrentMenuId()) {
        case MENU_SQL:
            sprintf(String, "%d", gSubMenuSelection);
            break;

        case MENU_MIC: {    // display the mic gain in actual dB rather than just an index number
            const uint8_t mic = gMicGain_dB2[gSubMenuSelection];
            sprintf(String, "+%u.%01udB", mic / 2, mic % 2);
        }
            break;

//#ifdef ENABLE_AUDIO_BAR
//            case MENU_MIC_BAR:
//                strcpy(String, gSubMenu_OFF_ON[gSubMenuSelection]);
//                break;
//#endif

        case MENU_STEP: {
            uint16_t step = gStepFrequencyTable[FREQUENCY_GetStepIdxFromSortedIdx(gSubMenuSelection)];
            sprintf(String, "%d.%02ukHz", step / 100, step % 100);
            break;
        }

//        case MENU_TXP:
//            strncpy(String, gSubMenu_TXP[gSubMenuSelection], sizeof(gSubMenu_TXP[gSubMenuSelection]));
//            String[sizeof(gSubMenu_TXP[gSubMenuSelection])] = '\0';
//
//
//            break;

        case MENU_R_DCS:
        case MENU_T_DCS:
            if (gSubMenuSelection == 0)
                //translate
#ifdef test
                strcpy(String, "OFF");

#else
                strcpy(String, 关闭);

#endif


#if ENABLE_CHINESE_FULL == 0 || defined(ENABLE_ENGLISH)
            else if (gSubMenuSelection < 105)
                sprintf(String, "D%03oN", DCS_Options[gSubMenuSelection - 1]);
            else
                sprintf(String, "D%03oI", DCS_Options[gSubMenuSelection - 105]);
#else

            else if (gSubMenuSelection < 105)
                {
                  uint8_t read_tmp[2];
         EEPROM_ReadBuffer(0x02C64+(gSubMenuSelection - 1)*2, read_tmp, 2);
         uint16_t DCS_Options_read=read_tmp[0]|(read_tmp[1]<<8);
             sprintf(String, "D%03oN", DCS_Options_read);
             }
         else{
               uint8_t read_tmp[2];
         EEPROM_ReadBuffer(0x02C64+(gSubMenuSelection - 105)*2, read_tmp, 2);
         uint16_t DCS_Options_read=read_tmp[0]|(read_tmp[1]<<8);
             sprintf(String, "D%03oI",DCS_Options_read);
             }

#endif

            break;

        case MENU_R_CTCS:
        case MENU_T_CTCS: {
            if (gSubMenuSelection == 0)
                // translate
#ifdef test
                strcpy(String, "OFF");

#else
                //关闭
                strcpy(String, 关闭);

#endif

            else {
#if ENABLE_CHINESE_FULL == 0 || defined(ENABLE_ENGLISH)
                sprintf(String, "%u.%uHz", CTCSS_Options[gSubMenuSelection - 1] / 10,
                        CTCSS_Options[gSubMenuSelection - 1] % 10);
#else
                uint8_t read_tmp[2];
            EEPROM_ReadBuffer(0x02C00+(gSubMenuSelection - 1)*2, read_tmp, 2);
            uint16_t CTCSS_Options_read=read_tmp[0]|(read_tmp[1]<<8);
            sprintf(String, "%u.%uHz", CTCSS_Options_read / 10,CTCSS_Options_read % 10);

#endif
            }

            break;
        }

        case MENU_SFT_D:
            strncpy(String, gSubMenu_SFT_D[gSubMenuSelection], sizeof(gSubMenu_SFT_D[gSubMenuSelection]));
            String[sizeof(gSubMenu_SFT_D[gSubMenuSelection])] = '\0';
            break;

        case MENU_OFFSET:
            if (!gIsInSubMenu || gInputBoxIndex == 0) {
                sprintf(String, "%3d.%05u", gSubMenuSelection / 100000, abs(gSubMenuSelection) % 100000);
                UI_PrintStringSmall(String, menu_item_x1, menu_item_x2, 2);
            } else {
                const char *ascii = INPUTBOX_GetAscii();
                sprintf(String, "%.3s.%.3s  ", ascii, ascii + 3);
                UI_PrintStringSmall(String, menu_item_x1, menu_item_x2, 2);
            }

            UI_PrintStringSmall("MHz", menu_item_x1, menu_item_x2, 4);

            already_printed = true;
            break;
#ifdef ENABLE_CUSTOM_SIDEFUNCTIONS

            case MENU_W_N:

                strcpy(String, gSubMenu_W_N[gSubMenuSelection]);
                break;
#endif

        case MENU_SCR:
            strcpy(String, gSubMenu_SCRAMBLER[gSubMenuSelection]);

#if 1
            //  if (gSubMenuSelection > 0 && gSetting_ScrambleEnable)
            if (gSubMenuSelection > 0)
                BK4819_EnableScramble(gSubMenuSelection - 1);
            else
                BK4819_DisableScramble();
#endif
            break;


        case MENU_ABR:
            strcpy(String, gSubMenu_BACKLIGHT[gSubMenuSelection]);


//            BACKLIGHT_SetBrightness(-1);
            break;

            // case MENU_ABR_MIN:
        case MENU_ABR_MAX:
            sprintf(String, "%d", gSubMenuSelection);
            if (gIsInSubMenu)
                BACKLIGHT_SetBrightness(gSubMenuSelection);
//            else
//                BACKLIGHT_SetBrightness(-1);
            break;

//        case MENU_AM:
//            strcpy(String, gModulationStr[gSubMenuSelection]);
//
//            break;

#ifdef ENABLE_AM_FIX_TEST1
            case MENU_AM_FIX_TEST1:
                strcpy(String, gSubMenu_AM_fix_test1[gSubMenuSelection]);
//				gSetting_AM_fix = gSubMenuSelection;
                break;
#endif


        case MENU_COMPAND:
            strcpy(String, gSubMenu_RX_TX[gSubMenuSelection]);


            break;

#ifdef ENABLE_AM_FIX
            case MENU_AM_FIX:
#endif
        case MENU_BCL:
            //     case MENU_BEEP:
//        case MENU_S_ADD1:
//        case MENU_S_ADD2:
        case MENU_STE:
        case MENU_D_ST:
#ifdef ENABLE_DTMF_CALLING
#ifdef ENABLE_CUSTOM_SIDEFUNCTIONS

            case MENU_D_DCD:
#endif
#endif
        case MENU_D_LIVE_DEC:
#ifdef ENABLE_NOAA
            case MENU_NOAA_S:
#endif
//        case MENU_350TX:
//        case MENU_200TX:
//        case MENU_500TX:
//        case MENU_350EN:
            strcpy(String, gSubMenu_OFF_ON[gSubMenuSelection]);

            break;
//        case MENU_SCREN:
//            strcpy(String, gSubMenu_OFF_ON[gSubMenuSelection]);
//
//
//            break;

        case MENU_MEM_CH:
        case MENU_1_CALL:
        case MENU_DEL_CH: {
            const bool valid = RADIO_CheckValidChannel(gSubMenuSelection, false, 1);

            UI_GenerateChannelStringEx(String, valid, gSubMenuSelection);
            UI_PrintStringSmall(String, menu_item_x1 - 12, menu_item_x2, 2);

            if (valid && !gAskForConfirmation) {    // show the frequency so that the user knows the channels frequency
                const uint32_t frequency = SETTINGS_FetchChannelFrequency(gSubMenuSelection);
                sprintf(String, "%u.%05u", frequency / 100000, frequency % 100000);
                UI_PrintStringSmall(String, menu_item_x1 - 12, menu_item_x2, 5);
            }
            SETTINGS_FetchChannelName(String, gSubMenuSelection);
#if ENABLE_CHINESE_FULL == 4 && !defined(ENABLE_ENGLISH)
            show_move_flag=1;
#endif
            UI_PrintStringSmall(String[0] ? String : "--", menu_item_x1 - 12, menu_item_x2, 3);
            already_printed = true;
            break;
        }
#ifdef ENABLE_MDC1200
            case MENU_MDC_ID:
            {
#ifdef ENABLE_MDC1200_EDIT
                    if(gIsInSubMenu){    // show the channel name being edited
                    UI_PrintStringSmall(edit, menu_item_x1, menu_item_x2, 3);
                    if (edit_index < 4)
                        UI_PrintStringSmall("^", menu_item_x1+(((menu_item_x2 - menu_item_x1) - (28)) + 1) / 2 + (7 * edit_index), 0, 4);  // show the cursor
                }else
                    {
#endif
                    sprintf(String, "%04X", gEeprom.MDC1200_ID); // %04X确保输出是4个字符长度的十六进制数
                    UI_PrintStringSmall(String, menu_item_x1, menu_item_x2, 3);//4

#ifdef ENABLE_MDC1200_EDIT

                    edit_index = -1;
                    edit[0]=String[0];
                    edit[1]=String[1];
                    edit[2]=String[2];
                    edit[3]=String[3];
                      edit[4]='\0';
#endif
#ifdef ENABLE_MDC1200_EDIT
                    }
#endif
                already_printed = true;
                break;
            }
#endif
        case MENU_MEM_NAME: { //输入法显示
//ok


            const bool valid = RADIO_CheckValidChannel(gSubMenuSelection, false, 1);
            UI_GenerateChannelStringEx(String, valid, gSubMenuSelection);
            UI_PrintStringSmall(String, menu_item_x1 - 12, menu_item_x2, 2);

            if (valid) {
                const uint32_t frequency = SETTINGS_FetchChannelFrequency(gSubMenuSelection);
                //bug way
                char tmp_name[17] = {0};
                SETTINGS_FetchChannelName(tmp_name, gSubMenuSelection);

                if (!gIsInSubMenu || edit_index < 0) {    // show the channel name
                    SETTINGS_FetchChannelName(String, gSubMenuSelection);
                    char *pPrintStr = String[0] ? String : "--";
#if ENABLE_CHINESE_FULL == 4 && !defined(ENABLE_ENGLISH)
                    show_move_flag=1;
#endif
                    UI_PrintStringSmall(pPrintStr, menu_item_x1 - 12, menu_item_x2, 3);

                }

//
#if ENABLE_CHINESE_FULL == 4 && !defined(ENABLE_PINYIN) && !defined(ENABLE_ENGLISH)

                    else if (CHINESE_JUDGE(tmp_name, strlen(tmp_name))) {
                    edit_index = -1;
                }else if (!CHINESE_JUDGE(tmp_name, strlen(tmp_name))) {    // show the channel name being edited
#else
                else {
#endif


#if ENABLE_CHINESE_FULL == 4 && !defined(ENABLE_ENGLISH)
                    show_move_flag=1;
#endif
                    UI_PrintStringSmall(edit, menu_item_x1 - 12, menu_item_x2, 3);

                    if (edit_index < MAX_EDIT_INDEX) {
//#if ENABLE_CHINESE_FULL == 4
//                        show_move_flag=1;
//#endif

#ifdef ENABLE_PINYIN
                        for (int j = 0; j < MAX_EDIT_INDEX; ++j) {
                            if (edit[j] >= 0xb0 && j != MAX_EDIT_INDEX - 1) {
                                edit_chn[j] = 1;
                                j++;
                                edit_chn[j] = 2;
                            } else edit_chn[j] = 0;
                        }
                        uint8_t sum_pxl = 0;
                        uint8_t cnt_chn = 0;
                        for (int j = 0; j < edit_index; j++) {
                            if (edit_chn[j] == 1) {
                                sum_pxl += 13;
                                j++;
                                cnt_chn++;
                            } else
                                sum_pxl += 7;
                        }
                        uint8_t add_point = edit_chn[edit_index] == 1 ? 6 : 3;
                        gFrameBuffer[4][menu_item_x1 - 12 + sum_pxl +
                                        (((menu_item_x2 - menu_item_x1 + 12) -
                                          (7 * (MAX_EDIT_INDEX - 2 * cnt_chn) + 13 * cnt_chn)) + 1) / 2 + add_point] |=
                                3 << 6;
                        gFrameBuffer[4][menu_item_x1 - 12 + sum_pxl +
                                        (((menu_item_x2 - menu_item_x1 + 12) -
                                          (7 * (MAX_EDIT_INDEX - 2 * cnt_chn) + 13 * cnt_chn)) + 1) / 2 + add_point +
                                        1] |=
                                3 << 6;
#else

                        gFrameBuffer[4][menu_item_x1 - 12 + 7 * edit_index +
                                        (((menu_item_x2 - menu_item_x1 + 12) - (7 * MAX_EDIT_INDEX)) + 1) / 2 + 3] |=
                                3 << 6;
                        gFrameBuffer[4][menu_item_x1 - 12 + 7 * edit_index +
                                        (((menu_item_x2 - menu_item_x1 + 12) - (7 * MAX_EDIT_INDEX)) + 1) / 2 + 4] |=
                                3 << 6;

#endif

#ifdef ENABLE_PINYIN //拼音显示
                        //OK
                        if (INPUT_MODE == 0)memcpy(&gFrameBuffer[3][0], BITMAP_CN, 7);
                        else if (INPUT_MODE == 1) UI_PrintStringSmall("A", 0, 0, 3);
                        else if (INPUT_MODE == 2) UI_PrintStringSmall("1", 0, 0, 3);
                        else if (INPUT_MODE == 3) UI_PrintStringSmall("*", 0, 0, 3);
                        if (INPUT_MODE == 0) {
                            sprintf(String, "%06d", PINYIN_CODE);
                            GUI_DisplaySmallest(String, 0, 18, 0, 1);
                            uint8_t tmp[12];


                            if (INPUT_STAGE >= 1)//显示拼音
                            {
  uint8_t num=(PINYIN_NUM_SELECT ) / 3;
                    if ((PINYIN_NOW_NUM + 2) / 3 >1+num )memcpy(&gFrameBuffer[1][123], BITMAP_ARRAY_DOWN, 5);
                    if (num)memcpy(&gFrameBuffer[0][123], BITMAP_ARRAY_UP, 5);

                                if (PINYIN_SEARCH_MODE == 1)//准确的组合
                                {



//OK
                                    uint8_t HAVE_PINYIN =
                                            PINYIN_NOW_NUM - PINYIN_NUM_SELECT / 3 * 3 > 3 ? 3 : PINYIN_NOW_NUM -
                                                                                                 PINYIN_NUM_SELECT / 3 *
                                                                                                 3;//目前有多少个拼音

//OK
//                                    show_uint32(PINYIN_NOW_NUM,0);
//                                    sprintf(String,"%d",PINYIN_NUM_SELECT);
//                                    UI_PrintStringSmall(String, 0, 0, 4);
//                                    show_uint32(PINYIN_NOW_NUM,1);
//                                    show_uint32(HAVE_PINYIN,1);
                                    for (int j = 0; j < HAVE_PINYIN; ++j) {
                                        EEPROM_ReadBuffer(
                                                PINYIN_NOW_INDEX * 128 + 0X20000 + 16 + PINYIN_NUM_SELECT / 3 * 3 * 16 +
                                                j * 16, tmp, 6);
                                        memcpy(&String[6 * j], tmp, 6);//0 1 2 3 4 5
                                    }
//#include "ui/menu.h"
//#include "ui/helper.h"
//
//                                    if (PINYIN_CODE == 200000 && test_flag) {
//                                        show_uint32(edit_index, 0);
//                                        show_uint32(gIsInSubMenu, 1);
//                                        show_uint32(1, 2);
//                                        while (1);
//                                    }
                                    //NOT OK
                                    String[6 * HAVE_PINYIN] = 0;
                                    UI_PrintStringSmall(String, 0, 0, 0);
//NOT OK

                                }
                            }
                            if (INPUT_STAGE == 2) {

                                if (PINYIN_SEARCH_MODE == 1)//准确的组合
                                {
                                    memcpy(&gFrameBuffer[1][(PINYIN_NUM_SELECT % 3) * 7 * 6], BITMAP_ARRAY_UP, 5);

                                    uint8_t SHOW_NUM =
                                            CHN_NOW_NUM - CHN_NOW_PAGE * 6 > 6 ? 6 : CHN_NOW_NUM - CHN_NOW_PAGE * 6;
                                    EEPROM_ReadBuffer(CHN_NOW_ADD + CHN_NOW_PAGE * 6 * 2, tmp, SHOW_NUM * 2);
//                                    show_uint32(PINYIN_NOW_INDEX * 128 + 0X20000 + 16 + PINYIN_NUM_SELECT * 16 + 6, 5);
                                    for (int j = 0; j < SHOW_NUM; ++j) {
                                        String[j * 3] = '0' + j + 1;
                                        String[j * 3 + 1] = tmp[j * 2];
                                        String[j * 3 + 2] = tmp[j * 2 + 1];
                                    }
                                    String[SHOW_NUM * 3] = 0;
                                    show_move_flag = 1;

                                    UI_PrintStringSmall(String, 0, 0, 5);
                                    if (CHN_NOW_PAGE) memcpy(&gFrameBuffer[5][123], BITMAP_ARRAY_UP, 5);
                                    if ((CHN_NOW_PAGE + 1) * 6 < CHN_NOW_NUM)
                                        memcpy(&gFrameBuffer[6][123], BITMAP_ARRAY_DOWN, 5);
                                }
                            }
//NOT OK


                        } else if (INPUT_MODE == 1) {
                            if (INPUT_STAGE == 1) {
                                char tmp[22] = {0};
                                if (num_size[INPUT_SELECT - 2] == 3) {
                                    sprintf(tmp, "1.%c 2.%c 3.%c", num_excel[INPUT_SELECT - 2][0],
                                            num_excel[INPUT_SELECT - 2][1], num_excel[INPUT_SELECT - 2][2]);
                                    UI_PrintStringSmall(tmp, 0, 127, 0);
                                    tmp[0] = '4', tmp[4] = '5', tmp[8] = '6';//flash?
                                    tmp[2] -= 32, tmp[6] -= 32, tmp[10] -= 32;

                                } else {
                                    sprintf(tmp, "1.%c 2.%c 3.%c 4.%c", num_excel[INPUT_SELECT - 2][0],
                                            num_excel[INPUT_SELECT - 2][1], num_excel[INPUT_SELECT - 2][2],
                                            num_excel[INPUT_SELECT - 2][3]);
                                    UI_PrintStringSmall(tmp, 0, 127, 0);

                                    tmp[0] ='5', tmp[4] = '6', tmp[8] = '7', tmp[12] = '8';
                                    tmp[2] -= 32, tmp[6] -= 32, tmp[10] -= 32, tmp[14] -= 32;
                                }
                                                                    UI_PrintStringSmall(tmp, 0, 127, 1);

                            }
                        }
#endif

                    }
                }

//NOT OK
//                sprintf(String, "%d", edit_index);
//                UI_PrintStringSmall(String, 0, 0, 1);
//                sprintf(String, "%d", gIsInSubMenu);
//                UI_PrintStringSmall(String, 20, 0, 1);
//
//                sprintf(String,"%d",edit[2]);
//                UI_PrintStringSmall(String, 0, 0, 3);


                if (!gAskForConfirmation) {    // show the frequency so that the user knows the channels frequency
                    sprintf(String, "%u.%05u", frequency / 100000, frequency % 100000);
#ifdef ENABLE_PINYIN

                    if (edit_index == MAX_EDIT_INDEX - 1 && INPUT_MODE == 0)
                        INPUT_MODE = 1;
                    if (!(gIsInSubMenu && edit_index >= 0))

#endif
                    {
//                        show_move_flag = 1;
                        UI_PrintStringSmall(String, menu_item_x1 - 12, menu_item_x2, 5);
                    }
                }
            }

            already_printed = true;

            break;
        }

        case MENU_SAVE:
            strcpy(String, gSubMenu_SAVE[gSubMenuSelection]);


            break;

        case MENU_TDR:
            strcpy(String, gSubMenu_RXMode[gSubMenuSelection]);


            break;

        case MENU_TOT:
            strcpy(String, gSubMenu_TOT[gSubMenuSelection]);


            break;

#ifdef ENABLE_VOICE
            case MENU_VOICE:
                strcpy(String, gSubMenu_VOICE[gSubMenuSelection]);
                break;
#endif

        case MENU_SC_REV:


            strcpy(String, gSubMenu_SC_REV[gSubMenuSelection]);


            break;

        case MENU_MDF:


            strcpy(String, gSubMenu_MDF[gSubMenuSelection]);


            break;

        case MENU_RP_STE:
            if (gSubMenuSelection == 0)
//translate
#ifdef test
                strcpy(String, "OFF");

#else
                //关闭
                strcpy(String, 关闭);

#endif


            else
                sprintf(String, "%d*100ms", gSubMenuSelection);
            break;

        case MENU_S_LIST:
            if (gSubMenuSelection < 2)

                //translate

#ifdef test
                sprintf(String, "list %u", 1 + gSubMenuSelection);

#else  //！！列表
                sprintf(String, 列表" %u", 1 + gSubMenuSelection);

#endif

            else

#ifdef test
                strcpy(String, "ALL");

#else
                //全部
                strcpy(String, 全部);

#endif
            break;

#ifdef ENABLE_ALARM
            case MENU_AL_MOD:
                sprintf(String, gSubMenu_AL_MOD[gSubMenuSelection]);
                break;
#endif
#ifdef ENABLE_DTMF_CALLING
            case MENU_ANI_ID:

                strcpy(String, gEeprom.ANI_DTMF_ID);
                break;
#endif

        case MENU_UPCODE:

            sprintf(String, "%.8s\n%.8s", gEeprom.DTMF_UP_CODE, gEeprom.DTMF_UP_CODE + 8);
            break;

        case MENU_DWCODE:
            sprintf(String, "%.8s\n%.8s", gEeprom.DTMF_DOWN_CODE, gEeprom.DTMF_DOWN_CODE + 8);
            break;
#ifdef ENABLE_DTMF_CALLING
            case MENU_D_RSP:
                strcpy(String, gSubMenu_D_RSP[gSubMenuSelection]);


                break;

            case MENU_D_HOLD:
                sprintf(String, "%ds", gSubMenuSelection);
                break;
#endif
        case MENU_D_PRE:
            sprintf(String, "%d*10ms", gSubMenuSelection);
            break;

        case MENU_PTT_ID:
            strcpy(String, gSubMenu_PTT_ID[gSubMenuSelection]);


            break;

//        case MENU_BAT_TXT:
//            strcpy(String, gSubMenu_BAT_TXT[gSubMenuSelection]);
//
//
//            break;
#ifdef ENABLE_DTMF_CALLING
            case MENU_D_LIST:
                gIsDtmfContactValid = DTMF_GetContact((int) gSubMenuSelection - 1, Contact);
                if (!gIsDtmfContactValid)
                    strcpy(String, "NULL");
                else
                    memcpy(String, Contact, 8);
                break;
#endif
#if ENABLE_CHINESE_FULL == 4

            case MENU_PONMSG:
                strcpy(String, gSubMenu_PONMSG[gSubMenuSelection]);
                break;
#endif
        case MENU_ROGER:
            strcpy(String, gSubMenu_ROGER[gSubMenuSelection]);


            break;

//        case MENU_VOL:
//            sprintf(String, "%u.%02uV\n%u%%",
//                    gBatteryVoltageAverage / 100, gBatteryVoltageAverage % 100,
//                    BATTERY_VoltsToPercent(gBatteryVoltageAverage));
//            break;

        case MENU_RESET:
            strcpy(String, gSubMenu_RESET[gSubMenuSelection]);


            break;

        case MENU_F_LOCK:
//            if (!gIsInSubMenu && gUnlockAllTxConfCnt > 0&& gUnlockAllTxConfCnt < 10)
//                strcpy(String, "READ\nMANUAL");
//
//            else
            strcpy(String, gSubMenu_F_LOCK[gSubMenuSelection]);


            break;

#ifdef ENABLE_F_CAL_MENU
            case MENU_F_CALI:
                {
                    const uint32_t value   = 22656 + gSubMenuSelection;
                    const uint32_t xtal_Hz = (0x4f0000u + value) * 5;

                    writeXtalFreqCal(gSubMenuSelection, false);

                    sprintf(String, "%d\n%u.%06u\nMHz",
                        gSubMenuSelection,
                        xtal_Hz / 1000000, xtal_Hz % 1000000);
                }
                break;
#endif

        case MENU_BATCAL: {
            const uint16_t vol = (uint32_t) gBatteryVoltageAverage * gBatteryCalibration[3] / gSubMenuSelection;
            sprintf(String, "%u.%02uV\n%u", vol / 100, vol % 100, gSubMenuSelection);
            break;
        }

        case MENU_BATTYP:
            strcpy(String, gSubMenu_BATTYP[gSubMenuSelection]);

            break;

#ifdef ENABLE_CUSTOM_SIDEFUNCTIONS
            case MENU_F1SHRT:
            case MENU_F1LONG:
            case MENU_F2SHRT:
            case MENU_F2LONG:
            case MENU_MLONG:
                strcpy(String, gSubMenu_SIDEFUNCTIONS[gSubMenuSelection].name);
                break;
#endif

    }

    if (!already_printed) {    // we now do multi-line text in a single string

        unsigned int y;
        unsigned int lines = 1;
        unsigned int len = strlen(String);
        bool small = false;

        if (len > 0) {
            // count number of lines
            for (i = 0; i < len; i++) {
                if (String[i] == '\n' && i < (len - 1)) {    // found new line char
                    lines += 1;
                    String[i] = 0;  // null terminate the line
                }
            }

            if (lines > 3) {    // use small text
                small = true;
                if (lines > 7)
                    lines = 7;
            }

            // center vertically'ish
            if (small)
                y = 3 - ((lines + 0) / 2);  // untested
            else
                y = 2 - ((lines + 0) / 2);

            // draw the text lines
            for (i = 0; i < len && lines > 0; lines--) {
                if (small)
                    UI_PrintStringSmall(String + i, menu_item_x1, menu_item_x2, y + 1);
                else
                    UI_PrintStringSmall(String + i, menu_item_x1, menu_item_x2, y + 1);

                // look for start of next line
                while (i < len && String[i] != 0 && String[i] != '\n')
                    i++;

                // hop over the null term char(s)
                while (i < len && (String[i] == 0 || String[i] == '\n'))
                    i++;

                y += small ? 1 : 2;
            }
        }
    }

    if (UI_MENU_GetCurrentMenuId() == MENU_SLIST1 || UI_MENU_GetCurrentMenuId() == MENU_SLIST2) {
        i = (UI_MENU_GetCurrentMenuId() == MENU_SLIST1) ? 0 : 1;

        char *pPrintStr = String;

        if (gSubMenuSelection < 0) {
            pPrintStr = "NULL";
        } else {
            UI_GenerateChannelStringEx(String, true, gSubMenuSelection);
            pPrintStr = String;
        }

        // channel number
        UI_PrintStringSmall(pPrintStr, menu_item_x1 - 12, menu_item_x2, 2);

#if ENABLE_CHINESE_FULL == 4 && !defined(ENABLE_ENGLISH)
        show_move_flag=1;
#endif
        SETTINGS_FetchChannelName(String, gSubMenuSelection);
        pPrintStr = String[0] ? String : "--";


// channel name and scan-list
        if (gSubMenuSelection < 0 || !gEeprom.SCAN_LIST_ENABLED[i]) {
            UI_PrintStringSmall(pPrintStr, menu_item_x1 - 12, menu_item_x2, 4);
        } else {
            UI_PrintStringSmall(pPrintStr, menu_item_x1 - 12, menu_item_x2, 4);

//
//            if (IS_MR_CHANNEL(gEeprom.SCANLIST_PRIORITY_CH1[i])) {
//                sprintf(String, "PRI%d:%u", 1, gEeprom.SCANLIST_PRIORITY_CH1[i] + 1);
//                UI_PrintStringSmall(String, menu_item_x1 - 12, menu_item_x2, 3);
//            }
//
//            if (IS_MR_CHANNEL(gEeprom.SCANLIST_PRIORITY_CH2[i])) {
//                sprintf(String, "PRI%d:%u", 2, gEeprom.SCANLIST_PRIORITY_CH2[i] + 1);
//                UI_PrintStringSmall(String, menu_item_x1 - 12, menu_item_x2, 6);
//            }

        }
    }


    if ((UI_MENU_GetCurrentMenuId() == MENU_R_CTCS || UI_MENU_GetCurrentMenuId() == MENU_R_DCS) && gCssBackgroundScan)
        //扫描
        UI_PrintStringSmall(扫描, menu_item_x1, menu_item_x2, 5);

//
//    if (UI_MENU_GetCurrentMenuId() == MENU_UPCODE)
//        if (strlen(gEeprom.DTMF_UP_CODE) > 12)
//            UI_PrintStringSmall(gEeprom.DTMF_UP_CODE + 12, menu_item_x1, menu_item_x2, 5);
//
//    if (UI_MENU_GetCurrentMenuId() == MENU_DWCODE)
//        if (strlen(gEeprom.DTMF_DOWN_CODE) > 12)
//            UI_PrintStringSmall(gEeprom.DTMF_DOWN_CODE + 12, menu_item_x1, menu_item_x2, 5);
#ifdef ENABLE_DTMF_CALLING
    if (UI_MENU_GetCurrentMenuId() == MENU_D_LIST && gIsDtmfContactValid) {

        Contact[11] = 0;
        memcpy(&gDTMF_ID, Contact + 8, 4);
        sprintf(String, "ID:%4s", gDTMF_ID);
        UI_PrintStringSmall(String, menu_item_x1, menu_item_x2, 5);
    }
#endif
    if (UI_MENU_GetCurrentMenuId() == MENU_R_CTCS ||
        UI_MENU_GetCurrentMenuId() == MENU_T_CTCS ||
        UI_MENU_GetCurrentMenuId() == MENU_R_DCS ||
        UI_MENU_GetCurrentMenuId() == MENU_T_DCS
#ifdef ENABLE_DTMF_CALLING
        || UI_MENU_GetCurrentMenuId() == MENU_D_LIST
#endif
            ) {

        sprintf(String, "%2d", gSubMenuSelection);
        UI_PrintStringSmall(String, 105, 0, 1);//small
    }

    if ((UI_MENU_GetCurrentMenuId() == MENU_RESET ||
         UI_MENU_GetCurrentMenuId() == MENU_MEM_CH ||
         UI_MENU_GetCurrentMenuId() == MENU_MEM_NAME ||
         #ifdef ENABLE_MDC1200_EDIT

         UI_MENU_GetCurrentMenuId() == MENU_MDC_ID ||
         #endif
         UI_MENU_GetCurrentMenuId() == MENU_DEL_CH) && gAskForConfirmation) {    // display confirmation
        char *pPrintStr = (gAskForConfirmation == 1) ? "SURE?" : "WAIT!";
        if ((UI_MENU_GetCurrentMenuId() == MENU_MEM_CH || UI_MENU_GetCurrentMenuId() == MENU_MEM_NAME ||
             UI_MENU_GetCurrentMenuId() == MENU_DEL_CH) && gAskForConfirmation)
            UI_PrintStringSmall(pPrintStr, menu_item_x1 - 12, menu_item_x2, 5);
        else UI_PrintStringSmall(pPrintStr, menu_item_x1, menu_item_x2, 5);

        gRequestSaveSettings = 1;

    }

    ST7565_BlitFullScreen();
}

//

#ifndef ENABLE_ENGLISH
void UI_ShowChineseMenu() {


    uint8_t size_menu = 0;
    uint8_t cnt_menu = 0;


#if ENABLE_CHINESE_FULL == 4 && !defined(ENABLE_ENGLISH)
    uint8_t name[15];
    name[15] = 0;
    EEPROM_ReadBuffer(0x028B0 + gMenuCursor * 14, name, 14);
    for (cnt_menu = 0; cnt_menu < 7 && name[cnt_menu]!= 0; cnt_menu++) {
        if (is_chn(/*MenuList[gMenuCursor].name[cnt_menu]*/name[cnt_menu]) != 255)//中文
#else
    for (cnt_menu = 0; cnt_menu < 7 && MenuList[gMenuCursor].name[cnt_menu] != 0; cnt_menu++) {
        if (is_chn(MenuList[gMenuCursor].name[cnt_menu]) != 255)//中文
#endif
        {
            size_menu += 12;
#if ENABLE_CHINESE_FULL != 0
            cnt_menu++;
#endif
        } else//英文
        {
            size_menu += 7;
        }
    }

    show_move_flag = 1;

#if ENABLE_CHINESE_FULL == 4

    UI_PrintStringSmall((const char *)name, size_menu < 48 ? (48 - size_menu) / 2 : 0, 0, 0);
#else

    UI_PrintStringSmall(MenuList[gMenuCursor].name, size_menu < 48 ? (48 - size_menu) / 2 : 0, 0, 0);

#endif

}
#endif
#ifdef ENABLE_PINYIN
uint8_t INPUT_SELECT = 0;//选择的按键
uint8_t INPUT_MODE_LAST = 0;
uint8_t INPUT_MODE = 0;//0中文 1英文 2数字、符号
uint8_t INPUT_STAGE = 0;//中文：0 还没输入，不显示拼音和汉字 1输入了
uint32_t PINYIN_CODE = 0;
uint32_t PINYIN_CODE_INDEX = 100000;
uint8_t PINYIN_SEARCH_INDEX = 0;
uint8_t PINYIN_SEARCH_FOUND = 0;
uint8_t PINYIN_SEARCH_NUM = 0;
uint8_t PINYIN_NOW_INDEX = 0;//当前拼音组合地址
uint8_t PINYIN_NOW_NUM = 0;
uint8_t PINYIN_SEARCH_MODE = 0;
uint8_t PINYIN_START_INDEX = 0;
uint8_t PINYIN_END_INDEX = 0;
uint8_t PINYIN_NOW_PAGE = 0;
uint8_t PINYIN_NUM_SELECT = 0;
uint32_t CHN_NOW_ADD = 0;
uint8_t CHN_NOW_NUM = 0;
uint8_t CHN_NOW_PAGE = 0;
uint8_t edit_chn[MAX_EDIT_INDEX];
//英语：0 未选字 1选字
//数字：0正常模式 1按了上下的轮询模式，需要按MENU确定
char input1[22];
char input2[22];
char num_excel[8][4] = {
        {'a','b','c','\0'},
        {'d','e','f','\0'},
        {'g','h','i','\0'},
        {'j','k','l','\0'},
        {'m','n','o','\0'},
        {'p','q','r','s'},
        {'t','u','v','\0'},
        {'w','x','y','z'},

};
uint8_t num_size[8]={3,3,3,3,3,4,3,4};

uint32_t formatInt(uint32_t number) {//数字转拼音编码
    uint32_t formatted = number;
    uint32_t length = 0;
    // 计算整数的位数
    while (number != 0) {
        number /= 10;
        length++;
    }
    // 如果位数不足6位，则在后面补0
    if (length < 6) {
        for (uint8_t i = 0; i < 6 - length; ++i) {
            formatted *= 10;
        }
    }
    return formatted;
}


uint32_t get_num(const char *a) {//拼音转数字
    uint32_t num = 0;
    uint32_t bin = 100000;
    for (unsigned int j = 0; j < strlen(a); j++) {
        uint32_t now_num = 0;
        for (int i = 0; i < 8; ++i) {
            for (int k = 0; k < num_size[i]; ++k) {
                if (num_excel[i][k] == a[j]) {
                    now_num = i + 2;
                    goto end_loop;
                }
            }
        }
        end_loop:
        num += bin * now_num;
        bin /= 10;
    }
    return num;
}

bool judge_belong(uint32_t a, uint32_t b)//拼音归属判断
{
    for (uint32_t i = 100000; i >= 1; i /= 10) {
        if (a / i == 0)break;
        if (a / i != b / i)return false;
        a = a - a / i * i;
        b = b - b / i * i;

    }
    return true;
}

uint8_t sear_pinyin_code(uint32_t target, uint8_t *pinyin_num, uint8_t *found)//返回拼音索引0~213，以及是否找到
{
    int left = 0;
    int right = 213;
    *found = 0; // 初始设定未找到

    while (left <= right) {
        int mid = left + (right - left) / 2;
        uint8_t tmp[5];
        EEPROM_ReadBuffer(mid * 128 + 0x20000, tmp, 5);
        uint32_t mid_num = tmp[0] | tmp[1] << 8 | tmp[2] << 16 | tmp[3] << 24;
        *pinyin_num = tmp[4];
        if (mid_num == target) {
            *found = 1; // 找到了
            return mid;
        } else if (target < mid_num) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }

    // 找不到目标值，返回比目标值大一个的值
    if (left <= 213) {
        uint8_t tmp[5];
        EEPROM_ReadBuffer(left * 128 + 0x20000, tmp, 5);
        uint32_t left_num = tmp[0] | tmp[1] << 8 | tmp[2] << 16 | tmp[3] << 24;


        if (judge_belong(target, left_num)) {
            return left;
        }
    }
    return 255;

}

#endif