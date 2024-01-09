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

#ifndef UI_BATTERY_H
#define UI_BATTERY_H
#ifdef DENABLE_SHOW_BAT_SYMBOL
#include <stdint.h>
void UI_DrawBattery(uint8_t* bitmap, uint8_t level, uint8_t blink);
void UI_DisplayBattery(uint8_t Level, uint8_t blink);

#endif
#endif
