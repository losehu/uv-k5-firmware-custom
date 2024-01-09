//
// Created by RUPC on 2024/1/8.
//
#include "bsp/dp32g030/timer.h"
#include "ARMCM0.h"

 uint8_t TIM0_CNT=0;

void TIM0_ISR()
{
    TIM0_CNT++;

    TIMERBASE0_IF |= (1 << 0); // 写1清零 清除定时器中断状态

}
void TIM0_SET_PSC(uint16_t prescaler) {
    // 确保传入的分频系数在合法范围内（0 到 0xFFFF）
    if (prescaler <= 0xFFFF) {
        // 清除 DIV 位域
        TIMERBASE0_DIV &= ~(0xFFFF); // 通过与操作清除 DIV 位域的内容
        // 设置 DIV 位域为传入的分频系数
        TIMERBASE0_DIV |= prescaler;
    }
}
void TIM0_SET_ARR(uint16_t Arr) {
    // 确保传入的分频系数在合法范围内（0 到 0xFFFF）
    if (Arr <= 0xFFFF) {
        // 清除 DIV 位域
        TIMERBASE0_LOW_LOAD &= ~(0xFFFF); // 通过与操作清除 DIV 位域的内容
        // 设置 DIV 位域为传入的分频系数
        TIMERBASE0_LOW_LOAD |= Arr;
    }

}
void TIM0_INIT()
{
// Define TIMERBASE0 base address
#define TIMERBASE0_BASE 0x40064000


#define TIMERBASE_EN_OFFSET  0x00
#define TIMERBASE_IE_OFFSET  0x10
#define TIMERBASE_IF_OFFSET  0x14


        // Enable TIMERBASE0
        *((volatile unsigned int *)(TIMERBASE0_BASE + TIMERBASE_EN_OFFSET)) |= 0x1; // Enable LOW_EN

        // Enable Timer0 interrupt
        *((volatile unsigned int *)(TIMERBASE0_BASE + TIMERBASE_IE_OFFSET)) |= 0x1; // Enable LOW_IE

        // Enable global interrupts (assuming your MCU supports this)
        __enable_irq(); // Function to enable interrupts


//
//    TIM0_SET_PSC(480-1);//48000000/480/100=1000
//    TIM0_SET_ARR(1000);//10ms
//    TIMERBASE0_IF |= (1 << 1) | (1 << 0); // 写1清零 清除定时器中断状态
//    TIMERBASE0_IE |= (1 << 1) | (1 << 0); // 1高 0低 使能定时器中断
//    TIMERBASE0_EN |=(1 << 1) | (1 << 0);//1高 0低 使能定时器
//
//    __enable_irq();
}



