#include "si.h"
#include "../driver/bk4819.h"
#include "../driver/si473x.h"
#include "../helper/rds.h"
#include "../misc.h"
//
// Created by RUPC on 2024/3/10.
//


#include "driver/si473x.h"
#include "bsp/dp32g030/gpio.h"
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

static const char SI47XX_BW_NAMES[5][8] = {
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

typedef struct // Band data
{
  const char *bandName; // Bandname
  BandType bandType;    // Band type (FM, MW or SW)
  SI47XX_MODE prefmod;  // Pref. modulation
  uint16_t minimumFreq; // Minimum frequency of the band
  uint16_t maximumFreq; // maximum frequency of the band
  uint16_t currentFreq; // Default frequency or current frequency
  uint8_t currentStep;  // Default step (increment and decrement)
  int lastBFO;          // Last BFO per band
  int lastmanuBFO;      // Last Manual BFO per band using X-Tal

} SIBand;

SIBand bands[] = {
    {"LW", LW_BT, SI47XX_AM, 148, 283, 198, 9, 0, 0},         //  LW          1
    {"LW", LW_BT, SI47XX_AM, 100, 514, 198, 9, 0, 0},         //  LW          1
    {"MW Bcast", MW_BT, SI47XX_AM, 526, 1606, 1395, 9, 0, 0}, //  MW          2
    {"MW", MW_BT, SI47XX_AM, 514, 1800, 1395, 9, 0, 0},       //  MW          2
    {"BACON Ham", LW_BT, SI47XX_AM, 280, 470, 284, 1, 0, 0},  // Ham  800M 3
    {"630M Ham", SW_BT, SI47XX_LSB, 470, 480, 475, 1, 0, 0},  // Ham  630M 4
    {"160M Ham", SW_BT, SI47XX_LSB, 1800, 2000, 1850, 1, 0, 0},  // Ham  160M 5
    {"120M Bcast", SW_BT, SI47XX_AM, 2300, 2495, 2400, 5, 0, 0}, //      120M 6
    {"120M", SW_BT, SI47XX_AM, 2000, 3200, 2400, 5, 0, 0},       //      120M 6
    {"90M Bcast", SW_BT, SI47XX_AM, 3200, 3400, 3300, 5, 0, 0},  //       90M 7
    {"90M", SW_BT, SI47XX_AM, 3200, 3500, 3300, 5, 0, 0},        //       90M 7
    {"80M Ham", SW_BT, SI47XX_LSB, 3500, 3900, 3630, 1, 0, 0},   // Ham   80M 8
    {"75M Bcast", SW_BT, SI47XX_AM, 3900, 4000, 3950, 5, 0, 0},  //       75M 9
    {"75M Bacst", SW_BT, SI47XX_AM, 4750, 5100, 3950, 5, 0, 0},  //       75M 9
    {"75M", SW_BT, SI47XX_AM, 3900, 5300, 3950, 5, 0, 0},        //       75M 9
    {"60M", SW_BT, SI47XX_USB, 5300, 5900, 5375, 1, 0, 0}, // Ham   60M   10
    {"49M Bcast", SW_BT, SI47XX_AM, 5850, 6350, 6000, 5, 0, 0}, //       49M 11
    {"49M", SW_BT, SI47XX_AM, 5900, 7000, 6000, 5, 0, 0},       //       49M 11
    {"41M Bcast", SW_BT, SI47XX_AM, 7200, 7500, 7210, 5, 0, 0}, //       41M 13
    {"40M Ham", SW_BT, SI47XX_LSB, 7000, 7500, 7074, 1, 0, 0}, // Ham   40M   12
    {"41M", SW_BT, SI47XX_AM, 7500, 9000, 7210, 5, 0, 0},      //       41M 13
    {"31M Bcast", SW_BT, SI47XX_AM, 9400, 9990, 9600, 5, 0, 0}, //       31M 14
    {"31M", SW_BT, SI47XX_AM, 9000, 10000, 9600, 5, 0, 0}, //       31M   14
    {"30M Ham", SW_BT, SI47XX_USB, 10000, 10200, 10099, 1, 0,
     0}, // Ham   30M   15
    {"25M Bcast", SW_BT, SI47XX_AM, 11600, 12100, 11700, 5, 0,
     0},                                                     //       25M   16
    {"25M", SW_BT, SI47XX_AM, 10200, 13500, 11700, 5, 0, 0}, //       25M   16
    {"22M  Bcast", SW_BT, SI47XX_AM, 13500, 13870, 13700, 5, 0,
     0},                                                     //       22M   17
    {"22M", SW_BT, SI47XX_AM, 13500, 14000, 13700, 5, 0, 0}, //       22M   17
    {"20M Ham", SW_BT, SI47XX_USB, 14000, 14500, 14074, 1, 0,
     0}, // Ham   20M   18
    {"19M Bcast", SW_BT, SI47XX_AM, 15100, 15800, 15700, 5, 0,
     0}, //       19M   19
    {"17M Bcast", SW_BT, SI47XX_AM, 17480, 18050, 17600, 5, 0,
     0},                                                     //       17M   20
    {"19M", SW_BT, SI47XX_AM, 14500, 17500, 15700, 5, 0, 0}, //       19M   19
    {"17M", SW_BT, SI47XX_AM, 17500, 18000, 17600, 5, 0, 0}, //       17M   20
    {"16M Ham", SW_BT, SI47XX_USB, 18000, 18500, 18100, 1, 0,
     0}, // Ham   16M   21
    {"15M Bcast", SW_BT, SI47XX_AM, 18900, 19020, 18950, 5, 0,
     0},                                                     //       15M   22
    {"15M", SW_BT, SI47XX_AM, 18500, 21000, 18950, 5, 0, 0}, //       15M   22
    {"14M Ham", SW_BT, SI47XX_USB, 21000, 21450, 21074, 1, 0,
     0}, // Ham   14M   23
    {"14M Bcast", SW_BT, SI47XX_USB, 21450, 21850, 21074, 1, 0,
     0},                                                     // Ham   14M   23
    {"13M", SW_BT, SI47XX_AM, 21500, 24000, 21500, 5, 0, 0}, //       13M   24
    {"12M Ham", SW_BT, SI47XX_USB, 24000, 25500, 24940, 1, 0,
     0}, // Ham   12M   25
    {"11M Bcast", SW_BT, SI47XX_AM, 25600, 26100, 25800, 5, 0,
     0},                                                     //       11M   26
    {"11M", SW_BT, SI47XX_AM, 25500, 26100, 25800, 5, 0, 0}, //       11M   26
    {"CB", SW_BT, SI47XX_AM, 26100, 28000, 27200, 1, 0, 0},  // CB band 27
    {"10M Ham", SW_BT, SI47XX_USB, 28000, 29750, 28500, 1, 0,
     0},                                                      // Ham   10M   28
    {"10M", SW_BT, SI47XX_USB, 28000, 30000, 28500, 1, 0, 0}, // Ham   10M   28
    {"SW", SW_BT, SI47XX_AM, 100, 30000, 15500, 5, 0, 0}      // Whole SW 29
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

static uint8_t att = 0;
static uint16_t divider = 1000;
static uint16_t step = 10;
static uint32_t lastUpdate = 0;
static uint32_t lastRdsUpdate = 0;
static uint32_t lastSeekUpdate = 0;
static DateTime dt;
static int16_t bfo = 0;
static bool showSNR = false;

static void tune(uint32_t f) {
  f /= divider;
  if (si4732mode == SI47XX_FM) {
    f -= f % 5;
  }
  SI47XX_ClearRDS();
  SI47XX_SetFreq(f);
  currentBandIndex = getCurrentBandIndex();
}

void SI_init() {

    BK4819_Idle();
  SI47XX_PowerUp();

  SI47XX_SetAutomaticGainControl(1, att);
}

static bool hasRDS = false;
static bool seeking = false;

//void SI_update() {
//  if (si4732mode == SI47XX_FM && Now() - lastRdsUpdate >= 1000) {
//    hasRDS = SI47XX_GetRDS();
//    lastRdsUpdate = Now();
//    if (hasRDS) {
//      gRedrawScreen = true;
//    }
//  }
//  if (Now() - lastUpdate >= 1000) {
//    if (showSNR) {
//      RSQ_GET();
//    }
//    lastUpdate = Now();
//    gRedrawScreen = true;
//  }
//  if (seeking && Now() - lastSeekUpdate >= 100) {
//    bool valid = false;
//    siCurrentFreq = SI47XX_getFrequency(&valid);
//    if (valid) {
//      seeking = false;
//    }
//    lastSeekUpdate = Now();
//    gRedrawScreen = true;
//  }
//}

static uint32_t lastFreqChange = 0;

static void resetBFO() {
  if (bfo != 0) {
    bfo = 0;
    SI47XX_SetBFO(bfo);
  }
}

//bool SI_key(KEY_Code_t key, bool bKeyPressed, bool bKeyHeld) {
//  // up-down keys
//  if (bKeyPressed || (!bKeyPressed && !bKeyHeld)) {
//    switch (key) {
//    case KEY_UP:
//      if (Now() - lastFreqChange > 250) {
//        lastFreqChange = Now();
//        tune((siCurrentFreq + step) * divider);
//        resetBFO();
//      }
//      return true;
//    case KEY_DOWN:
//      if (Now() - lastFreqChange > 250) {
//        lastFreqChange = Now();
//        tune((siCurrentFreq - step) * divider);
//        resetBFO();
//      }
//      return true;
//    case KEY_SIDE1:
//      if (SI47XX_IsSSB()) {
//        if (bfo < INT16_MAX - 10) {
//          bfo += 10;
//        }
//        SI47XX_SetBFO(bfo);
//      }
//      return true;
//    case KEY_SIDE2:
//      if (SI47XX_IsSSB()) {
//        if (bfo > INT16_MIN + 10) {
//          bfo -= 10;
//        }
//        SI47XX_SetBFO(bfo);
//      }
//      return true;
//    case KEY_2:
//      if (att < 37) {
//        att++;
//        SI47XX_SetAutomaticGainControl(1, att);
//      }
//      return true;
//    case KEY_8:
//      if (att > 0) {
//        att--;
//        SI47XX_SetAutomaticGainControl(att > 0, att);
//      }
//      return true;
//    default:
//      break;
//    }
//  }
//
//  // long held
//  if (bKeyHeld && bKeyPressed && !gRepeatHeld) {
//    switch (key) {
//    case KEY_STAR:
//      if (SI47XX_IsSSB()) {
//        return false;
//      }
//      if (si4732mode == SI47XX_FM) {
//        SI47XX_SetSeekFmSpacing(step);
//      } else {
//        SI47XX_SetSeekAmSpacing(step);
//      }
//      SI47XX_Seek(1, 1);
//      seeking = true;
//      return true;
//    default:
//      break;
//    }
//  }
//
//  // Simple keypress
//  if (!bKeyPressed && !bKeyHeld) {
//    switch (key) {
//    case KEY_1:
//      if (step < 1000) {
//        if (step == 1 || step == 10 || step == 100 || step == 1000) {
//          step *= 5;
//        } else {
//          step *= 2;
//        }
//      }
//      return true;
//    case KEY_7:
//      if (step > 1) {
//        if (step == 1 || step == 10 || step == 100 || step == 1000) {
//          step /= 2;
//        } else {
//          step /= 5;
//        }
//      }
//      return true;
//    case KEY_6:
//      if (SI47XX_IsSSB()) {
//        if (ssbBw == SI47XX_SSB_BW_1_0_kHz) {
//          ssbBw = SI47XX_SSB_BW_1_2_kHz;
//        } else {
//          ssbBw++;
//        }
//        SI47XX_SetSsbBandwidth(ssbBw);
//      } else {
//        if (bw == SI47XX_BW_1_kHz) {
//          bw = SI47XX_BW_6_kHz;
//        } else {
//          bw++;
//        }
//        SI47XX_SetBandwidth(bw, true);
//      }
//      return true;
//    case KEY_4:
//      showSNR = !showSNR;
//      return true;
//    case KEY_5:
//      gFInputCallback = tune;
//      APPS_run(APP_FINPUT);
//      return true;
//    case KEY_0:
//      divider = 100;
//      if (si4732mode == SI47XX_FM) {
//        SI47XX_SwitchMode(SI47XX_AM);
//        SI47XX_SetBandwidth(bw, true);
//        tune(720000);
//        step = 5;
//      } else if (si4732mode == SI47XX_AM) {
//        SI47XX_SwitchMode(SI47XX_LSB);
//        SI47XX_SetSsbBandwidth(ssbBw);
//        tune(711300);
//        step = 1;
//      } else {
//        divider = 1000;
//        SI47XX_SwitchMode(SI47XX_FM);
//        tune(10000000);
//        step = 10;
//      }
//      resetBFO();
//      return true;
//    case KEY_F:
//      if (SI47XX_IsSSB()) {
//        SI47XX_SwitchMode(si4732mode == SI47XX_LSB ? SI47XX_USB : SI47XX_LSB);
//        tune(siCurrentFreq * divider); // to apply SSB
//        return true;
//      }
//      return false;
//    case KEY_STAR:
//      BK4819_Idle();
//      return true;
//    case KEY_EXIT:
//      APPS_exit();
//      return true;
//    case KEY_SIDE1:
//      if (currentBandIndex > 0) {
//        currentBandIndex--;
//        tune(bands[currentBandIndex].currentFreq * divider);
//      }
//      return true;
//    case KEY_SIDE2:
//      if (currentBandIndex < BANDS_COUNT - 1) {
//        currentBandIndex++;
//        tune(bands[currentBandIndex].currentFreq * divider);
//      }
//      return true;
//    default:
//      break;
//    }
//  }
//  return false;
//}
//
//void SI_render() {
//  UI_ClearScreen();
//  const uint8_t BASE = 38;
//
//  uint32_t f = siCurrentFreq * divider;
//  uint16_t fp1 = f / 100000;
//  uint16_t fp2 = f / 100 % 1000;
//
//  // PrintSmallEx(0, 12, POS_L, C_FILL, "SNR: %u dB", rsqStatus.resp.SNR);
//
//  PrintBiggestDigitsEx(LCD_WIDTH - 22, BASE, POS_R, C_FILL, "%3u.%03u", fp1,
//                       fp2);
//  PrintSmallEx(LCD_WIDTH - 1, BASE - 6, POS_R, C_FILL, "%s",
//               SI47XX_MODE_NAMES[si4732mode]);
//  if (SI47XX_IsSSB()) {
//    PrintSmallEx(LCD_WIDTH - 1, BASE, POS_R, C_FILL, "%d", bfo);
//  }
//
//  if (si4732mode == SI47XX_FM) {
//    if (rds.RDSSignal) {
//      PrintSmallEx(LCD_WIDTH - 1, 12, POS_R, C_FILL, "RDS");
//    }
//
//    char genre[17];
//    const char wd[8][3] = {"SU", "MO", "TU", "WE", "TH", "FR", "SA", "SU"};
//    SI47XX_GetProgramType(genre);
//    PrintSmallEx(LCD_XCENTER, 14, POS_C, C_FILL, "%s", genre);
//
//    if (SI47XX_GetLocalDateTime(&dt)) {
//      PrintSmallEx(LCD_XCENTER, 22, POS_C, C_FILL,
//                   "%02u.%02u.%04u, %s %02u:%02u", dt.day, dt.month, dt.year,
//                   wd[dt.wday], dt.hour, dt.minute);
//    }
//
//    PrintSmall(0, LCD_HEIGHT - 8, "%s", rds.radioText);
//  }
//
//  if (si4732mode == SI47XX_FM) {
//    PrintSmallEx(LCD_XCENTER, BASE + 6, POS_C, C_FILL, "STP %u ATT %u", step,
//                 att);
//  } else if (SI47XX_IsSSB()) {
//    PrintSmallEx(LCD_XCENTER, BASE + 6, POS_C, C_FILL, "STP %u ATT %u BW %s",
//                 step, att, SI47XX_SSB_BW_NAMES[ssbBw]);
//  } else {
//    PrintSmallEx(LCD_XCENTER, BASE + 6, POS_C, C_FILL, "STP %u ATT %u BW %s",
//                 step, att, SI47XX_BW_NAMES[bw]);
//  }
//
//  if (si4732mode != SI47XX_FM) {
//    if (currentBandIndex >= 0) {
//      PrintSmallEx(LCD_XCENTER, LCD_HEIGHT - 2, POS_C, C_FILL, "%s %d - %dkHz",
//                   bands[currentBandIndex].bandName,
//                   bands[currentBandIndex].minimumFreq,
//                   bands[currentBandIndex].maximumFreq);
//    }
//  }
//
//  if (showSNR) {
//    uint8_t rssi = rsqStatus.resp.RSSI;
//    if (rssi > 64) {
//      rssi = 64;
//    }
//    FillRect(0, 8, rssi * 2, 2, C_FILL);
//    PrintSmall(0, 15, "SNR %u", rsqStatus.resp.SNR);
//  }
//}
//
void SI_deinit() {
  SI47XX_PowerDown();
  BK4819_RX_TurnOn();
#ifdef ENABLE_DOPPLER
    SYSCON_DEV_CLK_GATE|=(1<<22);
#endif
}
bool display_flag=0;
KeyboardState kbds = {KEY_INVALID, KEY_INVALID, 0};
uint32_t NOW_STEP=100;

void SI4732_Display() {
    memset(gStatusLine, 0, sizeof(gStatusLine));
    UI_DisplayClear();




    uint8_t String[19];



    uint32_t f = siCurrentFreq * divider;
    uint16_t fp1 = f / 100000;
    uint16_t fp2 = f / 100 % 1000;

    // PrintSmallEx(0, 12, POS_L, C_FILL, "SNR: %u dB", rsqStatus.resp.SNR);
    sprintf(String, "%3u.%03u", fp1, fp2);
    UI_DisplayFrequency(String, 25, 1, false);


    sprintf(String, "%02d %02d ", rsqStatus.resp.RSSI, rsqStatus.resp.SNR);
    UI_PrintStringSmall(String, 0, 127, 5);

//    ST7565_BlitStatusLine();  // blank status line
    ST7565_BlitFullScreen();
}

void SI4732_Main()
{
#ifdef ENABLE_DOPPLER

    SYSCON_DEV_CLK_GATE= SYSCON_DEV_CLK_GATE & ( ~(1 << 22));
#endif

    SI_init();
    BACKLIGHT_TurnOn();
    int cnt = 0;
    while (1) {

        if (cnt == 500) {
            RSQ_GET();
            cnt = 0;
        }
        if (cnt % 5 == 0) {
            HandleUserInput();
            display_flag=1;

        }

        cnt++;
        if (display_flag) {
            display_flag = 0;
            SI4732_Display();
        }

        SYSTEM_DelayMs(1);


    }
}


 void HandleUserInput() {
    kbds.prev = kbds.current;
    kbds.current = GetKey();
    if (kbds.current == KEY_INVALID) {
        kbds.counter = 0;
    }

    if (kbds.current != KEY_INVALID && kbds.current == kbds.prev) {
        if (kbds.counter < 16)
            kbds.counter++;
        else
            kbds.counter -= 3;
        SYSTEM_DelayMs(20);
    } else {
        kbds.counter = 0;
    }


    if (kbds.counter == 2 || kbds.counter == 16) {
        switch (si4732mode) {
            case SI47XX_FM:
                Key_FM(kbds);
                break;
            case SI47XX_AM:
                Key_FM(kbds);
                break;

        }

    }

}


 void Key_FM(KeyboardState kbds) {
    switch (kbds.current) {
        case KEY_UP:
//            lastFreqChange = Now();
            tune((siCurrentFreq + step) * divider);
            resetBFO();

            RSQ_GET();
            break;
        case KEY_DOWN:
//            lastFreqChange = Now();
            tune((siCurrentFreq - step) * divider);
            resetBFO();

            RSQ_GET();

            break;

        case KEY_STAR:
            step /= 10;
            if (step == 0)step = 1000;
            break;

        case KEY_F:
          if(  si4732mode ==SI47XX_AM)  si4732mode =SI47XX_FM;
else si4732mode =SI47XX_AM;




            divider = 100;
            if (si4732mode == SI47XX_AM) {
                SI47XX_SwitchMode(SI47XX_AM);
                SI47XX_SetBandwidth(bw, true);
                tune(720000);
                step = 1;
            }  else {
                divider = 1000;
                SI47XX_SwitchMode(SI47XX_FM);
                tune(10210000);
                step = 10;
            }
            resetBFO();

            break;
        case KEY_1:

            break;

        case KEY_7:

            break;

        case KEY_2:

            break;

        case KEY_8:
//            b--;
//            AM_FRONTEND_AGC_CONTROL(a,b);
            break;


        default:
            break;
    }
}
