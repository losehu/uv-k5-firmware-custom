//
// Created by RUPC on 2024/6/5.
//

#ifndef UV_K5_FIRMWARE_CUSTOM_INPUT_H
#define UV_K5_FIRMWARE_CUSTOM_INPUT_H
#include "ui/helper.h"
#include "driver/keyboard.h"
extern uint32_t tempFreq;
extern char freqInputString[11];
extern uint8_t freqInputIndex ;
extern uint8_t freqInputDotIndex ;
static void ResetFreqInput() ;
uint32_t UpdateFreqInput(KEY_Code_t key) ;
 void RenderFreqInput() ;
    void FreqInput() ;
extern KEY_Code_t freqInputArr[10];

#endif //UV_K5_FIRMWARE_CUSTOM_INPUT_H
