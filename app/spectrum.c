/* Copyright 2023 fagci
 * https://github.com/fagci
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */
//#define ENABLE_DOPPLER

#include "functions.h"
#include "stdbool.h"

#ifdef ENABLE_DOPPLER

#include "app/doppler.h"
#include "bsp/dp32g030/rtc.h"

#endif

#include "app/spectrum.h"
#include "am_fix.h"
#include "audio.h"
#include "misc.h"


//#define ENABLE_DOPPLER
#ifdef ENABLE_SCAN_RANGES
#include "chFrScanner.h"
#endif

#include "driver/eeprom.h"

#include "driver/backlight.h"
#include "frequencies.h"
#include "ui/helper.h"
#include "ui/main.h"

static void ToggleRX(bool on);

struct FrequencyBandInfo {
    uint32_t lower;
    uint32_t upper;
    uint32_t middle;
};

int Mid(uint16_t *array, uint8_t n) {
    int32_t sum = 0;
    for (int i = 0; i < n; ++i) {
        sum += array[i];
    }
    return sum / n;
}
//
//static void UpdateBatteryInfo() {
//    for (uint8_t i = 0; i < 4; i++) {
//        BOARD_ADC_GetBatteryInfo(&gBatteryVoltages[i], &gBatteryCurrent);
//    }
//
//    uint16_t voltage = Mid(gBatteryVoltages, ARRAY_SIZE(gBatteryVoltages));
//    gBatteryDisplayLevel = 0;
//
//    for (int i = ARRAY_SIZE(gBatteryCalibration) - 1; i >= 0; --i) {
//        if (gBatteryCalibration[i] < voltage) {
//            gBatteryDisplayLevel = i + 1;
//            break;
//        }
//    }
//}

#define F_MIN frequencyBandTable[0].lower
#define F_MAX frequencyBandTable[BAND_N_ELEM - 1].upper

const uint16_t RSSI_MAX_VALUE = 65535;
int32_t time_diff, time_diff1;
static uint32_t initialFreq;
static char String[32];
#ifdef ENABLE_DOPPLER
bool DOPPLER_MODE = 0;
#endif
bool TX_ON = false;
bool isInitialized = false;
bool isListening = true;
bool monitorMode = false;
bool redrawStatus = true;
bool redrawScreen = false;
bool newScanStart = true;
bool preventKeypress = true;
bool audioState = true;
bool lockAGC = false;

State currentState = SPECTRUM, previousState = SPECTRUM;

PeakInfo peak;
ScanInfo scanInfo;
KeyboardState kbd = {KEY_INVALID, KEY_INVALID, 0};

#ifdef ENABLE_SCAN_RANGES
static uint16_t blacklistFreqs[15];
static uint8_t blacklistFreqsIdx;
#endif

const char *bwOptions[] = {"  25k", "12.5k", "6.25k"};
const uint8_t modulationTypeTuneSteps[] = {100, 50, 10};
const uint8_t modTypeReg47Values[] = {1, 7, 5};

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

uint32_t fMeasure = 0;
uint32_t currentFreq, tempFreq;
uint16_t rssiHistory[128];
int vfo;
uint8_t freqInputIndex = 0;
uint8_t freqInputDotIndex = 0;
KEY_Code_t freqInputArr[10];
char freqInputString[11];

uint8_t menuState = 0;
uint16_t listenT = 0;

RegisterSpec registerSpecs[] = {
        {},
        {"LNAs", BK4819_REG_13, 8, 0b11,   1},
        {"LNA",  BK4819_REG_13, 5, 0b111,  1},
        {"PGA",  BK4819_REG_13, 0, 0b111,  1},
        {"IF",   BK4819_REG_3D, 0, 0xFFFF, 0x2aaa},
        // {"MIX", 0x13, 3, 0b11, 1}, // '
};

uint16_t statuslineUpdateTimer = 0;
VfoState_t txAllowState;
bool isTransmitting = false;

static uint8_t DBm2S(int dbm) {
    uint8_t i = 0;
    dbm *= -1;
    for (i = 0; i < ARRAY_SIZE(U8RssiMap); i++) {
        if (dbm >= U8RssiMap[i]) {
            return i;
        }
    }
    return i;
}

uint16_t registersVault[128] = {0};
#ifdef ENABLE_DOPPLER

static void RegBackup() {
    for (int i = 0; i < 128; ++i) {
        registersVault[i] = BK4819_ReadRegister(i);

    }
}
#endif
#ifdef ENABLE_DOPPLER
static void RegRestore() {
    for (int i = 0; i < 128; ++i) {
        BK4819_WriteRegister(i, registersVault[i]);
    }
}
#endif
static void ToggleAudio(bool on) {
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

#ifdef ENABLE_DOPPLER
static void ToggleTX(bool on) {
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
        fMeasure = satellite_data.UPLink;

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
        if (satellite.SEND_CTCSS == 0)
            BK4819_ExitSubAu();
        else
            BK4819_SetCTCSSFrequency(satellite.SEND_CTCSS);

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
        fMeasure = satellite_data.DownLink;
        SetTxF(fMeasure, true);
                TX_ON=0;

    }
    BK4819_ToggleGpioOut(BK4819_GPIO0_PIN28_RX_ENABLE, !on);
    BK4819_ToggleGpioOut(BK4819_GPIO1_PIN29_PA_ENABLE, on);
}
#endif

static int Rssi2DBm(uint16_t rssi) {
    return (rssi / 2) - 160 + dBmCorrTable[gRxVfo->Band];
}

static uint16_t GetRegMenuValue(uint8_t st) {
    RegisterSpec s = registerSpecs[st];
    return (BK4819_ReadRegister(s.num) >> s.offset) & s.mask;
}

void LockAGC() {
    RADIO_SetupAGC(settings.modulationType == MODULATION_AM, lockAGC);
    lockAGC = true;
}

static void SetRegMenuValue(uint8_t st, bool add) {
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
    redrawScreen = true;
}


// Utility functions


static int clamp(int v, int min, int max) {
    return v <= min ? min : (v >= max ? max : v);
}

static uint8_t my_abs(signed v) { return v > 0 ? v : -v; }

void SetState(State state) {
    previousState = currentState;
    currentState = state;
    redrawScreen = true;
    redrawStatus = true;
}

// Radio functions

static void ToggleAFBit(bool on) {
    uint16_t reg = BK4819_ReadRegister(BK4819_REG_47);
    reg &= ~(1 << 8);
    if (on)
        reg |= on << 8;
    BK4819_WriteRegister(BK4819_REG_47, reg);
}

static const BK4819_REGISTER_t registers_to_save[] = {
//        BK4819_REG_30,
//        BK4819_REG_37,
//        BK4819_REG_3D,
//        BK4819_REG_43,
//        BK4819_REG_47,
//        BK4819_REG_48,
//        BK4819_REG_7E,

        0x13, 0x30, 0x31, 0x37, 0x3D, 0x40, 0x43, 0x47, 0x48, 0x7D, 0x7E,

};

static uint16_t registers_stack[sizeof(registers_to_save)];

static void BackupRegisters() {
    for (uint32_t i = 0; i < ARRAY_SIZE(registers_to_save); i++) {
        registers_stack[i] = BK4819_ReadRegister(registers_to_save[i]);
    }
}

static void RestoreRegisters() {

    for (uint32_t i = 0; i < ARRAY_SIZE(registers_to_save); i++) {
        BK4819_WriteRegister(registers_to_save[i], registers_stack[i]);
    }
}

static void ToggleAFDAC(bool on) {
    uint32_t Reg = BK4819_ReadRegister(BK4819_REG_30);
    Reg &= ~(1 << 9);
    if (on)
        Reg |= (1 << 9);
    BK4819_WriteRegister(BK4819_REG_30, Reg);
}

static void SetF(uint32_t f) {
    fMeasure = f;

    BK4819_SetFrequency(fMeasure);
    BK4819_PickRXFilterPathBasedOnFrequency(fMeasure);
    uint16_t reg = BK4819_ReadRegister(BK4819_REG_30);
    BK4819_WriteRegister(BK4819_REG_30, 0);
    BK4819_WriteRegister(BK4819_REG_30, reg);


}

// Spectrum related

bool IsPeakOverLevel() { return peak.rssi >= settings.rssiTriggerLevel; }

static void ResetPeak() {
    peak.t = 0;
    peak.rssi = 0;
}

bool IsCenterMode() { return settings.scanStepIndex < S_STEP_2_5kHz; }

// scan step in 0.01khz
uint16_t GetScanStep() { return scanStepValues[settings.scanStepIndex]; }

uint16_t GetStepsCount() {
#ifdef ENABLE_SCAN_RANGES
    if(gScanRangeStart) {
        return (gScanRangeStop - gScanRangeStart) / GetScanStep();
    }
#endif
    return 128 >> settings.stepsCount;
}

uint32_t GetBW() { return GetStepsCount() * GetScanStep(); }

uint32_t GetFStart() {
    return IsCenterMode() ? currentFreq - (GetBW() >> 1) : currentFreq;
}

uint32_t GetFEnd() { return currentFreq + GetBW(); }

static void TuneToPeak() {
    scanInfo.f = peak.f;
    scanInfo.rssi = peak.rssi;
    scanInfo.i = peak.i;
    SetF(scanInfo.f);
}

static void DeInitSpectrum() {
    SetF(initialFreq);
    RestoreRegisters();
    isInitialized = false;
}

static uint8_t GetBWRegValueForScan() {
    return scanStepBWRegValues[settings.scanStepIndex];
}

static uint16_t GetRssi() {
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

static void ToggleRX(bool on) {
//    if(isTransmitting&&on)return;


    isListening = on;
#ifdef ENABLE_DOPPLER
    if (DOPPLER_MODE && on) {
        ToggleTX(false);
    }

#endif
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

// Scan info

static void ResetScanStats() {
    scanInfo.rssi = 0;
    scanInfo.rssiMax = 0;
    scanInfo.iPeak = 0;
    scanInfo.fPeak = 0;
}

static void InitScan() {
    ResetScanStats();
    scanInfo.i = 0;
    scanInfo.f = GetFStart();

    scanInfo.scanStep = GetScanStep();
    scanInfo.measurementsCount = GetStepsCount();
}

static void ResetBlacklist() {
    for (int i = 0; i < 128; ++i) {
        if (rssiHistory[i] == RSSI_MAX_VALUE)
            rssiHistory[i] = 0;
    }
#ifdef ENABLE_SCAN_RANGES
    memset(blacklistFreqs, 0, sizeof(blacklistFreqs));
    blacklistFreqsIdx = 0;
#endif
}

static void RelaunchScan() {
    InitScan();
    ResetPeak();
    ToggleRX(false);
#ifdef SPECTRUM_AUTOMATIC_SQUELCH
    settings.rssiTriggerLevel = RSSI_MAX_VALUE;
#endif
    preventKeypress = true;
    scanInfo.rssiMin = RSSI_MAX_VALUE;
}

static void UpdateScanInfo() {
    if (scanInfo.rssi > scanInfo.rssiMax) {
        scanInfo.rssiMax = scanInfo.rssi;
        scanInfo.fPeak = scanInfo.f;
        scanInfo.iPeak = scanInfo.i;
    }

    if (scanInfo.rssi < scanInfo.rssiMin) {
        scanInfo.rssiMin = scanInfo.rssi;
        settings.dbMin = Rssi2DBm(scanInfo.rssiMin);
        redrawStatus = true;
    }
}

static void AutoTriggerLevel() {
    if (settings.rssiTriggerLevel == RSSI_MAX_VALUE) {
        settings.rssiTriggerLevel = clamp(scanInfo.rssiMax + 8, 0, RSSI_MAX_VALUE);
    }
}

static void UpdatePeakInfoForce() {
    peak.t = 0;
    peak.rssi = scanInfo.rssiMax;
    peak.f = scanInfo.fPeak;
    peak.i = scanInfo.iPeak;
    AutoTriggerLevel();
}

static void UpdatePeakInfo() {
    if (peak.f == 0 || peak.t >= 1024 || peak.rssi < scanInfo.rssiMax)
        UpdatePeakInfoForce();
}

static void SetRssiHistory(uint16_t idx, uint16_t rssi) {
#ifdef ENABLE_SCAN_RANGES
    if(scanInfo.measurementsCount > 128) {
        uint8_t i = (uint32_t)ARRAY_SIZE(rssiHistory) * 1000 / scanInfo.measurementsCount * idx / 1000;
        if(rssiHistory[i] < rssi || isListening)
            rssiHistory[i] = rssi;
        rssiHistory[(i+1)%128] = 0;
        return;
    }
#endif
    rssiHistory[idx] = rssi;
}

static void Measure() {
    uint16_t rssi = scanInfo.rssi = GetRssi();
    SetRssiHistory(scanInfo.i, rssi);
}

// Update things by keypress

static uint16_t dbm2rssi(int dBm) {
    return (dBm + 160 - dBmCorrTable[gRxVfo->Band]) * 2;
}

static void ClampRssiTriggerLevel() {
    settings.rssiTriggerLevel =
            clamp(settings.rssiTriggerLevel, dbm2rssi(settings.dbMin),
                  dbm2rssi(settings.dbMax));
}

static void UpdateRssiTriggerLevel(bool inc) {
    if (inc)
        settings.rssiTriggerLevel += 2;
    else
        settings.rssiTriggerLevel -= 2;

    ClampRssiTriggerLevel();

    redrawScreen = true;
    redrawStatus = true;
}

static void UpdateDBMax(bool inc) {
    uint8_t tmp = 12;
#ifdef ENBALE_DOPPLER
    if(DOPPLER_MODE) tmp=10;
#endif

    if (inc && settings.dbMax < 10) {
        settings.dbMax += 1;
    } else if (!inc && settings.dbMax > tmp + settings.dbMin) {
        settings.dbMax -= 1;
    } else {
        return;
    }

    ClampRssiTriggerLevel();
    redrawStatus = true;
    redrawScreen = true;
    SYSTEM_DelayMs(20);
}

static void UpdateScanStep(bool inc) {
    if (inc) {
        settings.scanStepIndex = settings.scanStepIndex != S_STEP_100_0kHz ? settings.scanStepIndex + 1 : 0;
    } else {
        settings.scanStepIndex = settings.scanStepIndex != 0 ? settings.scanStepIndex - 1 : S_STEP_100_0kHz;
    }

    settings.frequencyChangeStep = GetBW() >> 1;
    RelaunchScan();
    ResetBlacklist();
    redrawScreen = true;
}

static void UpdateCurrentFreq(bool inc) {
    if (inc && currentFreq < F_MAX) {
        currentFreq += settings.frequencyChangeStep;
    } else if (!inc && currentFreq > F_MIN) {
        currentFreq -= settings.frequencyChangeStep;
    } else {
        return;
    }
    RelaunchScan();
    ResetBlacklist();
    redrawScreen = true;
}

static void UpdateCurrentFreqStill(bool inc) {
    uint8_t offset = modulationTypeTuneSteps[settings.modulationType];
    uint32_t f = fMeasure;
    if (inc && f < F_MAX) {
        f += offset;
    } else if (!inc && f > F_MIN) {
        f -= offset;
    }
    SetF(f);
    redrawScreen = true;
}

static void UpdateFreqChangeStep(bool inc) {
    uint16_t diff = GetScanStep() * 4;
    if (inc && settings.frequencyChangeStep < 200000) {
        settings.frequencyChangeStep += diff;
    } else if (!inc && settings.frequencyChangeStep > 10000) {
        settings.frequencyChangeStep -= diff;
    }
    SYSTEM_DelayMs(100);
    redrawScreen = true;
}

static void ToggleModulation() {
    if (settings.modulationType < MODULATION_UKNOWN - 1) {
        settings.modulationType++;
    } else {
        settings.modulationType = MODULATION_FM;
    }
    RADIO_SetModulation(settings.modulationType);

    RelaunchScan();
    redrawScreen = true;
#ifdef ENABLE_DOPPLER
    if (DOPPLER_MODE) redrawStatus = true;
#endif
}

static void ToggleListeningBW() {
    if (settings.listenBw == BK4819_FILTER_BW_NARROWER) {
        settings.listenBw = BK4819_FILTER_BW_WIDE;
    } else {
        settings.listenBw++;
    }
    redrawScreen = true;
#ifdef ENABLE_DOPPLER
    if (DOPPLER_MODE) redrawStatus = true;
#endif
}

static void ToggleBacklight() {
    settings.backlightState = !settings.backlightState;
    if (settings.backlightState) {
        BACKLIGHT_TurnOn();
    } else {
        BACKLIGHT_TurnOff();
    }
}

static void ToggleStepsCount() {
    if (settings.stepsCount == STEPS_128) {
        settings.stepsCount = STEPS_16;
    } else {
        settings.stepsCount--;
    }
    settings.frequencyChangeStep = GetBW() >> 1;
    RelaunchScan();
    ResetBlacklist();
    redrawScreen = true;
}

 void ResetFreqInput() {
    tempFreq = 0;
    for (int i = 0; i < 10; ++i) {
        freqInputString[i] = '-';
    }
}

 void FreqInput() {
    freqInputIndex = 0;
    freqInputDotIndex = 0;
    ResetFreqInput();
    SetState(FREQ_INPUT);
}


 void UpdateFreqInput(KEY_Code_t key) {
    if (key != KEY_EXIT && freqInputIndex >= 10) {
        return;
    }
    if (key == KEY_STAR) {
        if (freqInputIndex == 0 || freqInputDotIndex) {
            return;
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

    uint8_t dotIndex =
            freqInputDotIndex == 0 ? freqInputIndex : freqInputDotIndex;

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
//#ifdef ENABLE_DOPPLER
//    if(DOPPLER_MODE)base=1;
//#endif
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
    redrawScreen = true;
}

static void Blacklist() {
#ifdef ENABLE_SCAN_RANGES
    blacklistFreqs[blacklistFreqsIdx++ % ARRAY_SIZE(blacklistFreqs)] = peak.i;
#endif

    SetRssiHistory(peak.i, RSSI_MAX_VALUE);
    ResetPeak();
    ToggleRX(false);
    ResetScanStats();
}

#ifdef ENABLE_SCAN_RANGES
static bool IsBlacklisted(uint16_t idx)
{
    for(uint8_t i = 0; i < ARRAY_SIZE(blacklistFreqs); i++)
        if(blacklistFreqs[i] == idx)
            return true;
    return false;
}
#endif

// Draw things
// applied x2 to prevent initial rounding
static uint8_t Rssi2PX(uint16_t rssi, uint8_t pxMin, uint8_t pxMax) {
    const int DB_MIN = settings.dbMin << 1;
    const int DB_MAX = settings.dbMax << 1;
    const int DB_RANGE = DB_MAX - DB_MIN;

    const uint8_t PX_RANGE = pxMax - pxMin;

    int dbm = clamp(Rssi2DBm(rssi) << 1, DB_MIN, DB_MAX);

//    return ((dbm - DB_MIN) * PX_RANGE + DB_RANGE / 2) / DB_RANGE + pxMin;
    return (dbm - DB_MIN) * PX_RANGE / DB_RANGE + pxMin;
}

uint8_t Rssi2Y(uint16_t rssi) {
    return DrawingEndY - Rssi2PX(rssi, 0, DrawingEndY);
}

static void DrawSpectrum() {
    for (uint8_t x = 0; x < 128; ++x) {
        uint16_t rssi = rssiHistory[x >> settings.stepsCount];
        if (rssi != RSSI_MAX_VALUE) {
            DrawVLine(Rssi2Y(rssi), DrawingEndY, x, true);
        }
    }
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

static void DrawStatus() {

#ifdef SPECTRUM_EXTRA_VALUES
    sprintf(String, "%d/%d P:%d T:%d", settings.dbMin, settings.dbMax,
          Rssi2DBm(peak.rssi), Rssi2DBm(settings.rssiTriggerLevel));
#else
    sprintf(String, "%d/%d", settings.dbMin, settings.dbMax);
#endif
#ifdef ENABLE_DOPPLER

    if (DOPPLER_MODE) {
        //UI绘制状态栏
        memset(gStatusLine, 0x7f, 39);
        GUI_DisplaySmallest(satellite.name, 2, 1, true, false);
        GUI_DisplaySmallest(String, 42 + (settings.dbMax > -100 ? 4 : 0), 1, true, true);

        sprintf(String, "%3s", gModulationStr[settings.modulationType]);
        GUI_DisplaySmallest(String, 42 + 38, 1, true, true);

        sprintf(String, "%s", bwOptions[settings.listenBw]);
        GUI_DisplaySmallest(String, 42 + 53 - (settings.listenBw == 0 ? 8 : 0), 1, true, true);
    } else {
#endif
    GUI_DisplaySmallest(String, 0, 1, true, true);
#ifdef ENABLE_DOPPLER
    }
#endif

    DrawPower();

}

static void DrawF(uint32_t f) {
#ifdef ENABLE_DOPPLER
    if (DOPPLER_MODE) {
        //UI绘制
        sprintf(String, "%03u.%05u", f / 100000, f % 100000);

        UI_DisplayFrequency(String, 8, 0, false);

    } else {
#endif
    sprintf(String, "%u.%05u", f / 100000, f % 100000);
    UI_PrintStringSmall(String, 8, 127, 0);


    sprintf(String, "%3s", gModulationStr[settings.modulationType]);
    GUI_DisplaySmallest(String, 116, 1, false, true);
    sprintf(String, "%s", bwOptions[settings.listenBw]);
    GUI_DisplaySmallest(String, 108, 7, false, true);
#ifdef ENABLE_DOPPLER
    }
#endif

}

static void DrawNums() {

    if (currentState == SPECTRUM) {
        sprintf(String, "%ux", GetStepsCount());
        GUI_DisplaySmallest(String, 0, 1, false, true);
        sprintf(String, "%u.%02uk", GetScanStep() / 100, GetScanStep() % 100);
        GUI_DisplaySmallest(String, 0, 7, false, true);
    }

    if (IsCenterMode()) {
        sprintf(String, "%u.%05u \x7F%u.%02uk", currentFreq / 100000,
                currentFreq % 100000, settings.frequencyChangeStep / 100,
                settings.frequencyChangeStep % 100);
        GUI_DisplaySmallest(String, 36, 49, false, true);
    } else {
        sprintf(String, "%u.%05u", GetFStart() / 100000, GetFStart() % 100000);
        GUI_DisplaySmallest(String, 0, 49, false, true);

        sprintf(String, "\x7F%u.%02uk", settings.frequencyChangeStep / 100,
                settings.frequencyChangeStep % 100);
        GUI_DisplaySmallest(String, 48, 49, false, true);

        sprintf(String, "%u.%05u", GetFEnd() / 100000, GetFEnd() % 100000);
        GUI_DisplaySmallest(String, 93, 49, false, true);
    }
}

static void DrawRssiTriggerLevel() {
    if (settings.rssiTriggerLevel == RSSI_MAX_VALUE || monitorMode)
        return;
    uint8_t y = Rssi2Y(settings.rssiTriggerLevel);
    for (uint8_t x = 0; x < 128; x += 2) {
        PutPixel(x, y, true);
    }
}

static void DrawTicks() {
    uint32_t f = GetFStart();
    uint32_t span = GetFEnd() - GetFStart();
    uint32_t step = span / 128;
    for (uint8_t i = 0; i < 128; i += (1 << settings.stepsCount)) {
        f = GetFStart() + span * i / 128;
        uint8_t barValue = 0b00000001;
        (f % 10000) < step && (barValue |= 0b00000010);
        (f % 50000) < step && (barValue |= 0b00000100);
        (f % 100000) < step && (barValue |= 0b00011000);

        gFrameBuffer[5][i] |= barValue;
    }

    // center
    if (IsCenterMode()) {
        memset(gFrameBuffer[5] + 62, 0x80, 5);
        gFrameBuffer[5][64] = 0xff;
    } else {
        memset(gFrameBuffer[5] + 1, 0x80, 3);
        memset(gFrameBuffer[5] + 124, 0x80, 3);

        gFrameBuffer[5][0] = 0xff;
        gFrameBuffer[5][127] = 0xff;
    }
}

static void DrawArrow(uint8_t x) {
    for (signed i = -2; i <= 2; ++i) {
        signed v = x + i;
        if (!(v & 128)) {
            gFrameBuffer[5][v] |= (0b01111000 << my_abs(i)) & 0b01111000;
        }
    }
}

static void OnKeyDown(uint8_t key) {
    switch (key) {
        case KEY_3:
            UpdateDBMax(true);
            break;
        case KEY_9:
            UpdateDBMax(false);
            break;
        case KEY_1:
            UpdateScanStep(true);
            break;
        case KEY_7:
            UpdateScanStep(false);
            break;
        case KEY_2:
            UpdateFreqChangeStep(true);
            break;
        case KEY_8:
            UpdateFreqChangeStep(false);
            break;
        case KEY_UP:
#ifdef ENABLE_SCAN_RANGES
            if(!gScanRangeStart)
#endif
                UpdateCurrentFreq(true);
            break;
        case KEY_DOWN:
#ifdef ENABLE_SCAN_RANGES
            if(!gScanRangeStart)
#endif
                UpdateCurrentFreq(false);
            break;
        case KEY_SIDE1:
            Blacklist();
            break;
        case KEY_STAR:
            UpdateRssiTriggerLevel(true);
            break;
        case KEY_F:
            UpdateRssiTriggerLevel(false);
            break;
        case KEY_5:
#ifdef ENABLE_SCAN_RANGES
            if(!gScanRangeStart)

#endif
                FreqInput();


            break;
        case KEY_0:
            ToggleModulation();
            break;
        case KEY_6:
            ToggleListeningBW();
            break;
        case KEY_4:
#ifdef ENABLE_SCAN_RANGES
            if(!gScanRangeStart)
#endif
                ToggleStepsCount();
            break;
        case KEY_SIDE2:
            ToggleBacklight();
            break;
        case KEY_PTT:
            SetState(STILL);
            TuneToPeak();
            break;
        case KEY_MENU:
            break;
        case KEY_EXIT:
            if (menuState) {
                menuState = 0;
                break;
            }
            DeInitSpectrum();
            break;
        default:
            break;
    }
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
                SetState(previousState);
                break;
            }
            UpdateFreqInput(key);
            break;
        case KEY_MENU:
#ifdef ENABLE_DOPPLER
            if(DOPPLER_MODE)
           {


        time[3]=tempFreq/100000;
        time[4]=(tempFreq/1000)%100;
        time[5]=(tempFreq/10)%100;
                        RTC_Set();
                     SetState(previousState);

                break;
               }
#endif
            if (tempFreq < F_MIN || tempFreq > F_MAX) {
                break;
            }
            SetState(previousState);
            currentFreq = tempFreq;
            if (currentState == SPECTRUM) {
                ResetBlacklist();
                RelaunchScan();
            } else {
                SetF(currentFreq);
            }
            break;
        default:
            break;
    }
}

void OnKeyDownStill(KEY_Code_t key) {
    switch (key) {
        case KEY_3:
            UpdateDBMax(true);
            break;
        case KEY_9:
            UpdateDBMax(false);
            break;
        case KEY_UP:
            if (menuState) {
                SetRegMenuValue(menuState, true);
                break;
            }
#ifdef ENABLE_DOPPLER
            if (!DOPPLER_MODE)
#endif
            UpdateCurrentFreqStill(true);
            break;
        case KEY_DOWN:

            if (menuState) {
                SetRegMenuValue(menuState, false);
                break;
            }
#ifdef ENABLE_DOPPLER
            if (!DOPPLER_MODE)
#endif
            UpdateCurrentFreqStill(false);

            break;
        case KEY_STAR:
            UpdateRssiTriggerLevel(true);
            break;
        case KEY_F:
            UpdateRssiTriggerLevel(false);
            break;
        case KEY_5:
//#ifdef ENABLE_DOPPLER
//            if (DOPPLER_MODE) {
//
//
//            } else
//#endif


            FreqInput();


            break;
        case KEY_0:
#ifdef ENABLE_DOPPLER
            if (!DOPPLER_MODE)
#endif
            ToggleModulation();
            break;
        case KEY_6:
#ifdef ENABLE_DOPPLER
            if (!DOPPLER_MODE)
#endif
            ToggleListeningBW();
            break;
        case KEY_SIDE1:
            monitorMode = !monitorMode;
            break;
        case KEY_SIDE2:
            ToggleBacklight();
            break;
        case KEY_PTT:
#ifdef ENABLE_DOPPLER
            if (DOPPLER_MODE) {
                ToggleTX(true);
                redrawScreen = true;
            }
#endif
            break;
        case KEY_MENU:
            if (menuState == ARRAY_SIZE(registerSpecs) - 1) {
                menuState = 1;
            } else {
                menuState++;
            }
            redrawScreen = true;
            break;
        case KEY_EXIT:
            if (!menuState) {
                SetState(SPECTRUM);
                lockAGC = false;
                monitorMode = false;
                RelaunchScan();

#ifdef ENABLE_DOPPLER
                if (DOPPLER_MODE)DeInitSpectrum();
#endif


                break;
            }
            menuState = 0;
            break;

        default:


            break;
    }
}

 void RenderFreqInput() {
    UI_PrintStringSmall(freqInputString, 2, 127, 0);
//    show_uint32(tempFreq,3);
}

static void UpdateStill() {
    if (TX_ON)return;
    Measure();
    redrawScreen = true;
    preventKeypress = false;

    peak.rssi = scanInfo.rssi;
    AutoTriggerLevel();

    ToggleRX((IsPeakOverLevel() || monitorMode));
}

static void RenderStatus() {

    memset(gStatusLine, 0, sizeof(gStatusLine));
    DrawStatus();
    ST7565_BlitStatusLine();
}

static void RenderSpectrum() {
    DrawTicks();
    DrawArrow(128u * peak.i / GetStepsCount());
    DrawSpectrum();
    DrawRssiTriggerLevel();
    DrawF(peak.f);
    DrawNums();
}

#ifdef ENABLE_DOPPLER

static void Draw_DOPPLER_Process(uint8_t DATA_LINE) {
    int process = 0;
    if (time_diff > 0)//还没来卫星
    {
        if (time_diff > 1000)//还早
        {
            strcpy(String, "Long");

        } else//1000s以内
        {
            sprintf(String, "-%4d sec", time_diff);
            process = time_diff * 45 / 1000;
        }
    } else { //已经来了
        if (time_diff1 >= 0)//正在过境
        {
            sprintf(String, "+%4d sec", satellite.sum_time + time_diff);
            process = 45 - (satellite.sum_time + time_diff) * 45 / satellite.sum_time;
        } else {

            strcpy(String, "Passed");
        }
    }
    GUI_DisplaySmallest(String, 85, DATA_LINE + 15, false, true);
    memset(&gFrameBuffer[6][80], 0b01000000, 45);
    gFrameBuffer[6][79] = 0b00111110;
    gFrameBuffer[6][45 + 80] = 0b00111110;
    for (int i = 0; i < 45; i++) {
        if (i < process)
            gFrameBuffer[6][i + 80] = 0b00111110;
        else
            gFrameBuffer[6][i + 80] = 0b00100010;
    }
    sprintf(String, "20%02d-%02d-%02d %02d:%02d:%02d", time[0], time[1], time[2], time[3], time[4], time[5]);
    GUI_DisplaySmallest(String, 1, DATA_LINE + 23, false, true);
}

#endif

static void RenderStill() {
    DrawF(fMeasure);//绘制频率
    uint8_t METER_PAD_LEFT = 3;
    uint8_t P_WIDTH = 120;
    uint8_t S_LINE = 25;
    uint8_t S_X = 4;
    uint8_t DBM_X = 22;
#ifdef ENABLE_DOPPLER
    if (DOPPLER_MODE) {
        P_WIDTH = 50;
        METER_PAD_LEFT = 70;
        S_LINE = 18;
        S_X = 58;
        DBM_X = 6;
    }
#endif
    memset(&gFrameBuffer[2][METER_PAD_LEFT], 0b01000000, P_WIDTH);

    for (int i = 0; i <= P_WIDTH; i += 5) { //小刻度
        gFrameBuffer[2][i + METER_PAD_LEFT] = 0b01100000;

    }
    uint8_t x = Rssi2PX(scanInfo.rssi, 0, P_WIDTH);//信号强度
    for (int i = 0; i < x; i++) {
        if (i % 5) {
            gFrameBuffer[2][i + METER_PAD_LEFT] |= 0b00001110;
        }
    }

//S表参数绘制
    int dbm = Rssi2DBm(scanInfo.rssi);
    uint8_t s = DBm2S(dbm);
    bool fill = true;
#ifdef ENABLE_DOPPLER
    if ((monitorMode || IsPeakOverLevel()) && DOPPLER_MODE) {
        memset(gFrameBuffer[2] + DBM_X - 2, 0b11111110, 51);
        fill = false;
    }
#endif
    sprintf(String, "S%u", s);
    GUI_DisplaySmallest(String, S_X, S_LINE, false, true);
    sprintf(String, "%4d/%4ddBm", dbm, Rssi2DBm(settings.rssiTriggerLevel));
    GUI_DisplaySmallest(String, DBM_X, S_LINE, false, fill);

    if (!monitorMode) {
        uint8_t x = Rssi2PX(settings.rssiTriggerLevel, 0, P_WIDTH);
        gFrameBuffer[2][METER_PAD_LEFT + x] = 0b11111111;
    }
    //增益参数
    const uint8_t PAD_LEFT = 4;
    const uint8_t CELL_WIDTH = 30;
    uint8_t offset = PAD_LEFT;
    uint8_t row = 4;
    uint8_t DATA_LINE;
    uint8_t SHOW_LINE=4;
#ifdef ENABLE_DOPPLER
    if (DOPPLER_MODE)SHOW_LINE = 3;
#endif
    for (int i = 0, idx = 1; idx <= 4; ++i, ++idx) {
//        if (idx == 5) {
//            row += 2;
//            i = 0;
//        }
        offset = PAD_LEFT + i * CELL_WIDTH;
        if (menuState == idx) {
            for (int j = 0; j < CELL_WIDTH; ++j) {
                gFrameBuffer[SHOW_LINE][j + offset] = 0xFF;
                gFrameBuffer[SHOW_LINE + 1][j + offset] = 0xFF;
            }
        }
        DATA_LINE = row * 8 + 2;
#ifdef ENABLE_DOPPLER
        if (DOPPLER_MODE)DATA_LINE -= 8;
#endif
        sprintf(String, "%s", registerSpecs[idx].name);
        GUI_DisplaySmallest(String, offset + 2, DATA_LINE, false,
                            menuState != idx);
        sprintf(String, "%u", GetRegMenuValue(idx));
        GUI_DisplaySmallest(String, offset + 2, DATA_LINE + 7, false,
                            menuState != idx);
    }
#ifdef ENABLE_DOPPLER

    if (DOPPLER_MODE) {
        Draw_DOPPLER_Process(26);
        bool flag = true;
        if (!isTransmitting)
            sprintf(String, "UPLink:%4d.%05d", satellite_data.UPLink / 100000, satellite_data.UPLink % 100000);
        else {
            memset(gFrameBuffer[5], 0x7f, 77);
            flag = false;
            sprintf(String, "DownLink:%4d.%05d", satellite_data.DownLink / 100000, satellite_data.DownLink % 100000);
        }
        GUI_DisplaySmallest(String, 1, DATA_LINE + 15, false, flag);

    }
#endif
}

static void Render() {
    UI_DisplayClear();
    switch (currentState) {
        case SPECTRUM:
            RenderSpectrum();
            break;
        case FREQ_INPUT:
            RenderFreqInput();
            break;
        case STILL:
            RenderStill();
            break;

    }
    ST7565_BlitFullScreen();
}

static void HandleUserInput() {
    kbd.prev = kbd.current;
    kbd.current = GetKey();
    if (kbd.current == KEY_INVALID) {
        kbd.counter = 0;
#ifdef ENABLE_DOPPLER
        if (DOPPLER_MODE && isTransmitting) {
            ToggleTX(false);
        }
#endif
//        return true;
    }

    if (kbd.current != KEY_INVALID && kbd.current == kbd.prev) {
        if (kbd.counter < 16)
            kbd.counter++;
        else
            kbd.counter -= 3;
        SYSTEM_DelayMs(20);
    } else {
        kbd.counter = 0;
    }


    if (kbd.counter == 2 || kbd.counter == 16) {
        switch (currentState) {
            case SPECTRUM:
                OnKeyDown(kbd.current);
                break;
            case FREQ_INPUT:
                OnKeyDownFreqInput(kbd.current);
                break;
            case STILL:
                OnKeyDownStill(kbd.current);
                break;

        }
    }

}

static void Scan() {
    if (rssiHistory[scanInfo.i] != RSSI_MAX_VALUE
        #ifdef ENABLE_SCAN_RANGES
        && !IsBlacklisted(scanInfo.i)
#endif
            ) {
        SetF(scanInfo.f);
        Measure();
        UpdateScanInfo();
    }
}

static void NextScanStep() {
    ++peak.t;
    ++scanInfo.i;
    scanInfo.f += scanInfo.scanStep;
}

static void UpdateScan() {
    Scan();

    if (scanInfo.i < scanInfo.measurementsCount) {
        NextScanStep();
        return;
    }

    if (scanInfo.measurementsCount < 128)
        memset(&rssiHistory[scanInfo.measurementsCount], 0,
               sizeof(rssiHistory) - scanInfo.measurementsCount * sizeof(rssiHistory[0]));

    redrawScreen = true;
    preventKeypress = false;

    UpdatePeakInfo();
    if (IsPeakOverLevel()) {
        ToggleRX(true);
        TuneToPeak();
        return;
    }

    newScanStart = true;
}


static void UpdateListening() {
    preventKeypress = false;
    if (currentState == STILL) {
        listenT = 0;
    }
    if (listenT) {
        listenT--;
        SYSTEM_DelayMs(1);
        return;
    }

    if (currentState == SPECTRUM) {
        BK4819_WriteRegister(0x43, GetBWRegValueForScan());
        Measure();
        BK4819_WriteRegister(0x43, listenBWRegValues[settings.listenBw]);
    } else {
        Measure();
    }

    peak.rssi = scanInfo.rssi;
    redrawScreen = true;

    if (IsPeakOverLevel() || monitorMode) {
        listenT = 1000;
        return;
    }

    ToggleRX(false);
    ResetScanStats();
}

static void Tick() {
#ifdef ENABLE_AM_FIX
    if (gNextTimeslice) {
        gNextTimeslice = false;
        if(settings.modulationType == MODULATION_AM && !lockAGC) {
            AM_fix_10ms(vfo); //allow AM_Fix to apply its AGC action
        }
    }
#endif

#ifdef ENABLE_SCAN_RANGES
    if (gNextTimeslice_500ms) {
        gNextTimeslice_500ms = false;

        // if a lot of steps then it takes long time
        // we don't want to wait for whole scan
        // listening has it's own timer
        if(GetStepsCount()>128 && !isListening) {
            UpdatePeakInfo();
            if (IsPeakOverLevel()) {
                ToggleRX(true);
                TuneToPeak();
                return;
            }
            redrawScreen = true;
            preventKeypress = false;
        }
    }
#endif

    if (!preventKeypress) {
        HandleUserInput();
    }
    if (newScanStart) {
        InitScan();
        newScanStart = false;
    }
    if (isListening && currentState != FREQ_INPUT) {
        UpdateListening();
    } else {
        if (currentState == SPECTRUM) {
            UpdateScan();
        } else if (currentState == STILL) {
            UpdateStill();
        }


    }


    if (redrawStatus || ++statuslineUpdateTimer > 4096) {
        RenderStatus();
        redrawStatus = false;
        statuslineUpdateTimer = 0;
    }


    if (redrawScreen) {
        Render();
        redrawScreen = false;
    }
}


 void APP_RunSpectrum() {

    // TX here coz it always? set to active VFO
    vfo = gEeprom.TX_VFO;
    // set the current frequency in the middle of the display
#ifdef ENABLE_SCAN_RANGES
    if(gScanRangeStart) {
        currentFreq = initialFreq = gScanRangeStart;
        for(uint8_t i = 0; i < ARRAY_SIZE(scanStepValues); i++) {
            if(scanStepValues[i] >= gTxVfo->StepFrequency) {
                settings.scanStepIndex = i;
                break;
            }
        }
        settings.stepsCount = STEPS_128;
    }
    else
#endif
    {
        currentFreq = initialFreq = gTxVfo->pRX->Frequency -
                                    ((GetStepsCount() / 2) * GetScanStep());
    }
    BackupRegisters();

    isListening = true; // to turn off RX later
    redrawStatus = true;
    redrawScreen = true;
    newScanStart = true;


    ToggleRX(true), ToggleRX(false); // hack to prevent noise when squelch off
    RADIO_SetModulation(settings.modulationType = gTxVfo->Modulation);

    BK4819_SetFilterBandwidth(settings.listenBw = BK4819_FILTER_BW_WIDE, false);

    RelaunchScan();

    memset(rssiHistory, 0, sizeof(rssiHistory));
    isInitialized = true;
#ifdef ENABLE_DOPPLER
    statuslineUpdateTimer = 4097;

    if (DOPPLER_MODE) {
        settings.listenBw = 0;
        settings.modulationType = MODULATION_FM;
        SetState(STILL);
        TuneToPeak();


        settings.dbMin = -130;
    }
#endif
    while (isInitialized) {
//#ifdef ENABLE_DOPPLER
//
//        if (DOPPLER_MODE) {
//            satellite_data.DownLink=43850000;
//            SetF(satellite_data.DownLink);
//            currentFreq = satellite_data.DownLink;
//        }
//#endif
#ifdef ENABLE_DOPPLER
        if (DOPPLER_MODE&&!isTransmitting&&currentFreq!=satellite_data.DownLink) {
            SetF(satellite_data.DownLink);
            currentFreq = satellite_data.DownLink;
        }

#endif
        Tick();

    }

}

#ifdef ENABLE_DOPPLER

void RTCHandler(void) {


    RTC_Get();
    int32_t NOW_UNIX_TIME = UNIX_TIME(time);
    time_diff = satellite.START_TIME_UNIX - NOW_UNIX_TIME; //卫星开始时间-现在时间
    time_diff1 = satellite.sum_time + time_diff;//结束-开始+开始-现在

    READ_DATA(time_diff, time_diff1);


    RTC_IF |= (1 << 5);//清除中断标志位

}

#endif
