
#include "app/spectrum.h"
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include "tle.h"
#include "util.h"
#include "eci.h"
#include "driver/eeprom.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "tle/eci.h"
#include "bsp/dp32g030/gpio.h"
#include "driver/gpio.h"
#include "app/si.h"
#include "ARMCM0.h"

#include "driver/i2c.h"
#include "driver/system.h"
#include "frequencies.h"
#include "misc.h"
#include "app/doppler.h"
#include "driver/uart.h"
#include "string.h"
#include <stdio.h>
#include "ui/helper.h"
#include <string.h>
#include "driver/bk4819.h"
#include "font.h"
#include "ui/ui.h"
#include <stdint.h>
#include <string.h>
#include "font.h"
#include <stdio.h>     // NULL
#include "app/mdc1200.h"
#include "app/uart.h"
#include "string.h"
#include "app/messenger.h"
#include "time.h"
#include "app/doppler.h"
#include "bsp/dp32g030/rtc.h"
#include "bsp/dp32g030/uart.h"
#include "bsp/dp32g030/timer.h"
#include "audio.h"
#include "board.h"
#include "misc.h"
#include "radio.h"
#include "settings.h"
#include "version.h"
#include "app/app.h"
#include "app/dtmf.h"
#include "bsp/dp32g030/gpio.h"
#include "bsp/dp32g030/syscon.h"
#include "driver/backlight.h"
#include "driver/bk4819.h"
#include "driver/gpio.h"
#include "driver/system.h"
#include "driver/systick.h"
#include "bsp/dp32g030/pwmplus.h"
#include "driver/uart.h"
#include "app/spectrum.h"
#include "helper/battery.h"
#include "helper/boot.h"
#include "ui/lock.h"
#include "ui/welcome.h"
#include "ui/menu.h"
#include "driver/eeprom.h"
#include "driver/st7565.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "tle/eci.h"
#include  "ui/main.h"
uint8_t freqInputDotIndex = 0;
bool monitorMode = false;

bool TX_ON = false;
uint32_t fMeasure = 0;
uint8_t freqInputIndex = 0;
KEY_Code_t freqInputArr[12];

uint16_t listenT = 0;
const uint16_t RSSI_MAX_VALUE = 65535;
char freqInputString[13];
PeakInfo peak;

 bool isTransmitting = false;
 bool isListening = true;
 uint8_t vfo=0;
bool lockAGC = false;
 uint8_t menuState = 0;
 RegisterSpec registerSpecs[] = {
    {},
    {"LNAs", BK4819_REG_13, 8, 0b11,   1},
    {"LNA",  BK4819_REG_13, 5, 0b111,  1},
    {"PGA",  BK4819_REG_13, 0, 0b111,  1},
    {"IF",   BK4819_REG_3D, 0, 0xFFFF, 0x2aaa},
    // {"MIX", 0x13, 3, 0b11, 1}, // '
};

 ScanInfo scanInfo;
uint16_t rssiHistory[128];

SpectrumSettings settings = {.stepsCount = STEPS_64,
                                    .scanStepIndex = S_STEP_25_0kHz,
                                    .frequencyChangeStep = 80000,
                                    .scanDelay = 3200,
                                    .rssiTriggerLevel = 150,
                                    .backlightState = true,
                                    .bw = BK4819_FILTER_BW_WIDE,
                                    .listenBw = BK4819_FILTER_BW_WIDE,
                                    .modulationType = false,
                                    .dbMin = -130,
                                    .dbMax = -50
                                   };
int my_clamp(int v, int min, int max) {
    return v <= min ? min : (v >= max ? max : v);
}
 uint16_t dbm2rssi(int dBm) {
    return (dBm + 160 - dBmCorrTable[gRxVfo->Band]) * 2;
}
void ClampRssiTriggerLevel() {
    settings.rssiTriggerLevel =
            my_clamp(settings.rssiTriggerLevel, dbm2rssi(settings.dbMin),
                  dbm2rssi(settings.dbMax));
}

 void ResetScanStats() {
    scanInfo.rssi = 0;
    scanInfo.rssiMax = 0;
    scanInfo.iPeak = 0;
    scanInfo.fPeak = 0;
}

 void AutoTriggerLevel() {
    if (settings.rssiTriggerLevel == RSSI_MAX_VALUE) {
        settings.rssiTriggerLevel = my_clamp(scanInfo.rssiMax + 8, 0, RSSI_MAX_VALUE);
    }
}
uint8_t Rssi2Y(uint16_t rssi) {
    return DrawingEndY - Rssi2PX(rssi, 0, DrawingEndY);
}
static void SetRssiHistory(uint16_t idx, uint16_t rssi) {

    rssiHistory[idx] = rssi;
}
 uint16_t GetRssi() {
    // SYSTICK_DelayUs(800);
    // testing autodelay based on Glitch value
    while ((BK4819_ReadRegister(0x63) & 0b11111111) >= 255) {
        SYSTICK_DelayUs(100);
    }
    uint16_t rssi = BK4819_GetRSSI();
#ifdef ENABLE_AM_FIX
    if(settings.modulationType==MODULATION_AM && gSetting_AM_fix)
        rssi += AM_fix_get_gain_diff()*2;
#endif
    return rssi;
}
static void Measure() {
    uint16_t rssi = scanInfo.rssi = GetRssi();
    SetRssiHistory(scanInfo.i, rssi);
}


uint8_t GetBWRegValueForScan() {
    return scanStepBWRegValues[settings.scanStepIndex];
}

void DrawRssiTriggerLevel() {
    if (settings.rssiTriggerLevel == RSSI_MAX_VALUE || monitorMode)
        return;
    uint8_t y = Rssi2Y(settings.rssiTriggerLevel);
    for (uint8_t x = 0; x < 128; x += 2) {
        PutPixel(x, y, true);
    }
}

 void ToggleAFBit(bool on) {
    uint16_t reg = BK4819_ReadRegister(BK4819_REG_47);
    reg &= ~(1 << 8);
    if (on)
        reg |= on << 8;
    BK4819_WriteRegister(BK4819_REG_47, reg);
}

void ToggleAFDAC(bool on) {
    uint32_t Reg = BK4819_ReadRegister(BK4819_REG_30);
    Reg &= ~(1 << 9);
    if (on)
        Reg |= (1 << 9);
    BK4819_WriteRegister(BK4819_REG_30, Reg);
}
 void UpdateRssiTriggerLevel(bool inc) {
    if (inc)
        settings.rssiTriggerLevel += 2;
    else
        settings.rssiTriggerLevel -= 2;

    ClampRssiTriggerLevel();


}
void UpdateStill() {
    if (TX_ON)return;
    Measure();
    // redrawScreen = true;
    // preventKeypress = false;

    peak.rssi = scanInfo.rssi;
    AutoTriggerLevel();

    ToggleRX((IsPeakOverLevel() || monitorMode));
}


uint16_t GetRegMenuValue(uint8_t st) {
    RegisterSpec s = registerSpecs[st];
    return (BK4819_ReadRegister(s.num) >> s.offset) & s.mask;
}

 void UpdateListening() {
    // preventKeypress = false;
        listenT = 0;
    
    if (listenT) {
        listenT--;
        SYSTEM_DelayMs(1);
        return;
    }


    Measure();
    peak.rssi = scanInfo.rssi;

    if (IsPeakOverLevel() || monitorMode) {
        listenT = 1000;
        return;
    }

    ToggleRX(false);
    ResetScanStats();
}

void DeInitSpectrum() {
    // SetF(initialFreq);
    // RestoreRegisters();
    // isInitialized = false;
}


bool IsPeakOverLevel() { 
    return peak.rssi >= settings.rssiTriggerLevel; 
}



void ToggleAudio(bool on) {
//    if (on == audioState) {
//        return;
//    }
//    audioState = on;
    if (on) {
        AUDIO_AudioPathOn();
    } else {
        AUDIO_AudioPathOff();
    }
}






void ResetFreqInput() {
    for (int i = 0; i < 12; ++i) {
        freqInputString[i] = '-';
    }

}



void UpdateFreqInput(KEY_Code_t key) {
    if (key != KEY_EXIT && freqInputIndex >= 12) {
        return;
    }

    if (key == KEY_EXIT) {
        freqInputIndex--;
        if (freqInputDotIndex == freqInputIndex)
            freqInputDotIndex = 0;
    } else {
        freqInputArr[freqInputIndex++] = key;
    }

    ResetFreqInput();

    uint8_t dotIndex =
        freqInputDotIndex == 0 ? freqInputIndex : freqInputDotIndex;

    KEY_Code_t digitKey;
    for (int i = 0; i < 12; ++i) {
        if (i < freqInputIndex) {
            digitKey = freqInputArr[i];
            freqInputString[i] = '0' + digitKey ;
        } else {
            freqInputString[i] = '-';
        }
    }


}





void SetTxF(uint32_t f, bool precise) {
    BK4819_PickRXFilterPathBasedOnFrequency(f);
    BK4819_SetFrequency(f);
    uint16_t reg = BK4819_ReadRegister(BK4819_REG_30);
    if (precise) {
        BK4819_WriteRegister(BK4819_REG_30, 0x0200); // from radtel-rt-890-oefw
    } else {
        BK4819_WriteRegister(BK4819_REG_30, reg & ~BK4819_REG_30_ENABLE_VCO_CALIB);
    }
    BK4819_WriteRegister(BK4819_REG_30, reg);
}


void LockAGC() {
    RADIO_SetupAGC(settings.modulationType == MODULATION_AM, lockAGC);
    lockAGC = true;
}


void ToggleRX(bool on) {
//    if(isTransmitting&&on)return;


    isListening = on;
    if ( on) {
        ToggleTX(false);
    }
    RADIO_SetupAGC(on, lockAGC);
    BK4819_ToggleGpioOut(BK4819_GPIO6_PIN2_GREEN, on);

    ToggleAudio(on);
    ToggleAFDAC(on);
    ToggleAFBit(on);

    if (on) {
        listenT = 1000;
        BK4819_WriteRegister(0x43, listenBWRegValues[settings.listenBw]);
    } else {
        BK4819_WriteRegister(0x43, GetBWRegValueForScan());
    }
}



void SetRegMenuValue(uint8_t st, bool add) {
    uint16_t v = GetRegMenuValue(st);
    RegisterSpec s = registerSpecs[st];

    if (s.num == BK4819_REG_13)
        LockAGC();

    uint16_t reg = BK4819_ReadRegister(s.num);
    if (add && v <= s.mask - s.inc) {
        v += s.inc;
    } else if (!add && v >= 0 + s.inc) {
        v -= s.inc;
    }
    // mask in spec
    reg &= ~(s.mask << s.offset);
    BK4819_WriteRegister(s.num, reg | (v << s.offset));
}


void RelaunchScan() {
    // InitScan();
    // ResetPeak();
    ToggleRX(false);
#ifdef SPECTRUM_AUTOMATIC_SQUELCH
    settings.rssiTriggerLevel = RSSI_MAX_VALUE;
#endif
    scanInfo.rssiMin = RSSI_MAX_VALUE;
}

uint8_t DBm2S(int dbm) {
    uint8_t i = 0;
    dbm *= -1;
    for (i = 0; i < ARRAY_SIZE(U8RssiMap); i++) {
        if (dbm >= U8RssiMap[i]) {
            return i;
        }
    }
    return i;
}
// Draw things
// applied x2 to prevent initial rounding
uint8_t Rssi2PX(uint16_t rssi, uint8_t pxMin, uint8_t pxMax) {
    const int DB_MIN = settings.dbMin << 1;
    const int DB_MAX = settings.dbMax << 1;
    const int DB_RANGE = DB_MAX - DB_MIN;

    const uint8_t PX_RANGE = pxMax - pxMin;

    int dbm = my_clamp(Rssi2DBm(rssi) << 1, DB_MIN, DB_MAX);

//    return ((dbm - DB_MIN) * PX_RANGE + DB_RANGE / 2) / DB_RANGE + pxMin;
    return (dbm - DB_MIN) * PX_RANGE / DB_RANGE + pxMin;
}
int Rssi2DBm(uint16_t rssi) {
    return (rssi / 2) - 160 + dBmCorrTable[gRxVfo->Band];
}

void DrawPower() {
    BOARD_ADC_GetBatteryInfo(&gBatteryVoltages[gBatteryCheckCounter++ % 4],
                             &gBatteryCurrent);

    uint16_t voltage = (gBatteryVoltages[0] + gBatteryVoltages[1] +
                        gBatteryVoltages[2] + gBatteryVoltages[3]) /
                       4 * 760 / gBatteryCalibration[3];

    unsigned perc = BATTERY_VoltsToPercent(voltage);

    // sprintf(String, "%d %d", voltage, perc);
    // GUI_DisplaySmallest(String, 48, 1, true, true);

    gStatusLine[116] = 0b00011100;
    gStatusLine[117] = 0b00111110;
    for (int i = 118; i <= 126; i++) {
        gStatusLine[i] = 0b00100010;
    }

    for (unsigned i = 127; i >= 118; i--) {
        if (127 - i <= (perc + 5) * 9 / 100) {
            gStatusLine[i] = 0b00111110;
        }
    }
}


uint16_t registersVault[128] = {0};

static void RegBackup() {
    for (int i = 0; i < 128; ++i) {
        registersVault[i] = BK4819_ReadRegister(i);

    }
}
static void RegRestore() {
    for (int i = 0; i < 128; ++i) {
        BK4819_WriteRegister(i, registersVault[i]);
    }
}

void TuneToPeak() {
    scanInfo.f = peak.f;
    scanInfo.rssi = peak.rssi;
    scanInfo.i = peak.i;
    SetF(scanInfo.f);
}

void ToggleTX(bool on) {
    if (isTransmitting == on) {
        return;
    }
    isTransmitting = on;
    if (on) {
        ToggleRX(false);
    }

    BK4819_ToggleGpioOut(BK4819_GPIO5_PIN1_RED, on);

    if (on) {
        TX_ON=1;
        fMeasure = sat_get.UP_LINK;

        AUDIO_AudioPathOff();

        SetTxF(fMeasure, true);
        RegBackup();

        BK4819_WriteRegister(BK4819_REG_47, 0x6040);
        BK4819_WriteRegister(BK4819_REG_7E, 0x302E);
        BK4819_WriteRegister(BK4819_REG_50, 0x3B20);
        BK4819_WriteRegister(BK4819_REG_37, 0x1D0F);
        BK4819_WriteRegister(BK4819_REG_52, 0x028F);
        BK4819_WriteRegister(BK4819_REG_30, 0x0000);
        BK4819_WriteRegister(BK4819_REG_30, 0xC1FE);
        BK4819_WriteRegister(BK4819_REG_51, 0x9033);

        //亚音
        if (sat_get.TX_TONE == 0)
            BK4819_ExitSubAu();
        else
            BK4819_SetCTCSSFrequency(sat_get.TX_TONE);//sat_get.RX_TONE);

        //功率
        FREQUENCY_Band_t Band = FREQUENCY_GetBand(fMeasure);
        uint8_t Txp[3];
        EEPROM_ReadBuffer(0x1ED0 + (Band * 16) + (OUTPUT_POWER_HIGH * 3), Txp, 3);
        BK4819_SetupPowerAmplifier(Txp[2], fMeasure);


#if defined(ENABLE_MESSENGER) || defined(ENABLE_MDC1200)
        enable_msg_rx(false);
#endif
        //DTMF
        BK4819_DisableDTMF();
        //加密
        BK4819_DisableScramble();
    } else {
        BK4819_GenTail(4); // CTC55
        BK4819_WriteRegister(BK4819_REG_51, 0x904A);
//        SYSTEM_DelayMs(200);
        BK4819_SetupPowerAmplifier(0, 0);
        RegRestore();
//TODO:发射频率
        fMeasure = sat_get.UP_LINK;
        SetTxF(fMeasure, true);
        TX_ON=0;

    }
    BK4819_ToggleGpioOut(BK4819_GPIO0_PIN28_RX_ENABLE, !on);
    BK4819_ToggleGpioOut(BK4819_GPIO1_PIN29_PA_ENABLE, on);
}

void SetF(uint32_t f) {
    fMeasure = f;

    BK4819_SetFrequency(fMeasure);
    BK4819_PickRXFilterPathBasedOnFrequency(fMeasure);
    uint16_t reg = BK4819_ReadRegister(BK4819_REG_30);
    BK4819_WriteRegister(BK4819_REG_30, 0);
    BK4819_WriteRegister(BK4819_REG_30, reg);


}

bool isLeapYear(uint8_t year) {
    int fullYear = 2000 + year; // 假设年份是2000年之后的
    return (fullYear % 4 == 0 && fullYear % 100 != 0) || (fullYear % 400 == 0);
}

// 获取某个月的天数
uint8_t getDaysInMonth(uint8_t year, uint8_t month) {
    if (month == 2) {
        return isLeapYear(year) ? 29 : 28;
    }
    if (month == 4 || month == 6 || month == 9 || month == 11) {
        return 30;
    }
    return 31;
}

// UTC时间转北京时间
void utcToBeijingTime(uint8_t *datetime) {
    // 将UTC时间加上8小时
    datetime[3] += 8;

    // 处理小时的进位
    if (datetime[3] >= 24) {
        datetime[3] -= 24;
        datetime[2] += 1;

        // 处理日期的进位
        if (datetime[2] > getDaysInMonth(datetime[0], datetime[1])) {
            datetime[2] = 1;
            datetime[1] += 1;

            // 处理月份的进位
            if (datetime[1] > 12) {
                datetime[1] = 1;
                datetime[0] += 1;
            }
        }
    }
}


// 北京时间转UTC时间
void beijingToUtcTime(uint8_t *datetime) {
    // 将北京时间减去8小时
    if (datetime[3] < 8) {
        datetime[3] += 24;
        datetime[3] -= 8;
        datetime[2] -= 1;

        // 处理日期的退位
        if (datetime[2] < 1) {
            datetime[1] -= 1;

            // 处理月份的退位
            if (datetime[1] < 1) {
                datetime[1] = 12;
                datetime[0] -= 1;
            }

            // 设置日期为上个月的最后一天
            datetime[2] = getDaysInMonth(datetime[0], datetime[1]);
        }
    } else {
        datetime[3] -= 8;
    }
}
