//
// Created by RUPC on 2024/1/30.
//
#ifndef HARDWARE_DP32G030_RTC_H
#define HARDWARE_DP32G030_RTC_H
#include <stdint.h>

//RCLF 32768HZ
#define RTC_BASE_ADD 0x40069000
#define RTC_CFG_ADD (0x00+RTC_BASE_ADD) //配置寄存器
#define RTC_IE_ADD (0x04+RTC_BASE_ADD) //中断使能寄存器
#define RTC_IF_ADD (0x08+RTC_BASE_ADD)// 状态寄存器
#define RTC_PRE_ADD (0x10+RTC_BASE_ADD) // 预分频寄存器
#define RTC_TR_ADD (0x14+RTC_BASE_ADD) //时间寄存器
#define RTC_DR_ADD (0x18+RTC_BASE_ADD) // 日期寄存器
#define RTC_AR_ADD (0x1C+RTC_BASE_ADD) //闹钟寄存器
#define RTC_TSTR_ADD (0x20+RTC_BASE_ADD) //当前时间寄存器
#define RTC_TSDR_ADD (0x24+RTC_BASE_ADD) // 当前日期寄存器
#define RTC_CNT_ADD (0x28+RTC_BASE_ADD) // 秒标当前计数值
#define RTC_VALID_ADD (0x2C+RTC_BASE_ADD) //当前时间有效标志寄存器

#define RTC_CFG (*(volatile uint32_t *)RTC_CFG_ADD) //配置寄存器
#define RTC_IE (*(volatile uint32_t *)RTC_IE_ADD) //中断使能寄存器
#define RTC_IF (*(volatile uint32_t *)RTC_IF_ADD)// 状态寄存器
#define RTC_PRE (*(volatile uint32_t *)RTC_PRE_ADD) // 预分频寄存器
#define RTC_TR (*(volatile uint32_t *)RTC_TR_ADD) //时间寄存器
#define RTC_DR (*(volatile uint32_t *)RTC_DR_ADD) // 日期寄存器
#define RTC_AR (*(volatile uint32_t *)RTC_AR_ADD) //闹钟寄存器
#define RTC_TSTR (*(volatile uint32_t *)RTC_TSTR_ADD) //当前时间寄存器
#define RTC_TSDR (*(volatile uint32_t *)RTC_TSDR_ADD) // 当前日期寄存器
#define RTC_CNT (*(volatile uint32_t *)RTC_CNT_ADD) // 秒标当前计数值
#define RTC_VALID (*(volatile uint32_t *)RTC_VALID_ADD) //当前时间有效标志寄存器
#define RC_FREQ_DELTA (*(volatile uint32_t *)(0x40000000u+0x78u))
#define TRIM_RCLF (*(volatile uint32_t *)(0x40000800u+0x34u))
#define TEMP1 (*(volatile uint32_t *)(0x40000800u+0x20u))
#define TEMP2 (*(volatile uint32_t *)(0x40000800u+0x24u))

void RTC_INIT(void);
void RTC_Set( );
void RTC_Get();

extern uint8_t time[6];

#endif