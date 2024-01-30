//
// Created by RUPC on 2024/1/30.
//
#include "bsp/dp32g030/rtc.h"

void RTC_INIT() {

    RTC_PRE |= (32768 - 1)//PRE_ROUND=32768HZ-1
               | (0 << 20)//DECIMAL=0
               | (0 << 24);//PRE_PERIOD=8s
    RTC_TR = (2 << 24)//day 2
             | (1 << 20) //h十位
             | (1 << 16)//h个位
             | (4 << 12)//min十位
             | (7 << 8)//min个位
             | (3 << 4)//sec十位
             | (0 << 0);//sec个位
    RTC_DR = (2 << 20)//YEAR TEN
             | (4 << 16) //YEAR ONE
             | (0 << 12)//MONTH TEN
             | (1 << 8)//MONTH ONE
             | (3 << 4)//DAY TEN
             | (0 << 0);//DAY ONE
    RTC_CFG |= (1 << 2)//打开设置时间功能
            |(1<<0);//RTC使能

}