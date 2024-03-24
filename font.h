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

#ifndef FONT_H
#define FONT_H
#include "stdbool.h"
#include <stdint.h>

extern const uint8_t gFontChinese_out[2261];
#define CHN_FONT_WIDTH 11U
#define CHN_FONT_HIGH 12U
#if  ENABLE_CHINESE_FULL ==4 &&!defined(ENABLE_ENGLISH)
extern bool audio_keep_flag;

#define MAX_EDIT_INDEX 13
#else
#define MAX_EDIT_INDEX 10
#endif
extern bool show_move_flag;

extern const uint8_t gFontBigDigits[11][20/*20*/];
extern const uint8_t gFont3x5[96][3];
extern const uint8_t gFontSmall[95 - 1][6];
extern const uint8_t font4[1814];



#endif

