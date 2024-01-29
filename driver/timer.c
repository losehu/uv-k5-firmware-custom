//
// Created by RUPC on 2024/1/8.
//
#include "bsp/dp32g030/timer.h"
#include "ARMCM0.h"



void TIM0_INIT()
{


    TIMERBASE0_DIV=48000;
    TIMERBASE0_LOW_LOAD=10000;

    // Enable TIMERBASE0
    NVIC_SetPriority (Interrupt5_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL); /* set Priority for Systick Interrupt */

    TIMERBASE0_IF = 0x01 ; // 写1清零 清除定时器中断状态
    TIMERBASE0_IE|= 0x01;

    TIMERBASE0_EN|= 0x01;
    NVIC_EnableIRQ(Interrupt5_IRQn);
    __enable_irq(); // Function to enable interrupts


}

uint32_t TIM0_CNT=0;
void TIM0Handler(void);

void TIM0Handler(void)
{
    TIMERBASE0_IF = 0x01 ;

    TIM0_CNT++;
}


