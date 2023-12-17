#include <bits/stdc++.h>
#include "font.h"

using namespace std;
ofstream out_chinese_array("../chinese_array.txt");

void set_bit(uint8_t *value, uint8_t bit_position, uint8_t bit_value) {
    if (bit_value == 0) {
        *value = *value & ~(1 << bit_position);
    } else {
        *value = *value | (1 << bit_position);
    }
}

#define IS_BIT_SET(byte, bit) ((byte>>bit) & (1))

void show_font(unsigned char show_font[22]) {
    unsigned char bitmap[CHN_FONT_HIGH][CHN_FONT_WIDTH] = {0};
    for (int i = 0; i < CHN_FONT_WIDTH * 2; i++) {
        if (i < CHN_FONT_WIDTH) {
            for (int j = 0; j < 8; j++) {
                if (IS_BIT_SET(show_font[i], j))
                    bitmap[j][i] = 1;
            }
        } else {
            for (int j = 0; j < CHN_FONT_HIGH - 8; ++j) {
                bitmap[j + 8][i - CHN_FONT_WIDTH] = IS_BIT_SET(show_font[i], j);
            }
        }

    }
    for (int i = 0; i < CHN_FONT_HIGH; ++i) {
        for (int j = 0; j < CHN_FONT_WIDTH; ++j) {
            if (bitmap[i][j])
                printf("1");
            else
                printf("0");
        }
        printf("\n");

    }
}
bool check_font(unsigned char *font1,unsigned char *font2)
{
    return (memcmp(font1,font2,CHN_FONT_WIDTH*2)==0);
}
void back_font(int num_show, unsigned char *font) { //压缩转显存显示
    unsigned int local = CHN_FONT_HIGH * CHN_FONT_WIDTH * num_show / 8;
    unsigned int local_bit = (CHN_FONT_HIGH * CHN_FONT_WIDTH * num_show) % 8;
    unsigned char now_font[CHN_FONT_WIDTH * 2] = {0};
    for (int i = 0; i < CHN_FONT_WIDTH*2; ++i) {
        unsigned char j_end=8;
        if(i>=CHN_FONT_WIDTH)
            j_end=CHN_FONT_HIGH-8 ;

        for (int j = 0; j < j_end; ++j) {

            if (IS_BIT_SET(font[local], local_bit))
                set_bit(&now_font[i], j, 1);
            local_bit++;
            if (local_bit == 8) {
                local_bit = 0;
                local++;
            }
        }

    }
 if(!check_font(now_font,new_font[num_show]))
 {
     printf("SB\n");
 }
}

int main() {
    unsigned int NEW_FONT_BYTE = ceil((float) (CHN_FONT_NUM) * (float) (CHN_FONT_HIGH) * (float) (CHN_FONT_WIDTH) / 8);
    cout << NEW_FONT_BYTE << endl;
    unsigned char gFontChinese_out[NEW_FONT_BYTE] = {0};

//    show_font(new_font[0]);
//    return 0;
    int now_byte_index = 0;
    int now_bit_index = 0;
    for (int k = 0; k < CHN_FONT_NUM; k++) {//压缩
        unsigned char bitmap[CHN_FONT_HIGH][CHN_FONT_WIDTH] = {0};
        for (int i = 0; i < CHN_FONT_WIDTH * 2; i++) {
            if (i < CHN_FONT_WIDTH) {
                for (int j = 0; j < 8; j++) {
                    if (IS_BIT_SET(new_font[k][i], j))
                        bitmap[j][i] = 1;
                }
            } else {
                for (int j = 0; j < CHN_FONT_HIGH - 8; ++j) {
                    bitmap[j + 8][i - CHN_FONT_WIDTH] = IS_BIT_SET(new_font[k][i], j);
                }
            }
        }

        for (int i = 0; i < CHN_FONT_WIDTH; ++i) {
            for (int j = 0; j < 8; ++j) {
                if (bitmap[j][i])
                    set_bit(&gFontChinese_out[now_byte_index], now_bit_index, 1);

                now_bit_index++;
                if (now_bit_index == 8) {
                    now_bit_index = 0;
                    now_byte_index++;
                }

            }
        }

        for (int i = 0; i < CHN_FONT_WIDTH; ++i) {
            for (int j = 8; j < CHN_FONT_HIGH; ++j) {
                if (bitmap[j][i])
                    set_bit(&gFontChinese_out[now_byte_index], now_bit_index, 1);
                now_bit_index++;
                if (now_bit_index == 8) {
                    now_bit_index = 0;
                    now_byte_index++;
                }

            }
        }

        back_font(k, gFontChinese_out);

    }
    out_chinese_array<<"gFontChinese_out[SUM_BYTE]={"<<endl;
    for (int i = 0; i < NEW_FONT_BYTE; i++) {
        out_chinese_array << "0X" << hex << setw(2) << setfill('0') << uppercase << (int) gFontChinese_out[i]<<",";
        if(i%6==0&&i!=0)out_chinese_array<<endl;
    }
    out_chinese_array<<"};";
    out_chinese_array.close();


}
