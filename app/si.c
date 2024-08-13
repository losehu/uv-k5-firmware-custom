#include "si.h"
#include "../driver/bk4819.h"
#include "../driver/si473x.h"
#include "../helper/rds.h"
#include "../misc.h"
#include "app/spectrum.h"

//
// Created by RUPC on 2024/3/10.
//
#include "board.h"

#include "driver/si473x.h"
#include "bsp/dp32g030/gpio.h"
#include "bsp/dp32g030/syscon.h"

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/system.h"
#include "frequencies.h"
#include "misc.h"
#include "ui/helper.h"
#include <string.h>
#include "./si.h"
#include "app/fm.h"
#include "driver/st7565.h"
#include "external/printf/printf.h"
#include "misc.h"
#include "settings.h"
#include "ui/fmradio.h"
#include "ui/helper.h"
#include "ui/inputbox.h"
#include "ui/ui.h"
#include "chinese.h"
#include "si.h"
#include "helper/rds.h"
#include "functions.h"
#include "stdbool.h"
#include "app/spectrum.h"
#include "am_fix.h"
#include "audio.h"
#include "misc.h"
#include "driver/eeprom.h"
#include "driver/backlight.h"
#include "frequencies.h"
#include "ui/helper.h"
#include "ui/main.h"
#include "driver/backlight.h"

#include <stdint.h>

typedef enum {
    FM_BT,
    MW_BT,
    SW_BT,
    LW_BT,
} BandType;

static const char SI47XX_BW_NAMES[5][6] = {
        "6 kHz", "4 kHz", "3 kHz", "2 kHz", "1 kHz",
};

static const char SI47XX_SSB_BW_NAMES[6][8] = {
        "1.2 kHz", "2.2 kHz", "3 kHz", "4 kHz", "0.5 kHz", "1 kHz",
};

static const char SI47XX_MODE_NAMES[5][4] = {
        "FM", "AM", "LSB", "USB", "CW",
};

static SI47XX_FilterBW bw = SI47XX_BW_6_kHz;
static SI47XX_SsbFilterBW ssbBw = SI47XX_SSB_BW_3_kHz;
static int8_t currentBandIndex = -1;
bool SI_run = true;
typedef struct // Band data
{
    const char *bandName; // Bandname
//    BandType bandType;    // Band type (FM, MW or SW)
//    SI47XX_MODE prefmod;  // Pref. modulation
    uint16_t minimumFreq; // Minimum frequency of the band
    uint16_t maximumFreq; // maximum frequency of the band
//    uint16_t currentFreq; // Default frequency or current frequency
//    uint8_t currentStep;  // Default step (increment and decrement)
//    int lastBFO;          // Last BFO per band
//    int lastmanuBFO;      // Last Manual BFO per band using X-Tal

} SIBand;

SIBand bands[] = {
        {"LW"/*,         LW_BT, SI47XX_AM*/,  148,   283/*,   198,   9, 0, 0*/},         //  LW          1
        {"LW"/*,         LW_BT, SI47XX_AM*/,  100,   514/*,   198,   9, 0, 0*/},         //  LW          1
        {"MW Bcast"/*,   MW_BT, SI47XX_AM*/,  526,   1606/*,  1395,  9, 0, 0*/}, //  MW          2
        {"MW"/*,         MW_BT, SI47XX_AM*/,  514,   1800/*,  1395,  9, 0, 0*/},       //  MW          2
        {"BACON Ham"/*,  LW_BT, SI47XX_AM*/,  280,   470/*,   284,   1, 0, 0*/},  // Ham  800M 3
        {"630M Ham"/*,   SW_BT, SI47XX_LSB*/, 470,   480/*,   475,   1, 0, 0*/},  // Ham  630M 4
        {"160M Ham"/*,   SW_BT, SI47XX_LSB*/, 1800,  2000/*,  1850,  1, 0, 0*/},  // Ham  160M 5
        {"120M Bcast"/*, SW_BT, SI47XX_AM*/,  2300,  2495/*,  2400,  5, 0, 0*/}, //      120M 6
        {"120M"/*,       SW_BT, SI47XX_AM*/,  2000,  3200/*,  2400,  5, 0, 0*/},       //      120M 6
        {"90M Bcast"/*,  SW_BT, SI47XX_AM*/,  3200,  3400/*,  3300,  5, 0, 0*/},  //       90M 7
        {"90M"/*,        SW_BT, SI47XX_AM*/,  3200,  3500/*,  3300,  5, 0, 0*/},        //       90M 7
        {"80M Ham"/*,    SW_BT, SI47XX_LSB*/, 3500,  3900/*,  3630,  1, 0, 0*/},   // Ham   80M 8
        {"75M Bcast"/*,  SW_BT, SI47XX_AM*/,  3900,  4000/*,  3950,  5, 0, 0*/},  //       75M 9
        {"75M Bacst"/*,  SW_BT, SI47XX_AM*/,  4750,  5100/*,  3950,  5, 0, 0*/},  //       75M 9
        {"75M"/*,        SW_BT, SI47XX_AM*/,  3900,  5300/*,  3950,  5, 0, 0*/},        //       75M 9
        {"60M"/*,        SW_BT, SI47XX_USB*/, 5300,  5900/*,  5375,  1, 0, 0*/}, // Ham   60M   10
        {"49M Bcast"/*,  SW_BT, SI47XX_AM*/,  5850,  6350/*,  6000,  5, 0, 0*/}, //       49M 11
        {"49M"/*,        SW_BT, SI47XX_AM*/,  5900,  7000/*,  6000,  5, 0, 0*/},       //       49M 11
        {"41M Bcast"/*,  SW_BT, SI47XX_AM*/,  7200,  7500/*,  7210,  5, 0, 0*/}, //       41M 13
        {"40M Ham"/*,    SW_BT, SI47XX_LSB*/, 7000,  7500/*,  7074,  1, 0, 0*/}, // Ham   40M   12
        {"41M"/*,        SW_BT, SI47XX_AM*/,  7500,  9000/*,  7210,  5, 0, 0*/},      //       41M 13
        {"31M Bcast"/*,  SW_BT, SI47XX_AM*/,  9400,  9990/*,  9600,  5, 0, 0*/}, //       31M 14
        {"31M"/*,        SW_BT, SI47XX_AM*/,  9000,  10000/*, 9600,  5, 0, 0*/}, //       31M   14
        {"30M Ham"/*,    SW_BT, SI47XX_USB*/, 10000, 10200/*, 10099, 1, 0, 0*/}, // Ham   30M   15
        {"25M Bcast"/*,  SW_BT, SI47XX_AM*/,  11600, 12100/*, 11700, 5, 0, 0*/},                                                     //       25M   16
        {"25M"/*,        SW_BT, SI47XX_AM*/,  10200, 13500/*, 11700, 5, 0, 0*/}, //       25M   16
        {"22M  Bcast"/*, SW_BT, SI47XX_AM*/,  13500, 13870/*, 13700, 5, 0,0*/},                                                     //       22M   17
        {"22M"/*,        SW_BT, SI47XX_AM*/,  13500, 14000/*, 13700, 5, 0, 0*/}, //       22M   17
        {"20M Ham"/*,    SW_BT, SI47XX_USB*/, 14000, 14500/*, 14074, 1, 0, 0*/}, // Ham   20M   18
        {"19M Bcast"/*,  SW_BT, SI47XX_AM*/,  15100, 15800/*, 15700, 5, 0, 0*/}, //       19M   19
        {"17M Bcast"/*,  SW_BT, SI47XX_AM*/,  17480, 18050/*, 17600, 5, 0, 0*/},                                                     //       17M   20
        {"19M"/*,        SW_BT, SI47XX_AM*/,  14500, 17500/*, 15700, 5, 0, 0*/}, //       19M   19
        {"17M"/*,        SW_BT, SI47XX_AM*/,  17500, 18000/*, 17600, 5, 0, 0*/}, //       17M   20
        {"16M Ham"/*,    SW_BT, SI47XX_USB*/, 18000, 18500/*, 18100, 1, 0, 0*/}, // Ham   16M   21
        {"15M Bcast"/*,  SW_BT, SI47XX_AM*/,  18900, 19020/*, 18950, 5, 0,0*/},                                                     //       15M   22
        {"15M"/*,        SW_BT, SI47XX_AM*/,  18500, 21000/*, 18950, 5, 0, 0*/}, //       15M   22
        {"14M Ham"/*,    SW_BT, SI47XX_USB*/, 21000, 21450/*, 21074, 1, 0,0*/}, // Ham   14M   23
        {"14M Bcast"/*,  SW_BT, SI47XX_USB*/, 21450, 21850/*, 21074, 1, 0,0*/},                                                     // Ham   14M   23
        {"13M"/*,        SW_BT, SI47XX_AM*/,  21500, 24000/*, 21500, 5, 0, 0*/}, //       13M   24
        {"12M Ham"/*,    SW_BT, SI47XX_USB*/, 24000, 25500/*, 24940, 1, 0,0*/}, // Ham   12M   25
        {"11M Bcast"/*,  SW_BT, SI47XX_AM*/,  25600, 26100/*, 25800, 5, 0,0*/},                                                     //       11M   26
        {"11M"/*,        SW_BT, SI47XX_AM*/,  25500, 26100/*, 25800, 5, 0, 0*/}, //       11M   26
        {"CB"/*,         SW_BT, SI47XX_AM*/,  26100, 28000/*, 27200, 1, 0, 0*/},  // CB band 27
        {"10M Ham"/*,    SW_BT, SI47XX_USB*/, 28000, 29750/*, 28500, 1, 0,0*/},                                                      // Ham   10M   28
        {"10M"/*,        SW_BT, SI47XX_USB*/, 28000, 30000/*, 28500, 1, 0, 0*/}, // Ham   10M   28
        {"SW" /*,         SW_BT, SI47XX_AM*/, 100,   30000/*, 15500, 5, 0, 0*/}      // Whole SW 29
};
static const uint8_t BANDS_COUNT = ARRAY_SIZE(bands);

static int8_t getCurrentBandIndex() {
    for (int8_t i = 0; i < BANDS_COUNT; ++i) {
        if (siCurrentFreq >= bands[i].minimumFreq &&
            siCurrentFreq <= bands[i].maximumFreq) {
            return i;
        }
    }
    return -1;
}

uint32_t LIGHT_TIME_SET[] = {0, 5000, 10000, 20000, 60000, 120000, 240000, UINT32_MAX};
static uint8_t att = 0;
static uint16_t step = 10;

static DateTime dt;
static int16_t bfo = 0;
uint32_t light_time;
bool INPUT_STATE = false;

static void light_open() {
    light_time = LIGHT_TIME_SET[gEeprom.BACKLIGHT_TIME];
    BACKLIGHT_TurnOn();
}
void WaitDisplay()
{
    UI_DisplayClear();
    memset(gStatusLine, 0, sizeof(gStatusLine));
    UI_PrintStringSmall("SI4732 Wait...", 0, 127, 3);
    ST7565_BlitStatusLine();
    ST7565_BlitFullScreen();

}
static void tune(uint32_t f) {
    if (si4732mode == SI47XX_FM) {
        if (f < 6400000 || f > 10800000) {
            return;
        }
    } else {
        if (f < 15000 || f > 3000000) {
            return;
        }
    }
    EEPROM_WriteBuffer(SI4732_FREQ_ADD + si4732mode * 4, (uint8_t *) &f, 4);

    f /= divider;
    if (si4732mode == SI47XX_FM) {
        f -= f % 5;
    }

    SI47XX_ClearRDS();

    SI47XX_SetFreq(f);
    SI47XX_SetAutomaticGainControl(att>0,att);
    currentBandIndex = getCurrentBandIndex();
}

void SI_init() {
    SI_run = true;
    BK4819_ToggleGpioOut(BK4819_GPIO6_PIN2_GREEN, false);
    BK4819_Disable();


    SI47XX_PowerUp();

    SI47XX_SetAutomaticGainControl(att>0, att);
}


static bool seeking = false;
static uint8_t seeking_way = 0;


static void resetBFO() {
    if (bfo != 0) {
        bfo = 0;
        SI47XX_SetBFO(bfo);
    }
}


void SI_deinit() {
    SI47XX_PowerDown();
    BK4819_RX_TurnOn();
    GPIO_SetBit(&GPIOC->DATA, GPIOC_PIN_AUDIO_PATH);
#ifdef ENABLE_DOPPLER
    SYSCON_DEV_CLK_GATE|=(1<<22);
#endif
}

bool display_flag = 0;
KeyboardState kbds = {KEY_INVALID, KEY_INVALID, 0};


void SI4732_Display() {
    memset(gStatusLine, 0, sizeof(gStatusLine));
    if (INPUT_STATE) {
        UI_PrintStringSmall(freqInputString, 2, 127, 1);

    } else {
        uint8_t String[19];

        //频率显示
        uint32_t f = siCurrentFreq * divider;
        uint16_t fp1 = f / 100000;
        uint16_t fp2 = f / 100 % 1000;
        sprintf(String, "%3u.%03u", fp1, fp2);
        UI_DisplayFrequency(String, 64 - strlen(String) * 13 / 2, 2, false);
        //模式显示
        const uint8_t BASE = 38;
        GUI_DisplaySmallest(SI47XX_MODE_NAMES[si4732mode], LCD_WIDTH - 12, BASE - 10 - 8, false, true);


        if (SI47XX_IsSSB()) {
            sprintf(String, "%d", bfo);
            GUI_DisplaySmallest(String, LCD_WIDTH - strlen(String) * 4, BASE - 8, false, true);
        }

        if (si4732mode == SI47XX_FM) {
            if (rds.RDSSignal) {
                GUI_DisplaySmallest("RDS", LCD_WIDTH - 12, 12 - 8, false, true);
            }

            char genre[17];
            const char wd[8][3] = {"SU", "MO", "TU", "WE", "TH", "FR", "SA", "SU"};
            SI47XX_GetProgramType(genre);

            GUI_DisplaySmallest(genre, 64 - strlen(genre) * 2, 15 - 8, false, true);


            if (SI47XX_GetLocalDateTime(&dt)) {
                sprintf(String, "%02u.%02u.%04u, %s %02u:%02u", dt.day, dt.month, dt.year, wd[dt.wday], dt.hour,
                        dt.minute);
                GUI_DisplaySmallest(String, 64 - strlen(String) * 2, 22 - 8, false, true);

            }
            GUI_DisplaySmallest(rds.radioText, 0, LCD_HEIGHT - 8 - 8, false, true);
        }

        if (si4732mode == SI47XX_FM) {
            sprintf(String, "STP %u ATT %u", step, att);
        } else if (SI47XX_IsSSB()) {
            sprintf(String, "STP %u ATT %u BW %s", step, att, SI47XX_SSB_BW_NAMES[ssbBw]);
        } else {
            sprintf(String, "STP %u ATT %u BW %s", step, att, SI47XX_BW_NAMES[bw]);
        }
        GUI_DisplaySmallest(String, 64 - strlen(String) * 2, BASE + 6 - 8, false, true);
        if (si4732mode != SI47XX_FM) {
            if (currentBandIndex >= 0) {
                sprintf(String, "%s %d - %dkHz", bands[currentBandIndex].bandName, bands[currentBandIndex].minimumFreq,
                        bands[currentBandIndex].maximumFreq);
                GUI_DisplaySmallest(String, 64 - strlen(String) * 2, LCD_HEIGHT - 5 - 9, false, true);
            }
        }

        uint8_t rssi = rsqStatus.resp.RSSI;
        if (rssi > 64) {
            rssi = 64;
        }
        for (int i = 0; i < rssi * 2; ++i) {
            gFrameBuffer[0][i] |= 0b00111100;
        }


        sprintf(String, "SNR %u", rsqStatus.resp.SNR);

        GUI_DisplaySmallest(String, 0, 15 - 8, false, true);


    }

    ST7565_BlitFullScreen();
}


static void OnKeyDownFreqInput(uint8_t key) {
    switch (key) {
        case KEY_0:
        case KEY_1:
        case KEY_2:
        case KEY_3:
        case KEY_4:
        case KEY_5:
        case KEY_6:
        case KEY_7:
        case KEY_8:
        case KEY_9:
        case KEY_STAR:
            UpdateFreqInput(key);
            break;
        case KEY_EXIT:
            if (freqInputIndex == 0) {
                INPUT_STATE = false;
                break;
            }
            UpdateFreqInput(key);

            break;
        case KEY_MENU:
            if (!FreqCheck(tempFreq)) {
                break;
            }
            INPUT_STATE = false;
            tune(tempFreq);
            resetBFO();

            break;
        default:
            break;
    }
}


void HandleUserInput() {
    kbds.prev = kbds.current;
    kbds.current = GetKey();
    bool KEY_TYPE1 = false, KEY_TYPE2 = false, KEY_TYPE3 = false;
    // 无按键
    if (kbds.current == KEY_INVALID) {
        if (kbds.counter > 2 && kbds.counter <= 6) {
            // 短按松手
            KEY_TYPE3 = true;
        }
        kbds.counter = 0;
    } else {
        if (kbds.counter >= 6 && kbds.counter % 2 == 1) {
            KEY_TYPE1 = true;
        }
        if (kbds.current == kbds.prev) {
            // 持续按下
            if (kbds.counter < 14) {
                kbds.counter++;
            } else if (kbds.counter == 14) {
                // 长按只触发一次
                KEY_TYPE2 = true;
                kbds.counter++;
            }
        } else {
            // 按键变化，重置计数器
            kbds.counter = 1;
        }
        SYSTEM_DelayMs(20);
    }

    if (KEY_TYPE1 || KEY_TYPE2 || KEY_TYPE3) {
        light_open();
        display_flag = 1;
    }
    SI_key(kbds.current, KEY_TYPE1, KEY_TYPE2, KEY_TYPE3, kbds.prev);

}

void SI_key(KEY_Code_t key, bool KEY_TYPE1, bool KEY_TYPE2, bool KEY_TYPE3, KEY_Code_t key_prev) {
    // up-down keys
    if (INPUT_STATE && KEY_TYPE3) {
        OnKeyDownFreqInput(key_prev);
        return;
    }
    if (KEY_TYPE1 || KEY_TYPE3) {
        if (KEY_TYPE3)key = key_prev;
        switch (key) {
            case KEY_UP:
                tune((siCurrentFreq + step) * divider);
                resetBFO();
                return;
            case KEY_DOWN:
                tune((siCurrentFreq - step) * divider);
                resetBFO();
                return;
#ifdef ENABLE_4732SSB
            case KEY_SIDE1:
                if (SI47XX_IsSSB()) {
                    if (bfo < INT16_MAX - 10) {
                        bfo += 10;
                    }
                    SI47XX_SetBFO(bfo);
                }
                return;

            case KEY_SIDE2:
                if (SI47XX_IsSSB()) {
                    if (bfo > INT16_MIN + 10) {
                        bfo -= 10;
                    }
                    SI47XX_SetBFO(bfo);

                }
                return;
#endif
            case KEY_2:
                if (att < 37) {
                    att++;
                    SI47XX_SetAutomaticGainControl(1, att);
                }
                return;
            case KEY_8:
                if (att > 0) {
                    att--;
                    SI47XX_SetAutomaticGainControl(att > 0, att);
                }
                return;
            default:
                break;
        }
    }

    // Simple keypress
    if (KEY_TYPE3) {

        switch (key_prev) {
            case KEY_1:
                if (step < 1000) {
                    if (step == 1 || step == 10 || step == 100 || step == 1000) {
                        step *= 5;
                    } else {
                        step *= 2;
                    }
                }
                return;
            case KEY_7:
                if (step > 1) {
                    if (step == 1 || step == 10 || step == 100 || step == 1000) {
                        step /= 2;
                    } else {
                        step /= 5;
                    }
                }
                return;

            case KEY_6:
#ifdef ENABLE_4732SSB

if (SI47XX_IsSSB()) {
                    if (ssbBw == SI47XX_SSB_BW_1_0_kHz) {
                        ssbBw = SI47XX_SSB_BW_1_2_kHz;
                    } else {
                        ssbBw++;
                    }
                    SI47XX_SetSsbBandwidth(ssbBw);
                } else {
#endif
                    if (bw == SI47XX_BW_1_kHz) {
                        bw = SI47XX_BW_6_kHz;
                    } else {
                        bw++;
                    }
                    SI47XX_SetBandwidth(bw, true);
#ifdef ENABLE_4732SSB

            }
#endif

                return;

            case KEY_5:
                INPUT_STATE = 1;
                FreqInput();
                return;
            case KEY_0:
                divider = 100;
                WaitDisplay();
                if (si4732mode == SI47XX_FM) {
                    SI47XX_SwitchMode(SI47XX_AM);
                    SI47XX_SetBandwidth(bw, true);
//                    tune(720000);
                    step = 5;
                }
#ifdef ENABLE_4732SSB


                else if (si4732mode == SI47XX_AM) {

                    SI47XX_SwitchMode(SI47XX_LSB);
                    SI47XX_SetSsbBandwidth(ssbBw);
//                    tune(711300);
                    step = 1;
                }
#endif

                else {
                    divider = 1000;
                    SI47XX_SwitchMode(SI47XX_FM);
//                    tune(10000000);
                    step = 10;
                }
                tune(Read_FreqSaved());
                resetBFO();
                return;
#ifdef ENABLE_4732SSB

            case KEY_F:
                if (SI47XX_IsSSB()) {
                    uint32_t tmpF;
                    SI47XX_SwitchMode(si4732mode == SI47XX_LSB ? SI47XX_USB : SI47XX_LSB);
                    tune(Read_FreqSaved()); // to apply SSB
                    return;
                }
                return;
#endif

            case KEY_EXIT:
                if (seeking) {
                    SI47XX_PowerDown();
                    SI47XX_PowerUp();
                    seeking = false;
                    return;
                }
                SI_run = false;
                return;
            case KEY_3:
            case KEY_9:
#ifdef ENABLE_4732SSB

if (SI47XX_IsSSB()) {
                    return;
                }
#endif
                if (si4732mode == SI47XX_FM) {
                    SI47XX_SetSeekFmSpacing(step);
                } else {
                    SI47XX_SetSeekAmSpacing(step);
                }

                SI47XX_Seek(key == KEY_3 ? 1 : 0, 1);
                if (key == KEY_3)seeking_way = 1;
                else seeking_way = 0;


                seeking = true;
                return;


            default:
                break;
        }
    }

}


void SI4732_Main() {
#ifdef ENABLE_DOPPLER
    SYSCON_DEV_CLK_GATE= SYSCON_DEV_CLK_GATE & ( ~(1 << 22));
#endif

    light_open();
    SI_init();

    uint16_t cnt = 500;
    while (SI_run) {
        if (light_time && gEeprom.BACKLIGHT_TIME != 7) {
            light_time--;
            if (light_time == 0)BACKLIGHT_TurnOff();
        }
        if (cnt == 500) {
            if (si4732mode == SI47XX_FM) {
                SI47XX_GetRDS();
            }
            RSQ_GET();
            cnt = 0;
            UI_DisplayClear();
            DrawPower();
            ST7565_BlitStatusLine();
            display_flag = 1;
        }

        if (cnt % 25 == 0) {
            HandleUserInput();


        }

        if (seeking && cnt % 100 == 0) {
            UI_PrintStringSmallBuffer("*", gStatusLine);
            bool valid = false;
            siCurrentFreq = SI47XX_getFrequency(&valid);
            uint32_t f = siCurrentFreq * divider;
            EEPROM_WriteBuffer(SI4732_FREQ_ADD + si4732mode * 4, (uint8_t *) &f, 4);

            if (valid) {
                seeking = false;
                light_open();
                tune((siCurrentFreq) * divider);
            }

            display_flag = 1;
        }
        cnt++;
        if (display_flag) {
            display_flag = 0;
            SI4732_Display();
        }
        SYSTEM_DelayMs(1);
    }
    SI_deinit();

}
