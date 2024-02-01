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

#ifndef DRIVER_CRC_H
#define DRIVER_CRC_H

#include <stdint.h>

void CRC_Init(void);
uint16_t CRC_Calculate(const void *buffer, const unsigned int size);
uint16_t CRC_Calculate1( void *pBuffer, uint16_t Size);
uint16_t compute_crc(const void *data, const unsigned int data_len) ;    // let the CPU's hardware do some work :)

void CRC_InitReverse(void);

#endif

