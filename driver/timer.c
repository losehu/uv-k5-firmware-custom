//
// Created by RUPC on 2024/1/8.
//
#include "bsp/dp32g030/timer.h"
#include "ARMCM0.h"
#include "app/spectrum.h"
#include "tle/tle.h"
KeyboardState my_kbd = {KEY_INVALID, KEY_INVALID, 0};

void TIM0_INIT() {
    TIMERBASE0_DIV = 480;
    TIMERBASE0_LOW_LOAD = 100; //0.001s
    NVIC_SetPriority(Interrupt5_IRQn, 0); /* set Priority for Systick Interrupt */
    TIMERBASE0_IF |= 0x01; // 写1清零 清除定时器中断状态
    TIMERBASE0_IE |= 0x01;
    TIMERBASE0_EN |= 0x01;
    NVIC_EnableIRQ(Interrupt5_IRQn);
}

uint8_t TIM0_CNT = 0;
uint8_t period=1;
void KEY_SCAN() {
    period=1;

    my_kbd.prev = my_kbd.current;
    my_kbd.current = GetKey();
    if (my_kbd.current == KEY_INVALID) {
        my_kbd.counter = 0;
        off_PTT=1;
    }

    if (my_kbd.current != KEY_INVALID && my_kbd.current == my_kbd.prev) {
        if (my_kbd.counter < 16)
            my_kbd.counter++;
        else
            my_kbd.counter -= 3;
        //        SYSTEM_DelayMs(20);
        if(my_kbd.counter>=2)            period=20;

    } else {
        my_kbd.counter = 0;

    }


    if (my_kbd.counter == 2 || my_kbd.counter == 16) {
        TLE_KEY();

    }

}
void TIM0Handler(void) {
    TIM0_CNT++;
    if(TIM0_CNT==period)
    {
        TIM0_CNT=0;
        KEY_SCAN();
    }


    TIMERBASE0_IF |= 0x01;

}




