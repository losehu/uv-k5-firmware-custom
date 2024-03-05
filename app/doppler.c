#include "doppler.h"
#include "string.h"
#include "driver/eeprom.h"
#include "bsp/dp32g030/rtc.h"
#include "ui/helper.h"

struct satellite_t satellite;
struct satellite_d satellite_data;
bool DOPPLER_FLAG = true;
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

void INIT_DOPPLER_DATA() {
    memset(&satellite, 0, sizeof(satellite));
    EEPROM_ReadBuffer(0x02BA0, &satellite, sizeof(satellite));
    if (satellite.name[9] != 0 ||
        !(satellite.name[0] >= 32 && satellite.name[0] <= 126)
            ) {
        DOPPLER_FLAG = 0;
        return;
    }

    for (int i = strlen(satellite.name); i < 10; i++)
        if (satellite.name[i] != 0) {
            DOPPLER_FLAG = 0;
            return;
        }


}

// 判断是否是闰年
int is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// 计算某个月的天数
int days_in_month(int year, int month) {
    int days[] = {31, 28 + is_leap_year(year), 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return days[month - 1];
}


int32_t UNIX_TIME(uint8_t time2[6]) {
    // 2000 年 1 月 1 日的年、月、日、时、分、秒
    int32_t seconds = 0;
    // 计算年份之间的秒数差
    for (int year = 0; year < time2[0]; year++) {
        seconds += (is_leap_year(year) + 365) * 24 * 3600;
    }

    // 计算当年之内的秒数差
    for (int month = 1; month < time2[1]; month++) {
        seconds += days_in_month(time2[0], month) * 24 * 3600;
    }
    // 计算当月之内的秒数差
    seconds += (time2[2] - 1) * 24 * 3600;
    seconds += time2[3] * 3600;
    seconds += time2[4] * 60;
    seconds += time2[5];
    return seconds;
}

void READ_DATA(int32_t time_diff, int32_t time_diff1) {
    int32_t n = -time_diff;
    if (time_diff <= 0 && time_diff1 >= 0)//正在过境
    {

        if ((n & 0x01) != 0)return;
        n = n >> 1;
    } else n = 0;

    EEPROM_ReadBuffer(0x1E200 + (n << 3), &satellite_data, sizeof(satellite_data));

//    AZ（-180~180，两位浮点，度）2B,EI（-180~180，两位浮点，度）2B,上行频率/10（正整数hz）4B、下行频率/10(正整数hz)4B、距离（两位浮点，km）3B：
//    第1B~2B:AZ的数字部分，只有正，低位在前高位在后，
//    低1~8位为AZ整数部分，8bit（0~180）
//    低9~16位为AZ浮点部分，8bit（0~99）
//    如：-179.85，那么为10110011（179）01010101（85）
//
//    第3B~4B:EI的数字部分,与AZ同理
//    第5B:AZ,EI的符号，低位在前高位在后，前4bit为AZ符号，后4bit为EI符号
//    4bit为0XA时表示正，0XC表示负
//    第6B~9B:上行频率/10,只有正,如:438.5MHZ，那么为438,500,00，都是低位在前，高位在后
//    第10B~13B:下行频率/10,只有正,如:144.5MHZ，那么为144,500,00，都是低位在前，高位在后
//    第14B~15B:距离整数部分，只有正，如：6748.85，那么为6748
//    第16B:距离浮点部分*100,只有正，如：6748.85，那么为85



}