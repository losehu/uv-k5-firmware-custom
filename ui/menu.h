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

#ifndef UI_MENU_H
#define UI_MENU_H

#include <stdbool.h>
#include <stdint.h>

#include "audio.h"     // VOICE_ID_t
#include "settings.h"
#include "font.h"

typedef struct {
    VOICE_ID_t voice_id;
    uint8_t menu_id;
#if ENABLE_CHINESE_FULL == 0 || defined(ENABLE_ENGLISH)
    const char name[7]; // 使用指针而不是固定长度数组


#endif
} t_menu_item;

enum {
    MENU_SQL = 0,
    MENU_STEP,
    //MENU_TXP,
    MENU_R_DCS,
    MENU_R_CTCS,
    MENU_T_DCS,
    MENU_T_CTCS,
    MENU_SFT_D,
    MENU_OFFSET,
    MENU_TOT,
    MENU_W_N,
    MENU_SCR,
    MENU_BCL,
    MENU_MEM_CH,
    MENU_DEL_CH,
    MENU_MEM_NAME,
    MENU_MDF,
    MENU_SAVE,
#ifdef ENABLE_VOX
    //MENU_VOX,
#endif
    MENU_ABR,
    //MENU_ABR_ON_TX_RX,
    //MENU_ABR_MIN,
    MENU_ABR_MAX,
    MENU_TDR,
    //MENU_BEEP,
#ifdef ENABLE_VOICE
    MENU_VOICE,
#endif
    MENU_SC_REV,
    //MENU_AUTOLK,
    MENU_S_ADD1,
    MENU_S_ADD2,
    MENU_STE,
    MENU_RP_STE,
    MENU_MIC,
//#ifdef ENABLE_AUDIO_BAR
    //MENU_MIC_BAR,
//#endif
    MENU_COMPAND,
    MENU_1_CALL,
    MENU_S_LIST,
    MENU_SLIST1,
    MENU_SLIST2,
#ifdef ENABLE_ALARM
    MENU_AL_MOD,
#endif
#ifdef ENABLE_DTMF_CALLING
    MENU_ANI_ID,
#endif

    MENU_MDC_ID,


    MENU_UPCODE,
    MENU_DWCODE,
    MENU_PTT_ID,
    MENU_D_ST,
#ifdef ENABLE_DTMF_CALLING
    MENU_D_RSP,
    MENU_D_HOLD,
#endif

    MENU_D_PRE,
#ifdef ENABLE_DTMF_CALLING
#ifdef ENABLE_CUSTOM_SIDEFUNCTIONS

    MENU_D_DCD,
#endif
    MENU_D_LIST,
#endif

    MENU_D_LIVE_DEC,
#if ENABLE_CHINESE_FULL == 4

    MENU_PONMSG,
#endif

    MENU_ROGER,
    // MENU_VOL,
    //MENU_BAT_TXT,
    //MENU_AM,
#ifdef ENABLE_AM_FIX
    MENU_AM_FIX,
#endif
#ifdef ENABLE_AM_FIX_TEST1
    MENU_AM_FIX_TEST1,
#endif
#ifdef ENABLE_NOAA
    MENU_NOAA_S,
#endif
    MENU_RESET,
    MENU_F_LOCK,
//    MENU_200TX,
//    MENU_350TX,
//    MENU_500TX,
//    MENU_350EN,
    //  MENU_SCREN,
#ifdef ENABLE_F_CAL_MENU
    MENU_F_CALI,  // reference xtal calibration
#endif
    MENU_BATCAL,  // battery voltage calibration
#ifdef ENABLE_CUSTOM_SIDEFUNCTIONS
    MENU_F1SHRT,
    MENU_F1LONG,
    MENU_F2SHRT,
    MENU_F2LONG,
    MENU_MLONG,
#endif
    MENU_BATTYP
};

extern const t_menu_item MenuList[];
#if ENABLE_CHINESE_FULL == 4

extern const char gSubMenu_PONMSG[3][5];
#endif

//extern const char        gSubMenu_TXP[3][2];//5
#if ENABLE_CHINESE_FULL != 4 || defined(ENABLE_ENGLISH)

#ifdef ENABLE_ENGLISH
extern const char        gSubMenu_SFT_D[3][4];//3
#else
extern const char        gSubMenu_SFT_D[3][10];//3
#endif
#else

extern const char gSubMenu_SFT_D[3][16];//3
#endif
#ifdef ENABLE_CUSTOM_SIDEFUNCTIONS
#if ENABLE_CHINESE_FULL == 0 || defined(ENABLE_ENGLISH)
#ifdef ENABLE_ENGLISH
extern const char        gSubMenu_W_N[2][7];//7
#else
extern const char        gSubMenu_W_N[2][3];//7
#endif
#else
extern const char gSubMenu_W_N[2][5];//7
#endif
#endif
#if ENABLE_CHINESE_FULL != 4 || defined(ENABLE_ENGLISH)
#ifdef ENABLE_ENGLISH
extern const char        gSubMenu_OFF_ON[2][4];//4
#else
extern const char        gSubMenu_OFF_ON[2][3];//4
#endif
#else
extern const char gSubMenu_OFF_ON[2][5];//4
#endif
#if ENABLE_CHINESE_FULL != 4 || defined(ENABLE_ENGLISH)
extern const char        gSubMenu_SAVE[5][4];//4
#else
extern const char gSubMenu_SAVE[5][6];//4
#endif
#if ENABLE_CHINESE_FULL != 4 || defined(ENABLE_ENGLISH)
extern const char        gSubMenu_TOT[11][7];//7
#else
extern const char gSubMenu_TOT[11][6];//7
#endif
extern const char *const gSubMenu_RXMode[4];

#ifdef ENABLE_VOICE
extern const char    gSubMenu_VOICE[3][4];
#endif
#if ENABLE_CHINESE_FULL != 4 || defined(ENABLE_ENGLISH)


#ifdef ENABLE_ENGLISH
extern const char        gSubMenu_SC_REV[3][8];//8
#else
extern const char        gSubMenu_SC_REV[3][10];//8
#endif


#else
extern const char gSubMenu_SC_REV[3][18];//8
#endif
extern const char *const gSubMenu_MDF[4];
#ifdef ENABLE_ALARM
extern const char    gSubMenu_AL_MOD[2][5];
#endif
#ifdef ENABLE_DTMF_CALLING
#if ENABLE_CHINESE_FULL != 4 || defined(ENABLE_ENGLISH)

#ifdef ENABLE_ENGLISH
extern const char        gSubMenu_D_RSP[4][11];//11
#else
extern const char        gSubMenu_D_RSP[4][10];//11
#endif
#else
extern const char gSubMenu_D_RSP[4][18];//11
#endif
#endif

extern const char *const gSubMenu_PTT_ID[5];
#if ENABLE_CHINESE_FULL != 4 || defined(ENABLE_ENGLISH)

#ifdef ENABLE_ENGLISH
extern const char        gSubMenu_ROGER[6][15];
#else
extern const char        gSubMenu_ROGER[6][13];
#endif

#else
extern const char gSubMenu_ROGER[6][15];
#endif
#if ENABLE_CHINESE_FULL != 4 || defined(ENABLE_ENGLISH)
#ifdef ENABLE_ENGLISH
extern const char        gSubMenu_RESET[2][4];//4
#else
extern const char        gSubMenu_RESET[2][6];//4
#endif

#else
extern const char gSubMenu_RESET[2][11];//4
#endif
extern const char *const gSubMenu_F_LOCK[F_LOCK_LEN];
#if ENABLE_CHINESE_FULL != 4 || defined(ENABLE_ENGLISH)


#ifdef ENABLE_ENGLISH
extern const char        gSubMenu_BACKLIGHT[8][7];//7
#else
extern const char        gSubMenu_BACKLIGHT[8][5];//7
#endif
#else
extern const char gSubMenu_BACKLIGHT[8][6];//7
#endif
#if ENABLE_CHINESE_FULL != 4 || defined(ENABLE_ENGLISH)

#ifdef ENABLE_ENGLISH
extern const char        gSubMenu_RX_TX[4][6];//6

#else
extern const char        gSubMenu_RX_TX[4][7];//6
#endif

#else
extern const char gSubMenu_RX_TX[4][12];//6
#endif
#ifdef ENABLE_AM_FIX_TEST1
extern const char    gSubMenu_AM_fix_test1[4][8];
#endif
//extern const char        gSubMenu_BAT_TXT[3][3];//8
extern const char gSubMenu_BATTYP[2][8];
extern const char gSubMenu_SCRAMBLER[11][7];

typedef struct {
    char *name;
    uint8_t id;
} t_sidefunction;
extern const uint8_t gSubMenu_SIDEFUNCTIONS_size;
extern const t_sidefunction *gSubMenu_SIDEFUNCTIONS;
//extern const t_sidefunction SIDEFUNCTIONS[];

extern bool gIsInSubMenu;

extern uint8_t gMenuCursor;

extern int32_t gSubMenuSelection;

extern char edit_original[17];
extern char edit[17];
extern int edit_index;
extern uint8_t num_size[8];

void UI_DisplayMenu(void);

uint8_t pinyin_search(uint8_t *target, uint8_t size, uint32_t *add);

uint8_t pinyin_cmp(uint8_t *a, uint8_t *b);

int UI_MENU_GetCurrentMenuId();

uint8_t UI_MENU_GetMenuIdx(uint8_t id);

void UI_ShowChineseMenu(void);

#ifdef ENABLE_PINYIN
extern char num_excel[8][4] ;
bool judge_belong(uint32_t a,uint32_t b);//拼音归属判断

uint8_t sear_pinyin_code(uint32_t target,uint8_t *pinyin_num,uint8_t *found);//返回拼音索引0~213，以及是否找到



extern uint8_t INPUT_MODE;//0中文 1英文 2数字、符号
extern uint8_t INPUT_STAGE;//中文：0 还没输入，不显示拼音和汉字 1输入了
//英语：0 未选字 1选字
//数字：0正常模式 1按了上下的轮询模式，需要按MENU确定

extern uint8_t INPUT_SELECT;//选择的按键
extern uint8_t INPUT_MODE_LAST;
extern uint32_t PINYIN_CODE;
extern uint32_t PINYIN_CODE_INDEX;
extern uint8_t PINYIN_SEARCH_INDEX;
extern uint8_t PINYIN_SEARCH_FOUND;
extern uint8_t PINYIN_SEARCH_NUM;
extern uint8_t PINYIN_NOW_INDEX;//当前拼音组合地址
extern uint8_t PINYIN_NOW_NUM;//当前拼音组合地址
extern uint8_t PINYIN_SEARCH_MODE;
extern uint8_t PINYIN_START_INDEX;
extern uint8_t PINYIN_NUM_SELECT;
extern uint32_t CHN_NOW_ADD;
extern uint8_t CHN_NOW_NUM;
extern uint8_t CHN_NOW_PAGE;
extern uint8_t edit_chn[MAX_EDIT_INDEX];

#endif
#endif
