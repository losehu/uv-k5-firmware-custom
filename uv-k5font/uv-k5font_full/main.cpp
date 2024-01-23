
#include<iostream>
#include<fstream>
#include<string>
#include <vector>
#include <array>
#include <map>
#include "font.h"
#include "bits/stdc++.h"
using namespace std;
#define IS_BIT_SET(byte, bit) ((byte>>bit) & (1))

ifstream file("../ALL_IN.txt"); // ?ùI????????????°§??
//ifstream file("../CHINESE7000_OUT.txt"); // ?ùI????????????°§??
ofstream outFile("../name_tmp.txt");
//ofstream out_chinese_array("../chinese_array.txt");
ofstream out_chinese_array("../chinese_array.txt", std::ios::binary);
ofstream output_file("../chinses_map.txt"); // ??????????????????????
string names[10000];
unsigned char chinese[10000][2];
unsigned char english[1000];
bool en_flag[256] = {false};
#define CAL
int init_file() {
    int lines = 0;
    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            // ????????????ß”??????????????????
            names[lines] = line;
            lines++;

        }

        file.close();
    } else {
        cout << "????????" << endl;
    }
    return lines;

}

bool sortByValue(const pair<array<unsigned char, 2>, int> &a, const pair<array<unsigned char, 2>, int> &b) {
    return a.second < b.second;
}

map<array<unsigned char, 2>, unsigned int> map_str;
map<array<unsigned char, 2>, unsigned int> all_code;

bool isGBKChineseCharacter(const string &str, size_t index) {
    // ???GBK????????????????
    if (index < str.size() - 1) {
        unsigned char firstByte = static_cast<unsigned char>(str[index]);
        unsigned char secondByte = static_cast<unsigned char>(str[index + 1]);
        if (firstByte >= 0x81 && firstByte <= 0xFE &&
            ((secondByte >= 0x40 && secondByte <= 0x7E) || (secondByte >= 0x80 && secondByte <= 0xFE))) {
            return true;
        }
    }
    return false;
}

void removeNullStrings(const std::string &inputFile, const std::string &outputFile) {
    std::ifstream input(inputFile, std::ios::binary);
    std::ofstream output(outputFile, std::ios::binary);

    if (!input.is_open()) {
        std::cerr << "Unable to open input file" << std::endl;
        return;
    }

    if (!output.is_open()) {
        std::cerr << "Unable to open output file" << std::endl;
        return;
    }

    std::string buffer;
    while (getline(input, buffer)) {
        size_t found = buffer.find("\\x00");
        while (found != std::string::npos) {
            buffer.erase(found, 4); // Erase 4 characters starting from found position
            found = buffer.find("\\x00", found);
        }
        output << buffer << std::endl;
    }

    input.close();
    output.close();
}


void set_bit(uint8_t *value, uint8_t bit_position, uint8_t bit_value) {
    if (bit_value == 0) {
        *value = *value & ~(1 << bit_position); // ?????¶À????? 0
    } else {
        *value = *value | (1 << bit_position);  // ?????¶À????? 1
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
void back_font(int num_show, unsigned char *font) { //??????????
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
    int num_names = init_file();
    // cout << num_names << endl;

    int num_chinese = 0;
    int num_english = 0;
    if (!outFile.is_open()) {
        return -5;
    }
    // ß’??????????


//        // ????????
//    outFile << "Hello, this is some text.\n";
//    outFile << "This is a new line.";

    for (int i = 0; i < num_names; i++) {


        for (size_t j = 0; j < names[i].size(); ++j) {
            if (isGBKChineseCharacter(names[i], j)) {

                array<unsigned char, 2> tmp = {0};
                tmp[0] = names[i][j];
                tmp[1] = names[i][j + 1];

                if (map_str.find(tmp) != map_str.end()) {
                } else {
                    // ???????????ßµ?????????????®Æ???????1
                    map_str[tmp] = num_chinese;
                    //   cout<<num_chinese<<":"<<tmp[0]<< tmp[1]<<endl;

                    chinese[num_chinese][0] = tmp[0];
                    chinese[num_chinese][0] = tmp[1];
                    //   outFile <<"\\x"<< hex << setw(2) << setfill('0') << uppercase <<  num_chinese << endl;

                    num_chinese++;
                }
                j++; // ????????????????????????????

            } else {

                if (en_flag[names[i][j]]) {

                } else {
                    //         ???????????ßµ?????????????®Æ???????1
                    en_flag[names[i][j]] = true;
                    english[num_english] = names[i][j];
                    num_english++;
                }
            }
        }
    }




    // outFile <<"\\x"<< hex << setw(2) << setfill('0') << uppercase <<   map_str[tmp] << endl;



    vector<pair<array<unsigned char, 2>, int>> vec(map_str.begin(), map_str.end());

    // ?????????????????????
    sort(vec.begin(), vec.end(), sortByValue);
    en_flag['\n'] = true;
    en_flag[' '] = true;

    for (int i = '!'; i <= '~'; i++) {
        en_flag[i] = true;
    }
    // ?????????????
    int now_code = 0x8000;
    for (const auto &pair: vec) {
        // cout << "{" << static_cast<int>(pair.first[0]) << ", " << static_cast<int>(pair.first[1]) << "} : " << pair.second << endl;
        array<unsigned char, 2> tmp = {0};
        tmp[0] = pair.first[0];
        tmp[1] = pair.first[1];
        if(now_code%256==0)
            now_code+=1;
        map_str[tmp] = now_code;

        array<unsigned char, 2> tmp1;
        tmp1[0] = tmp[0];
        tmp1[1] = tmp[1];
        all_code[tmp1] = map_str[tmp];

        now_code++;
//        cout << tmp[0] << tmp[1] << ":" <<hex <<(int) map_str[tmp] << endl;
        output_file << tmp[0] << tmp[1] << ":" << std::hex << (int)map_str[tmp] << std::endl;

    }


    for (int i = 0; i < num_names; i++) {
        for (size_t j = 0; j < names[i].size(); ++j) {
            if (isGBKChineseCharacter(names[i], j)) {

                array<unsigned char, 2> tmp = {0};
                tmp[0] = names[i][j];
                tmp[1] = names[i][j + 1];
                unsigned int value = all_code[tmp];
                unsigned char byte1 = (value >> 8) & 0xFF;
                unsigned char byte2 = value & 0xFF;

                outFile << "\\x" << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << (int) byte1;
                outFile.flush(); // ???????????????????

// Outputting byte2 as a character other than '\x00'
                if (byte2 == 0x00) {
                    outFile << "\\x\\x" << "0000";
                } else {
                    outFile << "\\x" << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << (int) byte2;
                }

                j++;
            } else {
                array<unsigned char, 2> tmp = {0};
                tmp[0] = 5;
                tmp[1] = names[i][j];
                outFile << "\\x" << hex << setw(2) << setfill('0') << uppercase << (int) tmp[1];
                //  outFile << "\\x" << hex << setw(2) << setfill('0') << uppercase << static_cast<unsigned int>(all_code[tmp]);
            }
        }
        outFile << endl;
    }

    outFile.close();

    std::string inputFile = "../name_tmp.txt"; // Replace with your input file name
    std::string outputFile = "../name_out.txt"; // Replace with your output file name
    removeNullStrings(inputFile, outputFile);


    cout << "chinese num:" << num_chinese << endl;
    cout << "english num" << num_english << endl;
    unsigned int NEW_FONT_BYTE = ceil((float) (CHN_FONT_NUM) * (float) (CHN_FONT_HIGH) * (float) (CHN_FONT_WIDTH) / 8);
    cout << NEW_FONT_BYTE << endl;
    unsigned char gFontChinese_out[NEW_FONT_BYTE] = {0};
//    return 0;

    int now_byte_index = 0;
    int now_bit_index = 0;
    for (int k = 0; k < CHN_FONT_NUM; k++) {//???
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


#ifdef CAL
        for (int i = 0; i < CHN_FONT_HIGH; ++i) {
            for (int j = 0; j < CHN_FONT_WIDTH; ++j) {
                if(bitmap[i][j])
                    set_bit(&gFontChinese_out[now_byte_index], now_bit_index, 1);
                now_bit_index++;
                if (now_bit_index == 8) {
                    now_bit_index = 0;
                    now_byte_index++;
                }
            }
        }
#else
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
#endif
    }

#ifndef CAL


    out_chinese_array<<"const uint8_t gFontChinese_out1["<<111590<<"]={"<<endl;
    for (int i = 0; i < 111590; i++) {
        out_chinese_array << "0X" << hex << setw(2) << setfill('0') << uppercase << (int) gFontChinese_out[i]<<",";
        if(i%20==0&&i!=0)out_chinese_array<<endl;
    }
    out_chinese_array<<"};"<<endl;
#else
    cout<<now_byte_index<<endl;
    out_chinese_array.write(reinterpret_cast<const char*>(gFontChinese_out), sizeof(gFontChinese_out));
#endif
    out_chinese_array.close();

cout<<"ƒ„"<<endl;
}



