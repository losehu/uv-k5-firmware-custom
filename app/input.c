//
// Created by RUPC on 2024/6/5.
//

#include "input.h"
uint32_t tempFreq;
char freqInputString[11];
uint8_t freqInputIndex = 0;
KEY_Code_t freqInputArr[10];
uint8_t freqInputDotIndex = 0;
static void ResetFreqInput() {
    tempFreq = 0;
    for (int i = 0; i < 10; ++i) {
        freqInputString[i] = '-';
    }
}

 void FreqInput() {
    freqInputIndex = 0;
    freqInputDotIndex = 0;
    ResetFreqInput();
}
 void RenderFreqInput() {
    UI_PrintStringSmall(freqInputString, 2, 127, 0);
//    show_uint32(tempFreq,3);
}
uint32_t UpdateFreqInput(KEY_Code_t key) {
    uint32_t tempFreq=0;
    if (key != KEY_EXIT && freqInputIndex >= 10) {
        return UINT32_MAX;
    }
    if (key == KEY_STAR) {
        if (freqInputIndex == 0 || freqInputDotIndex) {
            return UINT32_MAX;
        }
        freqInputDotIndex = freqInputIndex;
    }
    if (key == KEY_EXIT) {
        freqInputIndex--;
        if (freqInputDotIndex == freqInputIndex)
            freqInputDotIndex = 0;
    } else {
        freqInputArr[freqInputIndex++] = key;
    }

    ResetFreqInput();

    uint8_t dotIndex =freqInputDotIndex == 0 ? freqInputIndex : freqInputDotIndex;

    KEY_Code_t digitKey;
    for (int i = 0; i < 10; ++i) {
        if (i < freqInputIndex) {
            digitKey = freqInputArr[i];
            freqInputString[i] = digitKey <= KEY_9 ? '0' + digitKey : '.';
        } else {
            freqInputString[i] = '-';
        }
    }

    uint32_t base = 100000; // 1MHz in BK units

    for (int i = dotIndex - 1; i >= 0; --i) {
        tempFreq += (freqInputArr[i]) * base;
        base *= 10;
    }

    base = 10000; // 0.1MHz in BK units
    if (dotIndex < freqInputIndex) {
        for (int i = dotIndex + 1; i < freqInputIndex; ++i) {
            tempFreq += (freqInputArr[i]) * base;
            base /= 10;
        }
    }
    return tempFreq;
}