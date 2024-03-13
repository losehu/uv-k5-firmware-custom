//
// Created by RUPC on 2024/3/10.
//
#include "4732.h"
#include "bsp/dp32g030/gpio.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/system.h"
#include "frequencies.h"
#include "misc.h"
#include "ui/helper.h"
void SI4732_Init()
{
    RST_HIGH;

}

void SI4732_ReadBuffer(uint8_t *Value,uint8_t size) {
    I2C_Start();
    I2C_Write(0x23);
    I2C_ReadBuffer(Value, size);
    I2C_Stop();
}

uint8_t SI4732_Read(bool bFinal) {
    uint8_t Value;
    I2C_Start();
    I2C_Write(0x23);
    Value=  I2C_Read(1);
    I2C_Stop();
    return Value;
}

void SI4732_WriteRegister(uint8_t Register, uint16_t Value) {
    I2C_Start();
    I2C_Write(0x22);
    I2C_Write((Register << 1) | I2C_WRITE);
    Value = ((Value >> 8) & 0xFF) | ((Value & 0xFF) << 8);
    I2C_WriteBuffer(&Value, sizeof(Value));
    I2C_Stop();
}
int SI4732_WriteBuffer(uint8_t *buff,uint8_t size) {
    I2C_Start();
    if(I2C_WriteBuffer(buff, size)<0)
    {
        while(1)
        {
            show_uint32(8888, 0);

        }
        I2C_Stop();
        return -1;
    }
    I2C_Stop();
    return 0;
}