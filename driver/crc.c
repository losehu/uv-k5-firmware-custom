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

#include "bsp/dp32g030/crc.h"
#include "driver/crc.h"
#include "driver/uart.h"

void CRC_Init(void)
{
    CRC_CR =
            CRC_CR_CRC_EN_BITS_DISABLE          |
            CRC_CR_INPUT_REV_BITS_NORMAL        |
            CRC_CR_INPUT_INV_BITS_BIT_INVERTED  |
            CRC_CR_OUTPUT_REV_BITS_REVERSED     |
            CRC_CR_OUTPUT_INV_BITS_BIT_INVERTED |
            CRC_CR_DATA_WIDTH_BITS_8            |
            CRC_CR_CRC_SEL_BITS_CRC_16_CCITT;

    CRC_IV = 0;

}

#ifdef ENABLE_MDC1200
void CRC_InitReverse(void)
	{
		CRC_CR =
			CRC_CR_CRC_EN_BITS_DISABLE          |
			CRC_CR_INPUT_REV_BITS_NORMAL        |
			CRC_CR_INPUT_INV_BITS_BIT_INVERTED  |
			CRC_CR_OUTPUT_REV_BITS_REVERSED     |
			CRC_CR_OUTPUT_INV_BITS_BIT_INVERTED |
			CRC_CR_DATA_WIDTH_BITS_8            |
			CRC_CR_CRC_SEL_BITS_CRC_16_CCITT;

		CRC_IV = 0;
	}
#endif
#define CRC16_XMODEM_POLY 0x1021

 uint16_t CRC_Calculate1( void *pBuffer, uint16_t Size)
{
     uint8_t *pData = ( uint8_t *)pBuffer;
    uint16_t crc = 0; // 初始CRC值为0

    while (Size--) {
        crc ^= (*pData++) << 8; // 将数据字节的最高位与CRC异或
        for (uint8_t i = 0; i < 8; i++) {
            if (crc & 0x8000) { // 检查最高位是否为1
                crc = (crc << 1) ^ CRC16_XMODEM_POLY; // 如果最高位为1，执行CRC多项式计算
            } else {
                crc = crc << 1; // 如果最高位为0，继续左移
            }
        }
    }

    return crc;
}

//uint16_t CRC_Calculate( void *pBuffer, uint16_t Size) {
//
//		unsigned int   i;
//		 uint8_t *data8 = ( uint8_t *)pBuffer;
//		uint16_t       crc = 0;
//		for (i = 0; i < Size; i++)
//		{
//			unsigned int k;
//			crc ^= data8[i];
//			for (k = 8; k > 0; k--)
//				crc = (crc & 1u) ? (crc >> 1) ^ 0x8408 : crc >> 1;
//		}
//		return crc ^ 0xffff;
//}
uint16_t CRC_Calculate(const void *buffer, const unsigned int size)
{
    const uint8_t *data = (const uint8_t *)buffer;
    uint16_t       i;
    uint16_t       crc;

    CRC_CR = (CRC_CR & ~CRC_CR_CRC_EN_MASK) | CRC_CR_CRC_EN_BITS_ENABLE;

    for (i = 0; i < size; i++)
        CRC_DATAIN = data[i];
    crc = (uint16_t)CRC_DATAOUT;

    CRC_CR = (CRC_CR & ~CRC_CR_CRC_EN_MASK) | CRC_CR_CRC_EN_BITS_DISABLE;

    return crc;
}
