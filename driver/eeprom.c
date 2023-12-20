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

#include <stddef.h>
#include <string.h>

#include "driver/eeprom.h"
#include "driver/i2c.h"
#include "driver/system.h"

void EEPROM_ReadBuffer(uint16_t Address, void *pBuffer, uint8_t Size)
{
	I2C_Start();

	I2C_Write(0xA0);

	I2C_Write((Address >> 8) & 0xFF);
	I2C_Write((Address >> 0) & 0xFF);

	I2C_Start();

	I2C_Write(0xA1);

	I2C_ReadBuffer(pBuffer, Size);

	I2C_Stop();
}
void EEPROM_WriteBuffer(uint16_t Address, const void *pBuffer)
{
    if (pBuffer == NULL || Address >= 0x2000)
        return;


    uint8_t buffer[8];
    EEPROM_ReadBuffer(Address, buffer, 8);
    if (memcmp(pBuffer, buffer, 8) != 0)
    {
        I2C_Start();
        I2C_Write(0xA0);
        I2C_Write((Address >> 8) & 0xFF);
        I2C_Write((Address >> 0) & 0xFF);
        I2C_WriteBuffer(pBuffer, 8);
        I2C_Stop();
    }

    // give the EEPROM time to burn the data in (apparently takes 5ms)
    SYSTEM_DelayMs(8);
}

#define AT24C1024_ADDRESS 0xA4
#define AT24C1024_PAGE_SIZE 64

void E1EPROM_ReadBuffer_1024(uint32_t Address, void *pBuffer, uint16_t Size) {
    if (pBuffer == NULL || Address >= 0x20000) // Address limit for AT24C1024 (17-bit address)
        return;

    uint16_t bytesRead = 0;
    uint8_t* dataPointer = (uint8_t*)pBuffer;

    while (bytesRead < Size) {
        uint16_t bytesToRead = (Size - bytesRead < AT24C1024_PAGE_SIZE) ? (Size - bytesRead) : AT24C1024_PAGE_SIZE;
        uint16_t offset = Address % AT24C1024_PAGE_SIZE;

        I2C_Start();
        I2C_Write(AT24C1024_ADDRESS | ((Address >> 16) & 0x01)); // Sending device address and MSB of memory address
        I2C_Write((uint8_t)(Address >> 8)); // Sending address high byte
        I2C_Write((uint8_t)(Address & 0xFF)); // Sending address low byte
        I2C_Start();
        I2C_Write(AT24C1024_ADDRESS | 0x01); // Sending device address with read bit

        uint8_t buffer[AT24C1024_PAGE_SIZE];
        I2C_ReadBuffer(buffer, AT24C1024_PAGE_SIZE);

        // Copy the relevant portion of the buffer to the output buffer
        memcpy(dataPointer, buffer + offset, bytesToRead);

        bytesRead += bytesToRead;
        dataPointer += bytesToRead;
        Address += bytesToRead;

        I2C_Stop();
    }
}

void E1EPROM_WriteBuffer_1024(uint32_t Address, const void *pBuffer, uint16_t Size) {
    if (pBuffer == NULL || Address >= 0x20000) // Address limit for AT24C1024 (17-bit address)
        return;

    uint8_t buffer[AT24C1024_PAGE_SIZE];
    uint32_t endAddress = Address + Size;
    for (uint32_t addr = Address; addr < endAddress; addr += AT24C1024_PAGE_SIZE) {
        uint16_t bytesToWrite = (endAddress - addr < AT24C1024_PAGE_SIZE) ? (endAddress - addr) : AT24C1024_PAGE_SIZE;
        uint16_t offset = Address % AT24C1024_PAGE_SIZE;
        uint16_t remaining = AT24C1024_PAGE_SIZE - offset;
        bytesToWrite = (bytesToWrite < remaining) ? bytesToWrite : remaining;

        EEPROM_ReadBuffer(addr - offset, buffer, AT24C1024_PAGE_SIZE);
        if (memcmp(pBuffer, buffer + offset, bytesToWrite) != 0) {
            I2C_Start();
            I2C_Write(AT24C1024_ADDRESS | ((addr >> 16) & 0x01)); // Sending device address and MSB of memory address
            I2C_Write((uint8_t)(addr >> 8)); // Sending address high byte
            I2C_Write((uint8_t)(addr & 0xFF)); // Sending address low byte
            I2C_WriteBuffer(pBuffer, bytesToWrite);
            I2C_Stop();
        }
        pBuffer += bytesToWrite;
        // give the EEPROM time to burn the data in (assuming 5ms per page write)
        SYSTEM_DelayMs(5);
    }
}
