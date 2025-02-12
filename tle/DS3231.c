
#include <stddef.h>
#include <string.h>
#include "driver/eeprom.h"
#include "driver/i2c.h"
#include "driver/system.h"
#include "assert.h"
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "app/action.h"
#include "app/app.h"
#include "app/chFrScanner.h"
#include "app/dtmf.h"
#include "driver/uart.h"
#include "app/generic.h"
#include "app/main.h"
#include "app/menu.h"
#include "app/scanner.h"


#include "ARMCM0.h"
#include "audio.h"
#include "board.h"
#include "bsp/dp32g030/gpio.h"
#include "driver/backlight.h"
#include "driver/bk4819.h"
#include "driver/gpio.h"
#include "driver/keyboard.h"
#include "driver/st7565.h"
#include "driver/system.h"
#include "am_fix.h"
#include "bsp/dp32g030/rtc.h"
#include "frequencies.h"
#include "functions.h"
#include "helper/battery.h"
#include "misc.h"
#include "radio.h"
#include "settings.h"
#include "ui/battery.h"
#include "ui/inputbox.h"
#include "ui/main.h"
#include "ui/menu.h"
#include "ui/status.h"
#include "ui/ui.h"
#include "tle/DS3231.h"
//存放初始时间，数组内容顺序对应：秒、分、时、星期、日期、月、年。
// uint8_t DS3231_TimeConfig[7] = {00,50,23,4,29,2,24};
//存放实时时间
//存放设置闹钟信息

#define I2C_StartCondition I2C_Start
#define I2C_Send I2C_Write
#define I2C_StopCondition I2C_Stop

#define I2C_Receive I2C_Read

/**
 * @brief DS3231写单字节
 * @param WordAddress DS3231要写入的寄存器地址
 * @param byte 要写入的字节
*/
void DS3231_Write(uint8_t WordAddress,uint8_t byte)
{
    __disable_irq();

    I2C_StartCondition();
    I2C_Send(DS3231_I2C_ADDRESS_WR);
    I2C_Send(WordAddress);
    I2C_Send(byte);
    I2C_StopCondition();
    __enable_irq();

}
/**
 * @brief DS3231读单字节
 * @param WordAddress DS3231要读取的寄存器地址
 * @retval 接收到的字节
*/
uint8_t DS3231_Read(uint8_t WordAddress)
{
    __disable_irq();

    uint8_t byte;

    I2C_StartCondition();						//起始信号
    I2C_Send(DS3231_I2C_ADDRESS_WR);			//发从机地址写
    I2C_Send(WordAddress);						//发目标字地址
    I2C_StartCondition();						//再次起始信号
    I2C_Send(DS3231_I2C_ADDRESS_RD);			//发从机地址读
    byte = I2C_Receive(1);						//接收数据
    I2C_StopCondition();						//停止信号
    return byte;
    __enable_irq();

}

/**
 * @brief DS3231设置RTC时间信息，可在数组 DS3231_TimeConfig[] 中配置时间信息
*/
void DS3231_SetTime(uint8_t times_set[6])
{
    __disable_irq();

    // uint8_t i,temp_array[7];
    //存放初始时间，数组内容顺序对应：秒、分、时、星期、日期、月、年。

    uint8_t temp_array[7] = {times_set[5],times_set[4],times_set[3],4,times_set[2],times_set[1],times_set[0]};

    for (uint8_t i = 0; i < 7; i++)		//将初始时间数组引用到临时数组，循环将数组中内容转换为BCD码
        temp_array[i] = (temp_array[i] / 10*16) + (temp_array[i] % 10);

    I2C_StartCondition();
    I2C_Send(DS3231_I2C_ADDRESS_WR);
    I2C_Send(DS3231_ADDRESS_SECOND);		//写秒寄存器地址
    for (uint8_t i = 0; i < 7; i++)		//每次数据传送完成后DS3231内部寄存器将自动递增
        I2C_Send(temp_array[i]);
    // I2C_WriteBuffer(temp_array,7);
    I2C_StopCondition();
    __enable_irq();

}

/**
 * @brief DS3231获取当前时钟信息并存储到数组 DS3231_RTC[]
*/
void DS3231_GetTime(uint8_t get_time[6])
{
    __disable_irq();

    uint8_t DS3231_RTC[7];
    I2C_StartCondition();
    I2C_Send(DS3231_I2C_ADDRESS_WR);
    I2C_Send(DS3231_ADDRESS_SECOND);
    I2C_StartCondition();
    I2C_Send(DS3231_I2C_ADDRESS_RD);

    // I2C_ReadBuffer(DS3231_RTC, 7);


    for (uint8_t i = 0; i < 7; i++)		//每次数据传送完成后DS3231内部寄存器将自动递增
        //接收最后一个字节主机必须发送非应答
            DS3231_RTC[i] = (i < 6) ? I2C_Receive(0) : I2C_Receive(1);
    I2C_StopCondition();
    for (uint8_t i = 0; i < 7; i++)		//数组中内容转为DEC码
        DS3231_RTC[i] = (DS3231_RTC[i] / 16*10) + (DS3231_RTC[i] % 16);
    get_time[0] = DS3231_RTC[6];
    get_time[1] = DS3231_RTC[5];
    get_time[2] = DS3231_RTC[4];
    get_time[3] = DS3231_RTC[2];
    get_time[4] = DS3231_RTC[1];
    get_time[5] = DS3231_RTC[0];
    __enable_irq();

}