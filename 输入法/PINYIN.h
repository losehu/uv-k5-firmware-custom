//
// Created by RUPC on 2024/2/9.
//

#ifndef UNTITLED24_PINYIN_H
#define UNTITLED24_PINYIN_H
typedef unsigned int uint32_t;
typedef unsigned char uint8_t;
typedef int int32_t;
typedef unsigned short uint16_t;
 struct PINYIN_T
{
   const  char pinyin[7];
    uint8_t num;
    uint32_t add;
};
extern struct PINYIN_T pin[399] ;
extern char name[399][500];
#endif //UNTITLED24_PINYIN_H
