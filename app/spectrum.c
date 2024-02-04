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

struct FrequencyBandInfo {
    uint32_t lower;
    uint32_t upper;
    uint32_t middle;
};

#define F_MIN frequencyBandTable[0].lower
#define F_MAX frequencyBandTable[BAND_N_ELEM - 1].upper

const uint16_t RSSI_MAX_VALUE = 65535;
int32_t time_diff, time_diff1;
static uint32_t initialFreq;
static char String[32];
#ifdef ENABLE_DOPPLER
bool DOPPLER_MODE = 0;
#endif
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
        .dbMax = -50//TODO:多普勒频谱参数分离
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
    // TODO: use max value for bits count in max value, or reset by additional
    // mask in spec
    reg &= ~(s.mask << s.offset);
    BK4819_WriteRegister(s.num, reg | (v << s.offset));
    redrawScreen = true;
}


// Utility functions

KEY_Code_t GetKey() {
    KEY_Code_t btn = KEYBOARD_Poll();
    if (btn == KEY_INVALID && !GPIO_CheckBit(&GPIOC->DATA, GPIOC_PIN_PTT)) {
        btn = KEY_PTT;
    }
    return btn;
}

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
        BK4819_REG_30,
        BK4819_REG_37,
        BK4819_REG_3D,
        BK4819_REG_43,
        BK4819_REG_47,
        BK4819_REG_48,
        BK4819_REG_7E,
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

uint8_t GetBWRegValueForScan() {
    return scanStepBWRegValues[settings.scanStepIndex];
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

static void ToggleAudio(bool on) {
    if (on == audioState) {
        return;
    }
    audioState = on;
    if (on) {
        AUDIO_AudioPathOn();
    } else {
        AUDIO_AudioPathOff();
    }
}

static void ToggleRX(bool on) {
    isListening = on;

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

static void ResetFreqInput() {
    tempFreq = 0;
    for (int i = 0; i < 10; ++i) {
        freqInputString[i] = '-';
    }
}

static void FreqInput() {
    freqInputIndex = 0;
    freqInputDotIndex = 0;
    ResetFreqInput();
    SetState(FREQ_INPUT);
}


static void UpdateFreqInput(KEY_Code_t key) {
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
            freqInputString[i] = digitKey <= KEY_9 ? '0' + digitKey - KEY_0 : '.';
        } else {
            freqInputString[i] = '-';
        }
    }

    uint32_t base = 100000; // 1MHz in BK units
    for (int i = dotIndex - 1; i >= 0; --i) {
        tempFreq += (freqInputArr[i] - KEY_0) * base;
        base *= 10;
    }

    base = 10000; // 0.1MHz in BK units
    if (dotIndex < freqInputIndex) {
        for (int i = dotIndex + 1; i < freqInputIndex; ++i) {
            tempFreq += (freqInputArr[i] - KEY_0) * base;
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
uint8_t Rssi2PX(uint16_t rssi, uint8_t pxMin, uint8_t pxMax) {
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

static void DrawPower() {
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

void DrawStatus(bool refresh) {

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

    if (!refresh)return;
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

#ifdef ENABLE_DOPPLER

void OnKeyDownSTAR_SHOW(KEY_Code_t key) {
    switch (key) {

        case KEY_EXIT:
            SetState(STILL);
            break;
        default:
            break;
    }
}

#endif

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
#ifdef ENABLE_DOPPLER
            if (DOPPLER_MODE) {
                SetState(STAR_SHOW);
            } else
#endif


                FreqInput();


            break;
        case KEY_0:
            ToggleModulation();
            break;
        case KEY_6:
            ToggleListeningBW();
            break;
        case KEY_SIDE1:
            monitorMode = !monitorMode;
            break;
        case KEY_SIDE2:
            ToggleBacklight();
            break;
        case KEY_PTT:
            //TODO:发射

            // TODO: start transmit
            /* BK4819_ToggleGpioOut(BK4819_GPIO6_PIN2_GREEN, false);
            BK4819_ToggleGpioOut(BK4819_GPIO5_PIN1_RED, true); */
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

static void RenderFreqInput() {
    UI_PrintStringSmall(freqInputString, 2, 127, 0);
}

static void RenderStatus(bool refresh) {

    memset(gStatusLine, 0, refresh ? sizeof(gStatusLine) : 115);
    DrawStatus(refresh);
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
bool flag = 0;

static void Draw_DOPPLER_Process(uint8_t DATA_LINE) {
    int process = 0;
    if (time_diff >= 0)//还没来卫星
    {
        if (time_diff > 1000)//还早
        {
            strcpy(String, "Long");
        } else//1000s以内
        {
            sprintf(String, "%4d sec", time_diff);
            process = time_diff * 45 / 1000;
        }
    } else { //已经来了
        if (time_diff1 >= 0)//正在过境
        {
            sprintf(String, "%4d sec", satellite.sum_time + time_diff);
            process = (satellite.sum_time + time_diff) * 45 / satellite.sum_time;
        } else {
            strcpy(String, "Passed");
        }
    }
    GUI_DisplaySmallest(String, 90, DATA_LINE + 15, false, true);
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
    GUI_DisplaySmallest(String, 0, flag == 0 ? DATA_LINE + 23 : 2, flag == 0 ? false : true, true);
    flag = 0;
}

float my_sin(float x) {
    float result = 0.0;
    float term = x;
    // 使用泰勒级数展开计算 sin(x)
    for (int n = 1; n <= 10; ++n) {
        result += term;
        term = -term * x * x / ((2 * n) * (2 * n + 1));
    }
    return result;
}

float my_cos(float x) {

    float result = 1.0;
    float term = 1.0;
    for (int i = 1; i <= 10; ++i) {
        term *= (-1) * x * x / ((2 * i) * (2 * i - 1));
        result += term;
    }
    return result;
}
float al=4.5f/3.0f;


void printCircleCoordinates(int x, int y, int R) {
    // 圆心坐标
    int xc = x;
    int yc = y;
    int sum = 0;
    // 每隔5度打印一个点的坐标
    int last_x = 999, last_y = 999;
    for (int angle = 0; angle <= 360; angle += 1) {
        // 将角度转换为弧度
        float radian = angle * 3.1415926 / 180.0;
        // 计算圆上的点的坐标
        int x_coord = xc + (int) (al*(R * my_cos(radian) + 0.5)); // 四舍五入
        int y_coord = yc + (int) ((R * my_sin(radian) + 0.5));
        if (last_x != x_coord || last_y != y_coord) {
            PutPixel(x_coord, y_coord, true);
        }
        last_x=x_coord;
        last_y=y_coord;

    }
}
static void RenderSTARSHOW() {
    memset(gStatusLine, 0, sizeof(gStatusLine));
    flag = 1;
    Draw_DOPPLER_Process(26);
//TODO:绘制星图

    sprintf(String, "AZ: %3d.%02d", satellite_data.AZ_I, satellite_data.AZ_F);
    if ((satellite_data.SIGN & 0x0f) == 0x0a) String[3] = '-';
    GUI_DisplaySmallest(String, 87, 8, false, true);

    sprintf(String, "EI: %3d.%02d", satellite_data.EI_I, satellite_data.EI_F);
    if ((satellite_data.SIGN & 0xf0) == 0xa0) String[3] = '-';
    GUI_DisplaySmallest(String, 87, 16, false, true);

    sprintf(String, "D:%5d.%02d", satellite_data.DIS_I, satellite_data.DIS_F);
    GUI_DisplaySmallest(String, 87, 24, false, true);
       uint8_t circle[231][2] = {
            {83, 27},
            {83, 28},
            {83, 29},
            {83, 30},
            {82, 30},
            {82, 31},
            {82, 32},
            {82, 33},
            {82, 34},
            {81, 34},
            {81, 35},
            {81, 36},
            {80, 36},
            {80, 37},
            {80, 38},
            {79, 38},
            {79, 39},
            {78, 39},
            {78, 40},
            {77, 40},
            {77, 41},
            {76, 42},
            {75, 42},
            {75, 43},
            {74, 44},
            {73, 44},
            {73, 45},
            {72, 45},
            {71, 46},
            {70, 46},
            {70, 47},
            {69, 47},
            {68, 48},
            {67, 48},
            {67, 49},
            {66, 49},
            {65, 49},
            {64, 50},
            {63, 50},
            {62, 51},
            {61, 51},
            {60, 51},
            {59, 51},
            {59, 52},
            {58, 52},
            {57, 52},
            {56, 52},
            {55, 53},
            {54, 53},
            {53, 53},
            {52, 53},
            {51, 53},
            {50, 54},
            {49, 54},
            {48, 54},
            {47, 54},
            {46, 54},
            {45, 54},
            {44, 54},
            {43, 54},
            {42, 54},
            {41, 54},
            {40, 54},
            {39, 54},
            {38, 54},
            {37, 54},
            {36, 54},
            {35, 53},
            {34, 53},
            {33, 53},
            {32, 53},
            {31, 53},
            {30, 53},
            {29, 52},
            {28, 52},
            {27, 52},
            {26, 51},
            {25, 51},
            {24, 51},
            {23, 50},
            {22, 50},
            {21, 50},
            {21, 49},
            {20, 49},
            {19, 49},
            {18, 48},
            {17, 48},
            {17, 47},
            {16, 47},
            {15, 46},
            {14, 46},
            {14, 45},
            {13, 45},
            {12, 44},
            {11, 44},
            {11, 43},
            {10, 43},
            {10, 42},
            {9,  42},
            {9,  41},
            {8,  40},
            {7,  40},
            {7,  39},
            {7,  38},
            {6,  38},
            {6,  37},
            {5,  37},
            {5,  36},
            {5,  35},
            {4,  34},
            {4,  33},
            {3,  32},
            {3,  31},
            {3,  30},
            {3,  29},
            {3,  28},
            {3,  27},
            {3,  26},
            {3,  25},
            {3,  24},
            {3,  23},
            {4,  22},
            {4,  21},
            {5,  20},
            {5,  19},
            {5,  18},
            {6,  18},
            {6,  17},
            {7,  17},
            {7,  16},
            {7,  15},
            {8,  15},
            {8,  14},
            {9,  14},
            {9,  13},
            {10, 13},
            {10, 12},
            {11, 12},
            {11, 11},
            {12, 11},
            {13, 10},
            {14, 10},
            {14, 9},
            {15, 9},
            {16, 8},
            {17, 8},
            {17, 7},
            {18, 7},
            {19, 6},
            {20, 6},
            {21, 6},
            {21, 5},
            {22, 5},
            {23, 5},
            {24, 4},
            {25, 4},
            {26, 4},
            {27, 3},
            {28, 3},
            {29, 3},
            {30, 2},
            {31, 2},
            {32, 2},
            {33, 2},
            {34, 2},
            {35, 2},
            {36, 1},
            {37, 1},
            {38, 1},
            {39, 1},
            {40, 1},
            {41, 1},
            {42, 1},
            {43, 1},
            {44, 1},
            {45, 1},
            {46, 1},
            {47, 1},
            {48, 1},
            {49, 1},
            {50, 1},
            {51, 2},
            {52, 2},
            {53, 2},
            {54, 2},
            {55, 2},
            {56, 3},
            {57, 3},
            {58, 3},
            {59, 3},
            {59, 4},
            {60, 4},
            {61, 4},
            {62, 4},
            {63, 5},
            {64, 5},
            {65, 6},
            {66, 6},
            {67, 6},
            {67, 7},
            {68, 7},
            {69, 8},
            {70, 8},
            {70, 9},
            {71, 9},
            {72, 10},
            {73, 10},
            {73, 11},
            {74, 11},
            {75, 12},
            {75, 13},
            {76, 13},
            {77, 14},
            {78, 15},
            {78, 16},
            {79, 16},
            {79, 17},
            {80, 17},
            {80, 18},
            {80, 19},
            {81, 19},
            {81, 20},
            {81, 21},
            {82, 21},
            {82, 22},
            {82, 23},
            {82, 24},
            {82, 25},
            {83, 25},
            {83, 26},
            {83, 27},
    };
    uint8_t circle1[161][2] = {
            {69, 27},
            {69, 28},
            {69, 29},
            {69, 30},
            {69, 31},
            {68, 31},
            {68, 32},
            {68, 33},
            {67, 33},
            {67, 34},
            {67, 35},
            {66, 35},
            {66, 36},
            {65, 36},
            {65, 37},
            {64, 37},
            {64, 38},
            {63, 38},
            {63, 39},
            {62, 39},
            {62, 40},
            {61, 40},
            {60, 40},
            {60, 41},
            {59, 41},
            {58, 41},
            {58, 42},
            {57, 42},
            {56, 42},
            {56, 43},
            {55, 43},
            {54, 43},
            {53, 43},
            {53, 44},
            {52, 44},
            {51, 44},
            {50, 44},
            {49, 44},
            {48, 45},
            {47, 45},
            {46, 45},
            {45, 45},
            {44, 45},
            {43, 45},
            {42, 45},
            {41, 45},
            {40, 45},
            {39, 45},
            {38, 45},
            {37, 45},
            {37, 44},
            {36, 44},
            {35, 44},
            {34, 44},
            {33, 44},
            {32, 43},
            {31, 43},
            {30, 43},
            {29, 42},
            {28, 42},
            {27, 42},
            {27, 41},
            {26, 41},
            {25, 40},
            {24, 40},
            {24, 39},
            {23, 39},
            {22, 38},
            {21, 38},
            {21, 37},
            {20, 37},
            {20, 36},
            {19, 35},
            {19, 34},
            {18, 34},
            {18, 33},
            {17, 32},
            {17, 31},
            {17, 30},
            {16, 29},
            {16, 28},
            {16, 27},
            {16, 26},
            {17, 25},
            {17, 24},
            {17, 23},
            {18, 22},
            {18, 21},
            {19, 21},
            {19, 20},
            {20, 19},
            {20, 18},
            {21, 18},
            {21, 17},
            {22, 17},
            {23, 16},
            {24, 16},
            {24, 15},
            {25, 15},
            {26, 14},
            {27, 14},
            {27, 13},
            {28, 13},
            {29, 13},
            {30, 12},
            {31, 12},
            {32, 12},
            {33, 11},
            {34, 11},
            {35, 11},
            {36, 11},
            {37, 11},
            {37, 10},
            {38, 10},
            {39, 10},
            {40, 10},
            {41, 10},
            {42, 10},
            {43, 10},
            {44, 10},
            {45, 10},
            {46, 10},
            {47, 10},
            {48, 10},
            {49, 11},
            {50, 11},
            {51, 11},
            {52, 11},
            {53, 11},
            {53, 12},
            {54, 12},
            {55, 12},
            {56, 12},
            {56, 13},
            {57, 13},
            {58, 13},
            {58, 14},
            {59, 14},
            {60, 14},
            {60, 15},
            {61, 15},
            {62, 15},
            {62, 16},
            {63, 16},
            {63, 17},
            {64, 17},
            {64, 18},
            {65, 18},
            {65, 19},
            {66, 19},
            {66, 20},
            {67, 20},
            {67, 21},
            {67, 22},
            {68, 22},
            {68, 23},
            {68, 24},
            {69, 24},
            {69, 25},
            {69, 26},
            {69, 27},

    };
    uint8_t circle2[82][2]
            = {
                    {56, 27},
                    {56, 28},
                    {56, 29},
                    {55, 29},
                    {55, 30},
                    {55, 31},
                    {54, 31},
                    {54, 32},
                    {53, 32},
                    {53, 33},
                    {52, 33},
                    {51, 34},
                    {50, 34},
                    {50, 35},
                    {49, 35},
                    {48, 35},
                    {47, 35},
                    {47, 36},
                    {46, 36},
                    {45, 36},
                    {44, 36},
                    {43, 36},
                    {42, 36},
                    {41, 36},
                    {40, 36},
                    {39, 36},
                    {39, 35},
                    {38, 35},
                    {37, 35},
                    {36, 35},
                    {36, 34},
                    {35, 34},
                    {34, 34},
                    {34, 33},
                    {33, 33},
                    {32, 32},
                    {32, 31},
                    {31, 31},
                    {31, 30},
                    {30, 30},
                    {30, 29},
                    {30, 28},
                    {30, 27},
                    {30, 26},
                    {30, 25},
                    {31, 25},
                    {31, 24},
                    {32, 23},
                    {33, 22},
                    {34, 22},
                    {34, 21},
                    {35, 21},
                    {36, 21},
                    {36, 20},
                    {37, 20},
                    {38, 20},
                    {39, 20},
                    {39, 19},
                    {40, 19},
                    {41, 19},
                    {42, 19},
                    {43, 19},
                    {44, 19},
                    {45, 19},
                    {46, 19},
                    {47, 19},
                    {47, 20},
                    {48, 20},
                    {49, 20},
                    {50, 20},
                    {50, 21},
                    {51, 21},
                    {52, 22},
                    {53, 22},
                    {53, 23},
                    {54, 23},
                    {54, 24},
                    {55, 24},
                    {55, 25},
                    {55, 26},
                    {56, 26},
                    {56, 27},

            };
    for (int i = 0; i < 218; i++) {
        PutPixel(circle[i][0], circle[i][1], true);
    }
    for (int i = 0; i < 161; i++) {
        PutPixel(circle1[i][0], circle1[i][1], true);
    }
    for (int i = 0; i < 82; i++) {
        PutPixel(circle2[i][0], circle2[i][1], true);
    }
    satellite_data.AZ_I=30;
    satellite_data.AZ_F=0;
    satellite_data.EI_I=60;
    satellite_data.EI_F=0;//60520
float L=(90-(satellite_data.EI_I+satellite_data.EI_F*0.01f))/90*27;
float x=28.0f*al+L* my_cos((satellite_data.AZ_I+satellite_data.AZ_F*0.01f)/180*3.1415926);
    float y=27.0f+L* my_sin((satellite_data.AZ_I+satellite_data.AZ_F*0.01f)/180*3.1415926);
    for (int i = -1; i <=1; ++i) {
        for (int j = -1; j <=1 ; ++j) {
            PutPixel((int)x+i, (int )y+j, true);
        }
    }
    printCircleCoordinates(28.0f*al, 27, 27);
    printCircleCoordinates(28.0f*al, 27, 18);//161 231
    printCircleCoordinates(28.0f*al, 27, 9); //82
    if (time_diff <= 0 && time_diff1) {//卫星没来
        satellite_data.AZ_I = 0;
        satellite_data.AZ_F = 0;
        satellite_data.EI_I = 0;
        satellite_data.EI_F = 0;
        satellite_data.DIS_I = 0;
        satellite_data.DIS_F = 0;
        satellite_data.SIGN = 0xaa; //58012
    } else {

    }



    ST7565_BlitStatusLine();
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
    for (int i = 0, idx = 1; idx <= 4; ++i, ++idx) {
//        if (idx == 5) {
//            row += 2;
//            i = 0;
//        }
        offset = PAD_LEFT + i * CELL_WIDTH;
        if (menuState == idx) {
            for (int j = 0; j < CELL_WIDTH; ++j) {
                gFrameBuffer[row][j + offset] = 0xFF;
                gFrameBuffer[row + 1][j + offset] = 0xFF;
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
        sprintf(String, "UPLink:%4d.%5d", satellite_data.UPLink / 100000, satellite_data.UPLink % 100000);
        GUI_DisplaySmallest(String, 0, DATA_LINE + 15, false, true);

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
#ifdef ENABLE_DOPPLER
        case STAR_SHOW:
            RenderSTARSHOW();
            break;
#endif
    }
    ST7565_BlitFullScreen();
}

bool HandleUserInput() {
    kbd.prev = kbd.current;
    kbd.current = GetKey();

    if (kbd.current != KEY_INVALID && kbd.current == kbd.prev) {
        if (kbd.counter < 16)
            kbd.counter++;
        else
            kbd.counter -= 3;
        SYSTEM_DelayMs(20);
    } else {
        kbd.counter = 0;
    }

    if (kbd.counter == 3 || kbd.counter == 16) {
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
#ifdef ENABLE_DOPPLER

            case STAR_SHOW:
                OnKeyDownSTAR_SHOW(kbd.current);
                break;
#endif
        }
    }

    return true;
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

static void UpdateSTARTSHOW() {
    redrawScreen = true;
    preventKeypress = false;
}

static void UpdateStill() {
    Measure();
    redrawScreen = true;
    preventKeypress = false;

    peak.rssi = scanInfo.rssi;
    AutoTriggerLevel();

    ToggleRX(IsPeakOverLevel() || monitorMode);
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
#ifdef ENABLE_DOPPLER
        else if (currentState == STAR_SHOW) {
            UpdateSTARTSHOW();
        }
#endif
    }


    if (
#ifdef ENABLE_DOPPLER
currentState != STAR_SHOW &&
#endif
(redrawStatus || ++statuslineUpdateTimer > 4096)) {
        bool refresh = statuslineUpdateTimer > 4096;
        RenderStatus(refresh);
        redrawStatus = false;
        if (refresh)statuslineUpdateTimer = 0;
    }


    if (redrawScreen) {
        Render();
        redrawScreen = false;
    }
}

void APP_RunSpectrum() {
//#ifdef ENABLE_DOPPLER
//    if (DOPPLER_MODE) {
//        RTC_IF |= (1 << 0);//清除中断标志位
//        RTC_IE |= (1 << 0);//使能秒中断
//    }
//#endif
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
        SetState(STILL);
        TuneToPeak();
        //TODO:设置默认卫星频率
        satellite_data.DownLink = 43850000;
        SetF(satellite_data.DownLink);
        currentFreq = satellite_data.DownLink;
        settings.dbMin = -130;
    }
#endif
    while (isInitialized) {
        Tick();
    }
//#ifdef ENABLE_DOPPLER
//    if (DOPPLER_MODE) {
//        RTC_IE &= 0xfffffffe;//关闭秒中断
//    }
//#endif

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

