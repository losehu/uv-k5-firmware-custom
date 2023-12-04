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

const t_menu_item MenuList[] =
        {
//   text,     voice ID,                               menu ID
                {/*"Step",*/   VOICE_ID_FREQUENCY_STEP,                MENU_STEP          ,步进频率},
                {/*"RxDCS",*/  VOICE_ID_DCS,                           MENU_R_DCS         ,接收数字亚音}, // was "R_DCS"
                {/*"RxCTCS",*/ VOICE_ID_CTCSS,                         MENU_R_CTCS        ,接收模拟亚音}, // was "R_CTCS"
                {/*"TxDCS",*/  VOICE_ID_DCS,                           MENU_T_DCS         ,发送数字亚音}, // was "T_DCS"
                {/*"TxCTCS",*/ VOICE_ID_CTCSS,                         MENU_T_CTCS        ,发送模拟亚音}, // was "T_CTCS"
                {/*"TxODir",*/ VOICE_ID_TX_OFFSET_FREQUENCY_DIRECTION, MENU_SFT_D         ,频差方向}, // was "SFT_D"
                {/*"TxOffs",*/ VOICE_ID_TX_OFFSET_FREQUENCY,           MENU_OFFSET        ,频差频率}, // was "OFFSET"
                {/*"Scramb",*/ VOICE_ID_SCRAMBLER_ON,                  MENU_SCR           ,加密通话}, // was "SCR"
                {/*"BusyCL",*/ VOICE_ID_BUSY_LOCKOUT,                  MENU_BCL           ,遇忙禁发}, // was "BCL"
                {/*"Compnd",*/ VOICE_ID_INVALID,                       MENU_COMPAND       ,压扩},
                {/*"ChSave",*/ VOICE_ID_MEMORY_CHANNEL,                MENU_MEM_CH        ,存置信道}, // was "MEM-CH"
                {/*"ChDele",*/ VOICE_ID_DELETE_CHANNEL,                MENU_DEL_CH        ,删除信道}, // was "DEL-CH"
                {/*"ChName",*/ VOICE_ID_INVALID,                       MENU_MEM_NAME      ,命名信道},
                {/*"SList",*/  VOICE_ID_INVALID,                       MENU_S_LIST        ,信道扫描列表},
                {/*"SList1",*/ VOICE_ID_INVALID,                       MENU_SLIST1        ,扫描列表1},
                {/*"SList2",*/ VOICE_ID_INVALID,                       MENU_SLIST2        ,扫描列表2},
                {/*"ScnRev",*/ VOICE_ID_INVALID,                       MENU_SC_REV        ,搜索恢复模式},
                {/*"TxTOut",*/ VOICE_ID_TRANSMIT_OVER_TIME,            MENU_TOT           ,发送超时}, // was "TOT"
                {/*"BatSav",*/ VOICE_ID_SAVE_MODE,                     MENU_SAVE          ,省电模式}, // was "SAVE"
                {/*"Mic",*/    VOICE_ID_INVALID,                       MENU_MIC           ,麦克风增益},
                {/*"ChDisp",*/ VOICE_ID_INVALID,                       MENU_MDF           ,信道显示模式}, // was "MDF"
             //   {/*"POnMsg",*/ VOICE_ID_INVALID,                       MENU_PONMSG        ,开机显示},
                {/*"BackLt",*/ VOICE_ID_INVALID,                       MENU_ABR           ,自动背光}, // was "ABR"
                {/*"BLMax",*/  VOICE_ID_INVALID,                       MENU_ABR_MAX       ,背光亮度},
                {/*"Roger",*/  VOICE_ID_INVALID,                       MENU_ROGER         ,发送结束音},
                {/*"STE",*/    VOICE_ID_INVALID,                       MENU_STE           ,尾音消除},
                {/*"RP STE",*/ VOICE_ID_INVALID,                       MENU_RP_STE        ,过中继尾音消除},
                {/*"1 Call",*/ VOICE_ID_INVALID,                       MENU_1_CALL        ,按键即呼},

#ifdef ENABLE_DTMF_CALLING
                {/*"ANI ID",*/ VOICE_ID_ANI_CODE,                      MENU_ANI_ID        ,身份码},
#endif
                {/*"UPCode",*/ VOICE_ID_INVALID,                       MENU_UPCODE        ,DTMF上线码},
                {/*"DWCode",*/ VOICE_ID_INVALID,                       MENU_DWCODE        ,DTMF下线码},
                {/*"PTT ID",*/ VOICE_ID_INVALID,                       MENU_PTT_ID        ,DTMF发送},
                {/*"D ST",*/   VOICE_ID_INVALID,                       MENU_D_ST          ,DTMF侧音},
#ifdef ENABLE_DTMF_CALLING
                {/*"D Resp",*/ VOICE_ID_INVALID,                       MENU_D_RSP         ,DTMF响应},
                {/*"D Hold",*/ VOICE_ID_INVALID,                       MENU_D_HOLD        ,DTMF复位},
#endif
                {/*"D Prel",*/ VOICE_ID_INVALID,                       MENU_D_PRE         ,DTMF预载波},
#ifdef ENABLE_DTMF_CALLING
                {/*"D List",*/ VOICE_ID_INVALID,                       MENU_D_LIST        ,DTMF联系人},
#endif
                {/*"D Live",*/ VOICE_ID_INVALID,                       MENU_D_LIVE_DEC    ,DTMF显示}, // live DTMF decoder
#ifdef ENABLE_AM_FIX//1
                {/*"AM Fix",*/ VOICE_ID_INVALID,                       MENU_AM_FIX        ,AM自动增益},
#endif
#ifdef ENABLE_AM_FIX_TEST1//0
                {/*"AM FT1",*/ VOICE_ID_INVALID,                       MENU_AM_FIX_TEST1  ,""},
#endif

                {/*"RxMode",*/ VOICE_ID_DUAL_STANDBY,                  MENU_TDR           ,收发模式},
                {/*"Sql",*/    VOICE_ID_SQUELCH,                       MENU_SQL           ,静噪等级},

                // hidden menu items from here on
                // enabled if pressing both the PTT and upper side button at power-on
                {/*"F Lock",*/ VOICE_ID_INVALID,                       MENU_F_LOCK        ,频段解锁},
                {/*"Tx 200",*/ VOICE_ID_INVALID,                       MENU_200TX         ,两百M发射}, // was "200TX"
                {/*"Tx 350",*/ VOICE_ID_INVALID,                       MENU_350TX         ,三百五十M发射}, // was "350TX"
                {/*"Tx 500",*/ VOICE_ID_INVALID,                       MENU_500TX         ,五百M发射}, // was "500TX"
                {/*"350 En",*/ VOICE_ID_INVALID,                       MENU_350EN         ,三百五十M接收}, // was "350EN"
#ifdef ENABLE_F_CAL_MENU//0
                {/*"FrCali",*/ VOICE_ID_INVALID,                       MENU_F_CALI        ,""}, // reference xtal calibration
#endif
                {/*"BatCal",*/ VOICE_ID_INVALID,                       MENU_BATCAL        ,电池调压}, // battery voltage calibration
               {/*"BatTyp",*/ VOICE_ID_INVALID,                       MENU_BATTYP        ,电池大小}, // battery type 1600/2200mAh
                {/*"Reset",*/  VOICE_ID_INITIALISATION,                MENU_RESET         ,参数复位}, // might be better to move this to the hidden menu items ?

            //   {/*"",*/       VOICE_ID_INVALID,                       0xff               ,"\x00"}  // end of list - DO NOT delete or move this this
        };


const uint8_t FIRST_HIDDEN_MENU_ITEM = MENU_F_LOCK;
const char gSubMenu_SFT_D[][10] =//4
        {
//                "OFF",
//                "+",
//                "-"
                发送等于接收,
                发送等于接收加偏移,
                发送等于接收减偏移

        };



const char gSubMenu_OFF_ON[][3] =//4
        {
//                "OFF",
//                "ON"
                关闭,
                开启
        };

const char gSubMenu_SAVE[][4] =//4
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

const char gSubMenu_TOT[][5] = //7
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

 三十秒 ,
 一分 ,
 两分,
 三分 ,
 四分 ,
 五分 ,
 六分 ,
 七分 ,
 八分 ,
 九分 ,
 十五分

        };

const char *gSubMenu_RXMode[] =
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

const char gSubMenu_SC_REV[][10] =//8
        {
//                "TIMEOUT",
//                "CARRIER",
//                "STOP"
                遇信号5秒后搜索  ,
                信号停止后搜索  ,
                遇信号后停止搜索

        };

const char *gSubMenu_MDF[] =
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
const char gSubMenu_D_RSP[][10] =//11
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

const char *gSubMenu_PTT_ID[] =
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



const char gSubMenu_ROGER[][9] =
        {
//                "OFF",
//                "ROGER",
//                "MDC"

                关闭结束音,
                ROGER结束音,
                MDC蛙叫
        };

const char gSubMenu_RESET[][6] =//4
        {
//                "VFO",
//                "ALL"
                除信道参数,
                全部参数
        };

const char *gSubMenu_F_LOCK[] =
        {
                "DEFAULT+\n137-174\n400-470",
                "FCC HAM\n144-148\n420-450",
                "CE HAM\n144-146\n430-440",
                "GB HAM\n144-148\n430-440",
                "137-174\n400-430",
                "137-174\n400-438",
//                "DISABLE\nALL",
//                "UNLOCK\nALL",
                禁用全部,
                解锁全部,
        };

const char gSubMenu_BACKLIGHT[][5] =//7
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

const char gSubMenu_RX_TX[][7] =//6
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
    const unsigned int menu_item_x1 = (8 * menu_list_width) ;//+ 2;
    const unsigned int menu_item_x2 = LCD_WIDTH - 1;
    unsigned int i;
    char String[128];  // bigger cuz we can now do multi-line in one string (use '\n' char)
#ifdef ENABLE_DTMF_CALLING
    char               Contact[16];
#endif

    // clear the screen buffer
    memset(gFrameBuffer, 0, sizeof(gFrameBuffer));

#if 1
    // original menu layout



    // invert the current menu list item pixels反转当前菜单项的像素值 ：


    // draw vertical separating dotted line绘制垂直分隔的点线 ：
//    for (i = 0; i < 7; i++)
//        gFrameBuffer[i][(8 * menu_list_width) + 1] = 0xAA;


    // draw the little sub-menu triangle marker绘制子菜单三角标志：
    //const void *BITMAP_CurrentIndicator = BITMAP_MARKER;

    if (gIsInSubMenu)
        memmove(gFrameBuffer[2] + 40, BITMAP_MARKER, sizeof(BITMAP_MARKER));

    // draw the menu index number/count绘制菜单索引号/总数 ：
    sprintf(String, "%2u/%u", 1 + gMenuCursor, gMenuListCount);
    UI_PrintStringSmall(String, 2, 0, 6);
//    UI_ShowChineseMenu();
    UI_ShowChineseMenu();

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
            strcpy(String, MenuList[menu_index].name);
//			strcat(String, ":");
            UI_PrintStringSmall(String, 0, 0, 0);
//			UI_PrintStringSmall(String, 0, 0, 0);
        }
    }
#endif

    // **************

    memset(String, 0, sizeof(String));

    bool already_printed = false;

    switch (UI_MENU_GetCurrentMenuId()) {
        case MENU_SQL:
            sprintf(String, "%d", gSubMenuSelection);
            break;

        case MENU_MIC: {    // display the mic gain in actual dB rather than just an index number
            const uint8_t mic = gMicGain_dB2[gSubMenuSelection];
            sprintf(String, "+%u.%01udB", mic / 2, mic % 2);
        }
            break;

#ifdef ENABLE_AUDIO_BAR
//            case MENU_MIC_BAR:
//                strcpy(String, gSubMenu_OFF_ON[gSubMenuSelection]);
//                break;
#endif

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

            else if (gSubMenuSelection < 105)
                sprintf(String, "D%03oN", DCS_Options[gSubMenuSelection - 1]);
            else
                sprintf(String, "D%03oI", DCS_Options[gSubMenuSelection - 105]);
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

            else
                sprintf(String, "%u.%uHz", CTCSS_Options[gSubMenuSelection - 1] / 10,
                        CTCSS_Options[gSubMenuSelection - 1] % 10);
            break;
        }

        case MENU_SFT_D:
            strncpy(String, gSubMenu_SFT_D[gSubMenuSelection] , sizeof( gSubMenu_SFT_D[gSubMenuSelection] ));
            String[sizeof( gSubMenu_SFT_D[gSubMenuSelection])] = '\0';
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

//        case MENU_W_N:
//
//            strcpy(String, gSubMenu_W_N[gSubMenuSelection]);
//            break;
   
        case MENU_SCR:
            strcpy(String, gSubMenu_SCRAMBLER[gSubMenuSelection]);
       
#if 1
          //  if (gSubMenuSelection > 0 && gSetting_ScrambleEnable)
            if (gSubMenuSelection > 0 )
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
       // case MENU_D_DCD:
#endif
        case MENU_D_LIVE_DEC:
#ifdef ENABLE_NOAA
            case MENU_NOAA_S:
#endif
        case MENU_350TX:
        case MENU_200TX:
        case MENU_500TX:
        case MENU_350EN:
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
            UI_PrintStringSmall(String, menu_item_x1, menu_item_x2, 2);

            if (valid && !gAskForConfirmation) {    // show the frequency so that the user knows the channels frequency
                const uint32_t frequency = SETTINGS_FetchChannelFrequency(gSubMenuSelection);
                sprintf(String, "%u.%05u", frequency / 100000, frequency % 100000);
                UI_PrintStringSmall(String, menu_item_x1, menu_item_x2, 5);
            }

            already_printed = true;
            break;
        }

        case MENU_MEM_NAME: {
            const bool valid = RADIO_CheckValidChannel(gSubMenuSelection, false, 1);

            UI_GenerateChannelStringEx(String, valid, gSubMenuSelection);
            UI_PrintStringSmall(String, menu_item_x1, menu_item_x2, 2);

            if (valid) {
                const uint32_t frequency = SETTINGS_FetchChannelFrequency(gSubMenuSelection);
                //bug way
                if (!gIsInSubMenu || edit_index < 0) {    // show the channel name
                    SETTINGS_FetchChannelName(String, gSubMenuSelection);
                    if (String[0] == 0)
                        strcpy(String, "--");
                    
                    UI_PrintStringSmall(String, menu_item_x1, menu_item_x2, 3);//4
                } else {    // show the channel name being edited
                    UI_PrintStringSmall(edit, menu_item_x1, 0, 3);
                    if (edit_index < 10)
                        UI_PrintStringSmall("^", menu_item_x1 + (8 * edit_index), 0, 4);  // show the cursor
                }

                if (!gAskForConfirmation) {    // show the frequency so that the user knows the channels frequency
                    sprintf(String, "%u.%05u", frequency / 100000, frequency % 100000);
                    if (!gIsInSubMenu || edit_index < 0)
                        UI_PrintStringSmall(String, menu_item_x1, menu_item_x2, 5);
                    else
                        UI_PrintStringSmall(String, menu_item_x1, menu_item_x2, 5);
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
                sprintf(String, "\x85\x86 %u", 1 + gSubMenuSelection);

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
            strcpy(String, gEeprom.DTMF_UP_CODE);
            break;

        case MENU_DWCODE:
            strcpy(String, gEeprom.DTMF_DOWN_CODE);
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
                memmove(String, Contact, 8);
            break;
#endif
//        case MENU_PONMSG:
//            strcpy(String, gSubMenu_OFF_ON[gSubMenuSelection]);
//
//
//            break;

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

//        case MENU_F1SHRT:
//        case MENU_F1LONG:
//        case MENU_F2SHRT:
//        case MENU_F2LONG:
//        case MENU_MLONG:
//            strcpy(String, gSubMenu_SIDEFUNCTIONS[gSubMenuSelection].name);
//            break;

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
                    lines+=1;
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
                while (i < len && String[i] !=0&&String[i] !='\n')
                    i++;

                // hop over the null term char(s)
                while (i < len && (String[i] ==0||String[i] =='\n'))
                    i++;

                y += small ? 1 : 2;
            }
        }
    }

    if (UI_MENU_GetCurrentMenuId() == MENU_SLIST1 || UI_MENU_GetCurrentMenuId() == MENU_SLIST2) {
        i = (UI_MENU_GetCurrentMenuId() == MENU_SLIST1) ? 0 : 1;

//		if (gSubMenuSelection == 0xFF)
        if (gSubMenuSelection < 0)
            strcpy(String, "NULL");
        else
            UI_GenerateChannelStringEx(String, true, gSubMenuSelection);

//		if (gSubMenuSelection == 0xFF || !gEeprom.SCAN_LIST_ENABLED[i])
        if (gSubMenuSelection < 0 || !gEeprom.SCAN_LIST_ENABLED[i]) {
            // channel number
            UI_PrintStringSmall(String, menu_item_x1, menu_item_x2, 2);

            // channel name
            SETTINGS_FetchChannelName(String, gSubMenuSelection);
            if (String[0] == 0)
                strcpy(String, "--");
            UI_PrintStringSmall(String, menu_item_x1, menu_item_x2, 4);
        } else {
            // channel number
            UI_PrintStringSmall(String, menu_item_x1, menu_item_x2, 2);

            // channel name
            SETTINGS_FetchChannelName(String, gSubMenuSelection);
            if (String[0] == 0)
                strcpy(String, "--");
            UI_PrintStringSmall(String, menu_item_x1, menu_item_x2, 4);

            if (IS_MR_CHANNEL(gEeprom.SCANLIST_PRIORITY_CH1[i])) {
                sprintf(String, "PRI1:%u", gEeprom.SCANLIST_PRIORITY_CH1[i] + 1);
                UI_PrintStringSmall(String, menu_item_x1, menu_item_x2, 4);
            }

            if (IS_MR_CHANNEL(gEeprom.SCANLIST_PRIORITY_CH2[i])) {
                sprintf(String, "PRI2:%u", gEeprom.SCANLIST_PRIORITY_CH2[i] + 1);
                UI_PrintStringSmall(String, menu_item_x1, menu_item_x2, 5);
            }
        }
    }

    if (UI_MENU_GetCurrentMenuId() == MENU_MEM_CH ||
        UI_MENU_GetCurrentMenuId() == MENU_DEL_CH ||
        UI_MENU_GetCurrentMenuId() == MENU_1_CALL) {    // display the channel name
        char s[11];
        SETTINGS_FetchChannelName(s, gSubMenuSelection);
        if (s[0] == 0)
            strcpy(s, "--");
        UI_PrintStringSmall(s, menu_item_x1, menu_item_x2, 3);
    }

    if ((UI_MENU_GetCurrentMenuId() == MENU_R_CTCS || UI_MENU_GetCurrentMenuId() == MENU_R_DCS) && gCssBackgroundScan)
        //扫描
        UI_PrintStringSmall(扫描, menu_item_x1, menu_item_x2, 5);


    if (UI_MENU_GetCurrentMenuId() == MENU_UPCODE)
        if (strlen(gEeprom.DTMF_UP_CODE) > 8)
            UI_PrintStringSmall(gEeprom.DTMF_UP_CODE + 8, menu_item_x1, menu_item_x2, 5);

    if (UI_MENU_GetCurrentMenuId() == MENU_DWCODE)
        if (strlen(gEeprom.DTMF_DOWN_CODE) > 8)
            UI_PrintStringSmall(gEeprom.DTMF_DOWN_CODE + 8, menu_item_x1, menu_item_x2, 5);
#ifdef ENABLE_DTMF_CALLING
    if (UI_MENU_GetCurrentMenuId() == MENU_D_LIST && gIsDtmfContactValid) {
        Contact[11] = 0;
        memmove(&gDTMF_ID, Contact + 8, 4);
        sprintf(String, "ID:%s", Contact + 8);
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
            )

    {

        sprintf(String, "%2d", gSubMenuSelection);
        UI_PrintStringSmall(String, 105, 0, 1);//small
    }

    if ((UI_MENU_GetCurrentMenuId() == MENU_RESET ||
         UI_MENU_GetCurrentMenuId() == MENU_MEM_CH ||
         UI_MENU_GetCurrentMenuId() == MENU_MEM_NAME ||
         UI_MENU_GetCurrentMenuId() == MENU_DEL_CH) && gAskForConfirmation) {    // display confirmation
        strcpy(String, (gAskForConfirmation == 1) ? "SURE?" : "WAIT!");
        UI_PrintStringSmall(String, menu_item_x1, menu_item_x2, 5);
    }
//    for (int i = 0; i < 128; i++) {
//        // Set the 7th and 8th positions to 1, keep others unchanged
//        gFrameBuffer[1][i] |= (1 << 7)|(1<<6) ;
//
//    }

    ST7565_BlitFullScreen();
}

//


void UI_ShowChineseMenu() {
  //  return;
    uint8_t cnt_char = 0;

    uint8_t size_menu = 0;
    uint8_t cnt_menu=0;
    for ( cnt_menu = 0; cnt_menu < 7 && MenuList[gMenuCursor].name[cnt_menu] != 0; cnt_menu++) {

            if(is_chn(MenuList[gMenuCursor].name[cnt_menu])!=255)//中文
                size_menu+=12;
            else//英文
                size_menu+=7;

    }
    cnt_char = 0;
    if (size_menu < 48)cnt_char = (48 - size_menu ) / 2;
    menu_set_flag=1;
    UI_PrintStringSmall(MenuList[gMenuCursor].name, (cnt_char), 0, 0);
//
//    for (uint8_t i = 0; i < cnt_menu; i++) {
//        uint8_t num_solve=is_chn(MenuList[gMenuCursor].name[i]);
//        if(num_solve==255)//数字/字母
//        {
//            char tmp[2]={0};
//            tmp[0]=MenuList[gMenuCursor].name[i];
//            UI_PrintStringSmall((const char *)tmp, (cnt_char), 0, 0);
//            cnt_char += 7;
//        } else  {
//
//
//            UI_PrintChineseChar(num_solve, cnt_char, 0);
//
//            cnt_char += 13;
//        }
//
//    }

}