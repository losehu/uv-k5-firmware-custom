#include "doppler.h"
#include "string.h"
#include "driver/eeprom.h"
#include "bsp/dp32g030/rtc.h"
#include "ui/helper.h"
struct satellite_t satellite;
//0x02BA0~0x2BA9 10B,卫星名称,首字符在前,最多9个英文，最后一个为'\0'
//
//
//0x2BAA 1B,开始过境时间的年份的十位个位，0~99，如:2024即为24
//0x2BAB 1B,开始过境时间的月份，1~12
//0x2BAC 1B,开始过境时间的日期，1~31
//0x2BAD 1B,开始过境时间的时，0~23
//0x2BAE 1B,开始过境时间的分，0~59
//0x2BAF 1B,开始过境时间的秒，0~59
//
//0x2BB0 1B,离境时间的年份的十位个位，0~99，如:2077即为77
//0x2BB1 1B,离境时间的月份，1~12
//0x2BB2 1B,离境时间的日期，1~31
//0x2BB3 1B,离境时间的时，0~23
//0x2BB4 1B,离境时间的分，0~59
//0x2BB5 1B,离境时间的秒，0~59
//
//0x2BB6~0x2BB7 2B，总的过境时间（秒），高位在前，低位在后
//
//0x2BB8~0x2BB9 2B，手台的发射亚音
//0x2BBA~0x2BBB 2B，手台的接收亚音
#include <stdint.h>

void uint16_to_uint8_array(uint16_t value, uint8_t array[2]) {
    array[0] = value & 0xFF;        // 获取低8位
    array[1] = (value >> 8) & 0xFF; // 获取高8位
}

void INIT_DOPPLER_DATA()
{
    memset(&satellite, 0, sizeof(satellite));
    EEPROM_ReadBuffer(0x02BA0,&satellite,sizeof(satellite) );
}