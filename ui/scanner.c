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
#include "chinese.h"
#include <stdbool.h>
#include <string.h>
#include "app/scanner.h"
#include "dcs.h"
#include "driver/eeprom.h"
#include "driver/st7565.h"
#include "external/printf/printf.h"
#include "misc.h"
#include "ui/helper.h"
#include "ui/scanner.h"

void UI_DisplayScanner(void) {
    char String[16] = {0};
    char *pPrintStr = String;
    bool bCentered;
    uint8_t Start;

    UI_DisplayClear();
    if (gScanSingleFrequency || (gScanCssState != SCAN_CSS_STATE_OFF && gScanCssState != SCAN_CSS_STATE_FAILED)) {
//频率
        sprintf(String, 频率":%u.%05u", gScanFrequency / 100000, gScanFrequency % 100000);

        pPrintStr = String;
    } else {
        pPrintStr = 频率":**.*****";
    }


    UI_PrintStringSmall(pPrintStr, 2, 0, 1);

    if (gScanCssState < SCAN_CSS_STATE_FOUND || !gScanUseCssResult) {
        pPrintStr = 模拟亚音":******";
    } else if (gScanCssResultType == CODE_TYPE_CONTINUOUS_TONE) {
        //模拟亚音
#ifdef TEST_UNDE_CTCSS

        sprintf(String, 模拟亚音":%u.%uHz", gScanCssResultCode_all/10, gScanCssResultCode_all% 10);
#else
#if ENABLE_CHINESE_FULL == 0 || defined(ENABLE_ENGLISH)
        sprintf(String, 模拟亚音":%u.%uHz", CTCSS_Options[gScanCssResultCode] / 10,
                CTCSS_Options[gScanCssResultCode] % 10);

#else
        uint8_t read_tmp[2];
    EEPROM_ReadBuffer(0x02C00+gScanCssResultCode*2, read_tmp, 2);
    uint16_t CTCSS_Options_read=read_tmp[0]|(read_tmp[1]<<8);
          sprintf(String, 模拟亚音":%u.%uHz", CTCSS_Options_read/ 10,CTCSS_Options_read % 10);


#endif
#endif
        pPrintStr = String;
    } else {
//数字亚音
#if ENABLE_CHINESE_FULL == 0 || defined(ENABLE_ENGLISH)
        sprintf(String, 数字亚音":D%03oN", DCS_Options[gScanCssResultCode]);
#else
        uint8_t read_tmp[2];
        EEPROM_ReadBuffer(0x02C64+(gScanCssResultCode)*2, read_tmp, 2);
        uint16_t DCS_Options_read=read_tmp[0]|(read_tmp[1]<<8);
        sprintf(String, 数字亚音":D%03oN",DCS_Options_read);
#endif

        pPrintStr = String;
    }
    UI_PrintStringSmall(pPrintStr, 2, 0, 3);
    memset(String, 0, sizeof(String));

    if (gScannerSaveState == SCAN_SAVE_CHANNEL) {
        pPrintStr = 存置问;
        Start = 0;
        bCentered = 1;
    } else {
        Start = 2;
        bCentered = 0;
        if (gScannerSaveState == SCAN_SAVE_CHAN_SEL) {

//存置
            strcpy(String, 存置了);
#if ENABLE_CHINESE_FULL != 4 || defined(ENABLE_ENGLISH)

            UI_GenerateChannelStringEx(String + 3, gShowChPrefix, gScanChannel);
#else
            UI_GenerateChannelStringEx(String + 5, gShowChPrefix, gScanChannel);

#endif

            pPrintStr = String;
        } else if (gScanCssState < SCAN_CSS_STATE_FOUND) {

            //扫描
            strcpy(String, 扫描);
#if ENABLE_CHINESE_FULL != 4 || defined(ENABLE_ENGLISH)
            memset(String + 2, '.', (gScanProgressIndicator & 7) + 1);

#else
            memset(String + 4, '.', (gScanProgressIndicator & 7) + 1);

#endif
            pPrintStr = String;
        } else if (gScanCssState == SCAN_CSS_STATE_FOUND) {
            pPrintStr = 扫描" OK.";
        } else {
            pPrintStr = 扫描" FAIL.";
        }


    }
    UI_PrintStringSmall(pPrintStr, Start, bCentered ? 127 : 0, 5);
    ST7565_BlitFullScreen();
}
