//
// Created by RUPC on 2024/1/30.
//
#include "bsp/dp32g030/rtc.h"
#include "ARMCM0.h"
#include "driver/system.h"
uint8_t time[6]={24, 2, 1, 21, 52, 00};
void RTC_INIT() {

    RTC_PRE |= (32768 - 1)//PRE_ROUND=32768HZ-1
               | (0 << 20)//DECIMAL=0
               | (0 << 24);//PRE_PERIOD=8s
    RTC_Set(time);

    NVIC_SetPriority(Interrupt2_IRQn, 0);

    RTC_IF |= (1 << 5);//清除中断标志位
    RTC_IE |= (1 << 5);//使能半秒中断

    RTC_CFG |= (1 << 2)//打开设置时间功能
               | (1 << 0);//RTC使能

    NVIC_EnableIRQ(Interrupt2_IRQn);

}

void RTC_Set(uint8_t time[6]) {

    RTC_DR = (2 << 24)//day 2
             | (time[0] / 10 << 20)//YEAR TEN
             | (time[0] % 10 << 16) //YEAR ONE
             | (time[1] / 10 << 12)//MONTH TEN
             | (time[1] % 10 << 8)//MONTH ONE
             | (time[2] / 10 << 4)//DAY TEN
             | (time[2] % 10 << 0);//DAY ONE
    RTC_TR = (time[3] / 10 << 20) //h十位
             | (time[3] % 10 << 16)//h个位
             | (time[4] / 10 << 12)//min十位
             | (time[4] % 10 << 8)//min个位
             | (time[5] / 10 << 4)//sec十位
             | (time[5] % 10 << 0);//sec个位
}

void RTC_Get() {
    time[0]=(RTC_TSDR>>20&0b1111)*10+(RTC_TSDR>>16&0b1111);
    time[1]=(RTC_TSDR>>12&0b1)*10+(RTC_TSDR>>8&0b1111);
    time[2]=(RTC_TSDR>>4&0b1111)*10+(RTC_TSDR>>0&0b1111);

    time[3]=(RTC_TSTR>>20&0b111)*10+(RTC_TSTR>>16&0b1111);
    time[4]=(RTC_TSTR>>12&0b111)*10+(RTC_TSTR>>8&0b1111);
    time[5]=(RTC_TSTR>>4&0b111)*10+(RTC_TSTR>>0&0b1111);

}