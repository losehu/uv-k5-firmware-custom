//
// Created by RUPC on 2024/2/9.
//
#include "app/pinyin.h"
#include "string.h"
#include "driver/eeprom.h"
uint8_t pinyin_cmp(uint8_t *a, uint8_t *b) {
    for (int i = 0; i < 6; i++) {
        if (a[i] > b[i]) return 1;
        else if (a[i] < b[i]) return 0;
    }
    return 2;
}
int8_t pinyin_search(  uint8_t *target,uint8_t size,uint32_t *add) {
    int low = 0;
    int high = 398;
    uint8_t target_all[6];
    memset(target_all,' ',6);
    memcmp(target_all,target,size);
    while (low <= high) {
        int mid = low + (high - low) / 2;
        uint8_t tmp[11];
        EEPROM_ReadBuffer(0x20000+mid*16,tmp,11);
        uint8_t cmp=pinyin_cmp(tmp,target);
        if (cmp== 2) {
            *add=tmp[7]|(tmp[8]<<8)|(tmp[9]<<16)|(tmp[10]<<24);
            return (int8_t )tmp[6];
        }
        else if (cmp== 0) {
            low = mid + 1;
        }
        else {
            high = mid - 1;
        }
    }
    return -1; // 如果未找到目标元素，返回-1
}
void PINYIN()
{

}