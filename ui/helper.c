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
#include "driver/uart.h"
#include "driver/st7565.h"
#include "external/printf/printf.h"
#include "font.h"
#include "ui/menu.h"
#include "ui/helper.h"
#include "ui/inputbox.h"
#include "misc.h"
#include "chinese.h"
#include "driver/eeprom.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof((arr)[0]))
#endif
#define IS_BIT_SET(byte, bit) ((byte>>bit) & (1))

void set_bit(uint8_t *value, uint8_t bit_position) {
//    if (bit_value == 0) {
//        *value = *value & ~(1 << bit_position);
//    } else {
    *value = *value | (1 << bit_position);
//    }
}

uint8_t is_chn(uint8_t num) {
#if ENABLE_CHINESE_FULL != 0
    if (num >= 0x80) return 1;
    return 255;
#else
    if (num >= 1 && num < 10)return num - 1;
    else if (num > 10 && num < 32)return num - 2;
    else if (num > 126 && num <= 233)return num - 97;
    else return 255;

#endif
}

bool isChineseChar(char a, uint8_t now_index, uint8_t sum_index) {
    if ((uint8_t) a >= 0x80 && now_index < sum_index) return 1;
    return 0;
}

void UI_GenerateChannelString(char *pString, const uint8_t Channel) {
    unsigned int i;

    if (gInputBoxIndex == 0) {
        sprintf(pString, "CH-%02u", Channel + 1);
        return;
    }

    pString[0] = 'C';
    pString[1] = 'H';
    pString[2] = '-';
    for (i = 0; i < 2; i++)
        pString[i + 3] = (gInputBox[i] == 10) ? '-' : gInputBox[i] + '0';
}

bool CHINESE_JUDGE(char *name, uint8_t len) {
    for (int i = 0; i < len; i++)
        if ((uint8_t) name[i] >= 0x80 && i != len - 1 && name[i + 1] != 0)return 1;

    return 0;
}


void UI_GenerateChannelStringEx(char *pString, const bool bShowPrefix, const uint8_t ChannelNumber) {
    if (gInputBoxIndex > 0) {
        for (unsigned int i = 0; i < 3; i++) {
            pString[i] = (gInputBox[i] == 10) ? '-' : gInputBox[i] + '0';
        }

        pString[3] = 0;
        return;
    }

    if (bShowPrefix) {
        // BUG here? Prefixed NULLs are allowed
        sprintf(pString, "CH-%03u", ChannelNumber + 1);
    } else if (ChannelNumber == 0xFF) {
        strcpy(pString, "NULL");
    } else {
        sprintf(pString, "%03u", ChannelNumber + 1);
    }
}


// Example usage:
// UI_PrintChar('A', 0, 0, 16);

// Example usage:
// UI_PrintChar('A', 0, 0, 8);

//void UI_PrintCharSmall(char character, uint8_t Start, uint8_t Line) {
//    const uint8_t char_width = ARRAY_SIZE(gFontSmall[0]);
//
//    // Calculate the position for the character
//    uint8_t *pFb = gFrameBuffer[Line] + Start + (char_width + 1) / 2;
//
//    // Display the character if it's a printable charactergFontBigDigits
//    if (character > ' ') {
//        const unsigned int index = (unsigned int) character - ' ' - 1;
//        if (index < ARRAY_SIZE(gFontSmall)) {
//            memmove(pFb, &gFontSmall[index], char_width);
//        }
//    }
//}

void UI_PrintStringSmall(const char *pString, uint8_t Start, uint8_t End, uint8_t Line) {

#ifdef ENABLE_ENGLISH
    const size_t Length = strlen(pString);
	size_t       i;

	const unsigned int char_width   = ARRAY_SIZE(gFontSmall[0]);
	const unsigned int char_spacing = char_width + 1;

	if (End > Start)
		Start += (((End - Start) - (Length * char_spacing)) + 1) / 2;


	uint8_t            *pFb         = gFrameBuffer[Line] + Start;
	for (i = 0; i < Length; i++)
	{
		if (pString[i] > ' ')
		{
			const unsigned int index = (unsigned int)pString[i] - ' ' - 1;
			if (index < ARRAY_SIZE(gFontSmall))
				memmove(pFb + (i * char_spacing) + 1, &gFontSmall[index], char_width);
		}
	}
#else

    bool flag_move = 0;

    uint8_t Length = strlen(pString);

    if (show_move_flag) {
#if ENABLE_CHINESE_FULL == 0
        Length = Length > 7 ? 7 : Length;
//#else
//        #ifdef ENABLE_PINYIN == 0
////        if(PINYIN_NUM==0) //拼音判断
//            #endif
//        Length = Length > 14 ? 14 : Length;

#endif
        flag_move = 1;
        show_move_flag = 0;

    }
    uint8_t sum_pixel = 0;
    uint16_t true_char[Length];
    uint8_t cn_flag[Length];
    uint8_t char_num = 0;
    for (size_t j = 0; j < Length; j++) {

        uint8_t chn_judge = is_chn(pString[j]);
        if (chn_judge == 255 && pString[j] != '\n' && pString[j] != '\0') {
            true_char[char_num] = pString[j];
            cn_flag[char_num] = 0;

            char_num++;
            sum_pixel += 7;
        } else if (chn_judge != 255) {
            cn_flag[char_num] = 1;

#if ENABLE_CHINESE_FULL != 0

            true_char[char_num] = (pString[j] << 8) | pString[j + 1];
            j++;
#else
            true_char[char_num] = chn_judge;
#endif

            flag_move = 1;
            char_num++;
            sum_pixel += 13;
        }
    }
    if (End > Start)
        Start += (((End - Start) - (sum_pixel)) + 1) / 2;
    uint8_t *pFb = gFrameBuffer[Line] + Start;
    uint8_t *pFb1 = gFrameBuffer[Line + 1] + Start;
    uint8_t now_pixel = 0;
    for (unsigned short i = 0; i < char_num; i++) {
        if (cn_flag[i] == 0) {
            if (true_char[i] > ' ') {
                const unsigned int index = (unsigned int) true_char[i] - ' ' - 1;
#if ENABLE_CHINESE_FULL == 0

                if (index < ARRAY_SIZE(gFontSmall)) {
                    if (flag_move) {
                        uint8_t gFontSmall_More[12] = {0};
                        for (int j = 0; j < 12; ++j) {
                            if (j < 6) gFontSmall_More[j] = (gFontSmall[index][j] & 0x1F) << 3;//00011111
                            else gFontSmall_More[j] = (gFontSmall[index][j - 6] & 0XE0)
                                        >> 5;//|(0xFB& *(pFb1+ now_pixel + 1+j-6));//11100000
                        }
                        memcpy(pFb + now_pixel + 1, &gFontSmall_More[0], 6);
                        memcpy(pFb1 + now_pixel + 1, &gFontSmall_More[6], 6);
                    } else
                        memcpy(pFb + now_pixel + 1, &gFontSmall[index], 6);
                }
#else
                if (index < 94) {
                    uint8_t read_gFontSmall[6];
                    EEPROM_ReadBuffer(0x0267C + index * 6, read_gFontSmall, 6);
                    if (flag_move) {
                        uint8_t gFontSmall_More[12] = {0};

                        for (int j = 0; j < 12; ++j) {
                            if (j < 6) gFontSmall_More[j] = (read_gFontSmall[j] & 0x1F) << 3;//00011111
                            else gFontSmall_More[j] = (read_gFontSmall[j - 6] & 0XE0)
                                        >> 5;//|(0xFB& *(pFb1+ now_pixel + 1+j-6));//11100000
                        }
                        memcpy(pFb + now_pixel + 1, &gFontSmall_More[0], 6);
                        memcpy(pFb1 + now_pixel + 1, &gFontSmall_More[6], 6);
                    } else
                        memcpy(pFb + now_pixel + 1, &read_gFontSmall, 6);
                }

#endif
                now_pixel += 7;
            } else if (true_char[i] == ' ')
                now_pixel += 7;
        } else {
//            uint8_t gFontChinese[22] = {0};

#if ENABLE_CHINESE_FULL != 0
            true_char[i] =
                    true_char[i] < 0XD8A1 ? ((true_char[i] - 0xB0A0) >> 8) * 94 + ((true_char[i] - 0xB0A0) & 0xff) - 1 :
                    ((true_char[i] - 0xB0A0) >> 8) * 94 + ((true_char[i] - 0xB0A0) & 0xFF) - 6;
            uint8_t tmp[17] = {0};
            unsigned int local = (CHN_FONT_HIGH * CHN_FONT_WIDTH * true_char[i]) >> 3;
            unsigned int local_bit = (CHN_FONT_HIGH * CHN_FONT_WIDTH * true_char[i]) & 7;
            EEPROM_ReadBuffer(local + 0x02E00, tmp, 17);
            local = 0;
            for (unsigned char k = 0; k < CHN_FONT_WIDTH * 2; ++k) {
                unsigned char j_end = 8;
                if (k >= CHN_FONT_WIDTH)
                    j_end = CHN_FONT_HIGH - 8;
                for (unsigned char j = 0; j < j_end; ++j) {
                    if (IS_BIT_SET(tmp[local], local_bit))
//                        set_bit(&gFontChinese[k], j, 1);
                        if (k < CHN_FONT_WIDTH) set_bit(pFb + now_pixel + 1 + k, j);

                        else set_bit(pFb1 + now_pixel + 1 + k - CHN_FONT_WIDTH, j);

                    local_bit++;
                    if (local_bit == 8) {
                        local_bit = 0;
                        local++;
                    }
                }
            }

#else
            unsigned int local = (CHN_FONT_HIGH * CHN_FONT_WIDTH * true_char[i]) / 8;
            unsigned int local_bit = (CHN_FONT_HIGH * CHN_FONT_WIDTH * true_char[i]) % 8;
            for (unsigned char k = 0; k < CHN_FONT_WIDTH * 2; ++k) {
                unsigned char j_end = 8;
                if (k >= CHN_FONT_WIDTH)
                    j_end = CHN_FONT_HIGH - 8;
                for (unsigned char j = 0; j < j_end; ++j) {
                    if (IS_BIT_SET(gFontChinese_out[local], local_bit))
//                        set_bit(&gFontChinese[k], j, 1);

                    //                        set_bit(&gFontChinese[k], j, 1);

                        if(k<CHN_FONT_WIDTH)                         set_bit(pFb + now_pixel + 1+k, j);
                        else set_bit(pFb1 + now_pixel + 1+k-CHN_FONT_WIDTH, j);

                    local_bit++;
                    if (local_bit == 8) {
                        local_bit = 0;
                        local++;
                    }
                }
            }
//            memcpy(pFb + now_pixel + 1, &gFontChinese[0], 11);
//            memcpy(pFb1 + now_pixel + 1, &gFontChinese[11], 11);
#endif

            now_pixel += 13;
        }
    }
#endif
}


void UI_PrintStringSmallBuffer(const char *pString, uint8_t *buffer) {
    size_t i;
    const unsigned int char_width = ARRAY_SIZE(gFontSmall[0]);
    for (i = 0; i < strlen(pString); i++) {
        if (pString[i] > ' ') {
            const unsigned int index = (unsigned int) pString[i] - ' ' - 1;
#if ENABLE_CHINESE_FULL == 4
            if (index < 94) {
                uint8_t read_gFontSmall[6];
                EEPROM_ReadBuffer(0x267C + index * 6, read_gFontSmall, 6);
                memcpy(buffer + (i * (char_width + 1)) + 1, &read_gFontSmall, char_width);
            }
#else
            if (index < ARRAY_SIZE(gFontSmall))
                memcpy(buffer + (i * (char_width + 1)) + 1, &gFontSmall[index], char_width);
#endif

        }
    }
}

void UI_DisplayFrequency(const char *string, uint8_t X, uint8_t Y, bool center) {
    const unsigned int char_width = 13;
    uint8_t *pFb0 = gFrameBuffer[Y] + X;
    uint8_t *pFb1 = pFb0 + 128;
    bool bCanDisplay = false;

    uint8_t len = strlen(string);
    for (int i = 0; i < len; i++) {
        char c = string[i];
        if (c == '-') c = '9' + 1;
        if (bCanDisplay || c != ' ') {
            bCanDisplay = true;
            if (c >= '0' && c <= '9' + 1) {
#if ENABLE_CHINESE_FULL == 4
                uint8_t read_gFontBigDigits[20];
                EEPROM_ReadBuffer(0x02480 + 20 * (c - '0'), read_gFontBigDigits, 20);

                memcpy(pFb0 + 2, read_gFontBigDigits, char_width - 3);
                memcpy(pFb1 + 2, read_gFontBigDigits + char_width - 3, char_width - 3);
#else
                memcpy(pFb0 + 2, gFontBigDigits[c - '0'], char_width - 3);
                memcpy(pFb1 + 2, gFontBigDigits[c - '0'] + char_width - 3, char_width - 3);
#endif
            } else if (c == '.') {
                *pFb1 = 0x60;
                pFb0++;
                pFb1++;
                *pFb1 = 0x60;
                pFb0++;
                pFb1++;
                *pFb1 = 0x60;
                pFb0++;
                pFb1++;
                continue;
            }

        } else if (center) {
            pFb0 -= 6;
            pFb1 -= 6;
        }
        pFb0 += char_width;
        pFb1 += char_width;
    }
}


void UI_DrawPixelBuffer(uint8_t (*buffer)[128], uint8_t x, uint8_t y, bool black) {
    const uint8_t pattern = 1 << (y % 8);
    if (black)
        buffer[y / 8][x] |= pattern;
    else
        buffer[y / 8][x] &= ~pattern;
}


void UI_DisplayPopup(const char *string) {
    for (uint8_t i = 0; i < 7; i++) {
        memset(gFrameBuffer[i], 0x00, 128);
    }

    // for(uint8_t i = 1; i < 5; i++) {
    // 	memset(gFrameBuffer[i]+8, 0x00, 111);
    // }

    // for(uint8_t x = 10; x < 118; x++) {
    // 	UI_DrawPixel(x, 10, true);
    // 	UI_DrawPixel(x, 46-9, true);
    // }

    // for(uint8_t y = 11; y < 37; y++) {
    // 	UI_DrawPixel(10, y, true);
    // 	UI_DrawPixel(117, y, true);
    // }
    // DrawRectangle(9,9, 118,38, true);

    UI_PrintStringSmall(string, 9, 118, 2);
    //按EXIT键
    UI_PrintStringSmall(按EXIT键, 9, 118, 5);
}

void UI_DisplayClear() {
    memset(gFrameBuffer, 0, sizeof(gFrameBuffer));

}
// GUI functions

void PutPixel(uint8_t x, uint8_t y, bool fill) {
    UI_DrawPixelBuffer(gFrameBuffer, x, y, fill);
}

void PutPixelStatus(uint8_t x, uint8_t y, bool fill) {
    UI_DrawPixelBuffer(&gStatusLine, x, y, fill);
}


void DrawVLine(int sy, int ey, int nx, bool fill) {
    for (int i = sy; i <= ey; i++) {
        if (i < 56 && nx < 128) {
            PutPixel(nx, i, fill);
        }
    }
}

void GUI_DisplaySmallest(const char *pString, uint8_t x, uint8_t y,
                         bool statusbar, bool fill) {
    uint8_t c;
    uint8_t pixels;
    const uint8_t *p = (const uint8_t *) pString;

    while ((c = *p++) && c != '\0') {
        c -= 0x20;
#if ENABLE_CHINESE_FULL != 0
        uint8_t read_gFont3x5[3];
        EEPROM_ReadBuffer(0x0255C + c * 3, read_gFont3x5, 3);
        for (int i = 0; i < 3; ++i) {
            pixels = read_gFont3x5[i];
#else
            for (int i = 0; i < 3; ++i) {
                pixels = gFont3x5[c][i];
#endif
            for (int j = 0; j < 6; ++j) {
                if (pixels & 1) {
                    if (statusbar)
                        PutPixelStatus(x + i, y + j, fill);
                    else
                        PutPixel(x + i, y + j, fill);
                }
                pixels >>= 1;
            }
        }
        x += 4;
    }
}

void show_uint32(uint32_t num, uint8_t line) {
    memset(gFrameBuffer[line],0,128);
    char str[20] = {0};
    sprintf(str, "%d", num);
    UI_PrintStringSmall(str, 0, 127, line);
    ST7565_BlitFullScreen();
}

void show_hex(uint32_t num, uint8_t line) {
    memset(gFrameBuffer[line],0,128);
    char str[20] = {0};
    sprintf(str, "%X", num);
    UI_PrintStringSmall(str, 0, 127, line);
    ST7565_BlitFullScreen();
}
