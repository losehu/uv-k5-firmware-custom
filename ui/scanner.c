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

#include <stdbool.h>
#include <string.h>
#include "app/scanner.h"
#include "dcs.h"
#include "driver/st7565.h"
#include "external/printf/printf.h"
#include "misc.h"
#include "ui/helper.h"
#include "ui/scanner.h"

void UI_DisplayScanner(void)
{
	char    String[16];
	bool    bCentered;
	uint8_t Start;

	memset(gFrameBuffer, 0, sizeof(gFrameBuffer));

	memset(String, 0, sizeof(String));
	if (gScanSingleFrequency || (gScanCssState != SCAN_CSS_STATE_OFF && gScanCssState != SCAN_CSS_STATE_FAILED))

//ÆµÂÊ
    sprintf(String, "\x03\x12:%u.%05u", gScanFrequency / 100000, gScanFrequency % 100000);
    else
        strcpy(String, "\x03\x12:**.*****");
    UI_PrintStringSmall(String, 2, 0, 1);

	memset(String, 0, sizeof(String));
	if (gScanCssState < SCAN_CSS_STATE_FOUND || !gScanUseCssResult)
//Ä£ÄâÑÇÒô
    strcpy(String, "\x0F\x10\x0D\x0E:******");

	else
	if (gScanCssResultType == CODE_TYPE_CONTINUOUS_TONE)
   //Ä£ÄâÑÇÒô
        sprintf(String, "\x0F\x10\x0D\x0E:%u.%uHz", CTCSS_Options[gScanCssResultCode] / 10, CTCSS_Options[gScanCssResultCode] % 10);
	else
//Êý×ÖÑÇÒô
    sprintf(String, "\x0B\x0C\x0D\x0E:D%03oN", DCS_Options[gScanCssResultCode]);
    UI_PrintStringSmall(String, 2, 0, 3);

	memset(String, 0, sizeof(String));
	if (gScannerSaveState == SCAN_SAVE_CHANNEL)
	{
//´æÖÃ
        strcpy(String, "\x87\x88?");
		Start     = 0;
		bCentered = 1;
	}
	else
	{
		if (gScannerSaveState == SCAN_SAVE_CHAN_SEL) {

//´æÖÃ
            strcpy(String, "\x87\x88:");

			UI_GenerateChannelStringEx(String + 3, gShowChPrefix, gScanChannel);
		}
		else if (gScanCssState < SCAN_CSS_STATE_FOUND) {
	
            //É¨Ãè
            strcpy(String, "\x8F\x90");
            memset(String + 2, '.', (gScanProgressIndicator & 7) + 1);
		}
		else if (gScanCssState == SCAN_CSS_STATE_FOUND)
            //É¨Ãèok
        strcpy(String, "\x8F\x90 OK.");


        else
            //É¨Ãèfail
            strcpy(String, "\x8F\x90 FAIL.");
		Start     = 2;
		bCentered = 0;
	}
    UI_PrintStringSmall(String, Start, bCentered ? 127 : 0, 5);
	
	ST7565_BlitFullScreen();
}
