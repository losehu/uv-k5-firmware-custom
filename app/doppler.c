//
// Created by RUPC on 2024/1/30.
//
#include "app/mdc1200.h"
#include <string.h>
#include "driver/keyboard.h"
#include "driver/st7565.h"
#include "driver/bk4819.h"
#include "external/printf/printf.h"
#include "misc.h"
#include "settings.h"
#include "radio.h"
#include "app.h"
#include "audio.h"
#include "functions.h"
#include "frequencies.h"
#include "driver/system.h"
#include "ui/ui.h"
#include "ui/helper.h"

#include "driver/uart.h"
#include "stdbool.h"
#include "app/doppler.h"
#include "driver/eeprom.h"
#include "math.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "bsp/dp32g030/rtc.h"
static bool hasRecalculated;

uint8_t hasSatelliteKeps = 0;//卫星数量
const int SATELLITE_PREDICTION_INITIAL_TIME_STEP = 256;
uiDataGlobal_t uiDataGlobal;
satelliteData_t satelliteDataNative[NUM_SATELLITES];// Store native format Keps for each satellite
//计算给定日期距离某一起始日期（428天前）的天数。
uint32_t satelliteDayFn(int year, int month, int day) {
    if (month <= 2) {
        year -= 1;
        month += 12;
    }

    return (uint32_t) (year * SATELLITE_YM) + (int) ((month + 1) * 30.6) + (day - 428);
}

//读取eeprom中的TLE
uint8_t codeplugGetOpenGD77CustomData(uint8_t *dataBuf) {
    uint8_t tmp[2];
    EEPROM_ReadBuffer(0X1E1EE, tmp, 1);
    tmp[0]=1;
    char 		TLE_Name[16]="123456789ABCDEF";
    char 		TLE_Line1[70]="1 24278U 96046B   24032.55357472  .00000003  00000+0  38153-4 0  9993";
    char 		TLE_Line2[70]="2 24278  98.5652 345.2199 0350120 182.0395 177.9337 13.53171224355955";


    EEPROM_Buffer_MORE(0X1E200,TLE_Name,16,1);
    EEPROM_Buffer_MORE(0X1E200+16,TLE_Line1,70,1);
    EEPROM_Buffer_MORE(0X1E200+16+70,TLE_Line2,70,1);

    if (tmp > NUM_SATELLITES)return 0;
    for (int i = 0; i < tmp[0]; i++) {
        EEPROM_Buffer_MORE(0X1E200 + i * sizeof(codeplugSatelliteData_t), dataBuf, sizeof(codeplugSatelliteData_t), 0);
    }
    return tmp[0];
}

//TLE转物理量
static void satelliteSetElementsTLE2Native(float YE_in, float TE_in, float IN_in, float RA_in, float EC_in, float WP_in,
                                           float MA_in,
                                           float MM_in, float M2_in, float RV_in, float ALON_in,
                                           satelliteKeps_t *kepDataOut) {
    kepDataOut->RA = deg2rad(RA_in);
    kepDataOut->EC = EC_in;
    kepDataOut->WP = deg2rad(WP_in);
    kepDataOut->MA = deg2rad(MA_in);
    kepDataOut->MM = MM_in * 2.0 * M_PI;
    kepDataOut->N0 = kepDataOut->MM / 86400.0;            // Mean motion rads/s
    kepDataOut->A0 = pow(currentSatelliteData_GM / kepDataOut->N0 / kepDataOut->N0, 1.0 / 3.0);    // Semi major axis km

    kepDataOut->M2 = M2_in * 2.0 * M_PI;
    kepDataOut->RV = RV_in;// ------------------------------- POSSIBLY NOT USED

    int TE_IntPart = (int) TE_in;
    kepDataOut->TE_FloatPart = TE_in - TE_IntPart;
    kepDataOut->DE = satelliteDayFn(YE_in, 1, 0) + TE_IntPart;

    float IN = deg2rad(IN_in);
    kepDataOut->SI = sin(IN);
    kepDataOut->CI = cos(IN);
    kepDataOut->b0 = kepDataOut->A0 * sqrt(1.0 - kepDataOut->EC * kepDataOut->EC);        // Semi minor axis km

    float PC = satData_RE * kepDataOut->A0 / (kepDataOut->b0 * kepDataOut->b0);
    PC = 1.5 * currentSatelliteData_J2 * PC * PC * kepDataOut->MM;        // Precession const, rad/day
    kepDataOut->QD = -PC * kepDataOut->CI;                // Node Precession rate, rad/day
    kepDataOut->WD = PC * (5.0 * kepDataOut->CI * kepDataOut->CI - 1.0) / 2.0;    // Perigee Precession rate, rad/day
    kepDataOut->DC = -2.0 * kepDataOut->M2 / kepDataOut->MM / 3.0;        // Drag coeff

    // Bring Sun data to satellite epoch
    float TEG = (kepDataOut->DE - satelliteDayFn(currentSatelliteData_YG, 1, 0)) +
                kepDataOut->TE_FloatPart;    // Elapsed Time: Epoch - YG
    kepDataOut->GHAE = deg2rad(currentSatelliteData_G0) + TEG * currentSatelliteData_WE;        // GHA Aries, epoch
}

float satelliteGetElement(const char *gstr, int gstart, int gstop) //转数字
{
    int k, glength;
    char gestr[40];

    glength = gstop - gstart + 1;
    for (k = 0; k <= glength; k++) {
        gestr[k] = gstr[gstart + k - 1];
    }

    gestr[glength] = 0;

    return atof(gestr);
}

bool satelliteTLE2Native(const char *kep0, const char *kep1, const char *kep2, satelliteData_t *kepDataOut)//TLE参数分析
{
    if (!kep0 || !*kep0) {
        kep0 = "NoName";
    }
    if (!kep1 || strlen(kep1) < 69 || kep1[0] != '1') {
        return false;
    }
    if (!kep2 || strlen(kep2) < 69 || kep2[0] != '2') {
        return false;
    }
    strncpy(kepDataOut->name, kep0, 16);
    kepDataOut->name[16] = 0;

    satelliteSetElementsTLE2Native(
            satelliteGetElement(kep1, 19, 20) + 2000,        // Year
            satelliteGetElement(kep1, 21, 32),        // TE: Elapsed time (Epoch - YG)
            satelliteGetElement(kep2, 9, 16),        // IN: Inclination (deg)
            satelliteGetElement(kep2, 18, 25),        // RA: R.A.A.N (deg)
            satelliteGetElement(kep2, 27, 33) * 1.0e-7,    // EC: Eccentricity
            satelliteGetElement(kep2, 35, 42),        // WP: Arg perifee (deg)
            satelliteGetElement(kep2, 44, 51),        // MA: Mean motion (rev/d)
            satelliteGetElement(kep2, 53, 63),        // MM: Mean motion (rev/d)
            satelliteGetElement(kep1, 34, 43),        // M2: Decay rate (rev/d/d)
            (satelliteGetElement(kep2, 64, 68) + ONEPPM),    // RV: Orbit number
            0,                    // ALON: Sat attitude (deg)
            &kepDataOut->keps);


    return true;
}

//加载卫星数据
void loadKeps(void) {
    codeplugSatelliteData_t codeplugKepsData[NUM_SATELLITES];
    hasSatelliteKeps = codeplugGetOpenGD77CustomData((uint8_t *) &codeplugKepsData);
    for (uint8_t numSatellitesLoaded = 0; numSatellitesLoaded < hasSatelliteKeps; numSatellitesLoaded++) {
        if (codeplugKepsData[numSatellitesLoaded].TLE_Name[0] != 0) {
            if (!satelliteTLE2Native(
                    codeplugKepsData[numSatellitesLoaded].TLE_Name,
                    codeplugKepsData[numSatellitesLoaded].TLE_Line1,
                    codeplugKepsData[numSatellitesLoaded].TLE_Line2, &satelliteDataNative[numSatellitesLoaded])) {
                hasSatelliteKeps -= 1;
                numSatellitesLoaded -= 1;
                continue;
            }
            satelliteDataNative[numSatellitesLoaded].rxFreq = codeplugKepsData[numSatellitesLoaded].rxFreq;
            satelliteDataNative[numSatellitesLoaded].txFreq = codeplugKepsData[numSatellitesLoaded].txFreq;
            satelliteDataNative[numSatellitesLoaded].txCTCSS = codeplugKepsData[numSatellitesLoaded].txCTCSS;
            satelliteDataNative[numSatellitesLoaded].armCTCSS = codeplugKepsData[numSatellitesLoaded].armCTCSS;
            memset(&satelliteDataNative[numSatellitesLoaded].predictions, 0x00, sizeof(satellitePredictions_t));
        }
    }
}


#if defined(USING_EXTERNAL_DEBUGGER)
#include "SeggerRTT/RTT/SEGGER_RTT.h"
#endif

static int satelliteGetDoppler(float dopplerFactor, uint32_t freq);


satelliteObserver_t observerData;

satelliteData_t *currentActiveSatellite;
static const int MAX_TOTAL_ITERATIONS = 1000;


float satelliteAtnFn(float y, float x) {
    float a;

    if (x != 0.0) {
        a = atan(y / x);
    } else {
        a = M_PI / 2.0 * sin(y);
    }


    if (x < 0.0) {
        a = a + M_PI;
    }

    if (a < 0.0) {
        a = a + 2.0 * M_PI;
    }
    return a;
}

static int satelliteGetDoppler(float dopplerFactor, uint32_t freq) {
    int digit;
    float tally = 0.0;
    float inBetween;
    long bare;
    long factor = dopplerFactor * 1E11;

    freq = (freq + 50000L) / 100000L;
    for (int x = 4; x > -1; x--) {
        digit = freq / pow(10, x);
        bare = digit * pow(10, x);
        freq = freq - bare;
        inBetween = (factor * (float) bare) / 1E6;
        tally += inBetween;
    }
    return (int) (tally + 0.5);
}

void satelliteSetObserverLocation(float lat, float lon, int height) {
    observerData.LatInRadians = deg2rad(lat);
    observerData.LonInRadians = deg2rad(lon);
    observerData.HeightInKilometers = ((float) height) / 1000.0; // this needs to be in km

    float ObserverCosLat = cos(observerData.LatInRadians);
    float ObserverSinLat = sin(observerData.LatInRadians);
    float ObserverCosLon = cos(observerData.LonInRadians);
    float ObserverSineLon = sin(observerData.LonInRadians);

    float D = sqrt(currentSatelliteData_XX * ObserverCosLat * ObserverCosLat +
                   currentSatelliteData_ZZ * ObserverSinLat * ObserverSinLat);
    float observerRx = currentSatelliteData_XX / D + observerData.HeightInKilometers;
    float observerRz = currentSatelliteData_ZZ / D + observerData.HeightInKilometers;

    // Observer's unit vectors Up EAST and NORTH in geocentric coordinates
    observerData.Ux = ObserverCosLat * ObserverCosLon;
    observerData.Ex = -ObserverSineLon;
    observerData.Nx = -ObserverSinLat * ObserverCosLon;

    observerData.Uy = ObserverCosLat * ObserverSineLon;
    observerData.Ey = ObserverCosLon;
    observerData.Ny = -ObserverSinLat * ObserverSineLon;

    observerData.Uz = ObserverSinLat;
    //currentSatelliteData.observerEz = 0;
    observerData.Nz = ObserverCosLat;

    // Observer's XYZ coordinates at earth's surface
    observerData.Ox = observerRx * observerData.Ux;
    observerData.Oy = observerRx * observerData.Uy;
    observerData.Oz = observerRz * observerData.Uz;

    // Observer's velocity, geocentric coordinates
    observerData.VOx = -observerData.Oy * currentSatelliteData_W0;
    observerData.VOy = observerData.Ox * currentSatelliteData_W0;
}

/* Move epoch from 01.01.1970 to 01.03.0000 (yes, Year 0) - this is the first
 * day of a 400-year long "era", right after additional day of leap year.
 * This adjustment is required only for date calculation, so instead of
 * modifying time_t value (which would require 64-bit operations to work
 * correctly) it's enough to adjust the calculated number of days since epoch.
 */
#define EPOCH_ADJUSTMENT_DAYS    719468L
/* year to which the adjustment was made */
#define ADJUSTED_EPOCH_YEAR    0
/* 1st March of year 0 is Wednesday */
#define ADJUSTED_EPOCH_WDAY    3
/* there are 97 leap years in 400-year periods. ((400 - 97) * 365 + 97 * 366) */
#define DAYS_PER_ERA        146097L
/* there are 24 leap years in 100-year periods. ((100 - 24) * 365 + 24 * 366) */
#define DAYS_PER_CENTURY    36524L
/* there is one leap year every 4 years */
#define DAYS_PER_4_YEARS    (3 * 365 + 366)
/* number of days in a non-leap year */
#define DAYS_PER_YEAR        365
/* number of days in January */
#define DAYS_IN_JANUARY        31
/* number of days in non-leap February */
#define DAYS_IN_FEBRUARY    28
/* number of years per era */
#define YEARS_PER_ERA        400

#define SECSPERMIN    60L
#define MINSPERHOUR    60L
#define HOURSPERDAY    24L
#define SECSPERHOUR    (SECSPERMIN * MINSPERHOUR)
#define SECSPERDAY    (SECSPERHOUR * HOURSPERDAY)
#define DAYSPERWEEK    7
#define MONSPERYEAR    12
#define YEAR_BASE    1900
#define EPOCH_YEAR      1970
#define EPOCH_WDAY      4
#define EPOCH_YEARS_SINCE_LEAP 2
#define EPOCH_YEARS_SINCE_CENTURY 70
#define EPOCH_YEARS_SINCE_LEAP_CENTURY 370

#define isleap(y) ((((y) % 4) == 0 && ((y) % 100) != 0) || ((y) % 400) == 0)

struct tm *gmtime_r_Custom(const time_t_custom *__restrict tim_p, struct tm *__restrict res) {
    long days, rem;
    const time_t_custom lcltime = *tim_p;
    int era, weekday, year;
    unsigned erayear, yearday, month, day;
    unsigned long eraday;

    days = lcltime / SECSPERDAY + EPOCH_ADJUSTMENT_DAYS;
    rem = lcltime % SECSPERDAY;
    if (rem < 0) {
        rem += SECSPERDAY;
        --days;
    }

    /* compute hour, min, and sec */
    res->tm_hour = (int) (rem / SECSPERHOUR);
    rem %= SECSPERHOUR;
    res->tm_min = (int) (rem / SECSPERMIN);
    res->tm_sec = (int) (rem % SECSPERMIN);



    /* compute year, month, day & day of year */
    /* for description of this algorithm see
     * http://howardhinnant.github.io/date_algorithms.html#civil_from_days */
    era = (days >= 0 ? days : days - (DAYS_PER_ERA - 1)) / DAYS_PER_ERA;
    eraday = days - era * DAYS_PER_ERA;    /* [0, 146096] */
    erayear = (eraday - eraday / (DAYS_PER_4_YEARS - 1) + eraday / DAYS_PER_CENTURY -
               eraday / (DAYS_PER_ERA - 1)) / 365;    /* [0, 399] */
    yearday = eraday - (DAYS_PER_YEAR * erayear + erayear / 4 - erayear / 100);    /* [0, 365] */
    month = (5 * yearday + 2) / 153;    /* [0, 11] */
    day = yearday - (153 * month + 2) / 5 + 1;    /* [1, 31] */
    month += month < 10 ? 2 : -10;
    year = ADJUSTED_EPOCH_YEAR + erayear + era * YEARS_PER_ERA + (month <= 1);


    res->tm_year = year - YEAR_BASE;
    res->tm_mon = month;
    res->tm_mday = day;


    return (res);
}

void satelliteCalculateForDateTimeSecs(const satelliteData_t *satelliteData, time_t_custom dateTimeSecs,
                                       satelliteResults_t *currentSatelliteData,
                                       satellitePredictionLevel_t predictionLevel) {
    struct tm timeAndDate;
    gmtime_r_Custom(&dateTimeSecs, &timeAndDate);

    uint32_t tmpDN = (uint32_t) satelliteDayFn((timeAndDate.tm_year + 1900), (timeAndDate.tm_mon + 1),
                                               timeAndDate.tm_mday);
    float tmpTN =
            ((float) timeAndDate.tm_hour + ((float) timeAndDate.tm_min + ((float) timeAndDate.tm_sec / 60.0)) / 60.0) /
            24.0;

    float tmpT = (tmpDN - satelliteData->keps.DE) +
                 (tmpTN - satelliteData->keps.TE_FloatPart);//83.848;	// Elapsed T since epoch
    float tmpDT = satelliteData->keps.DC * tmpT / 2.0;            // Linear drag terms
    float tmpKD = 1.0 + 4.0 * tmpDT;
    float tmpKDP = 1.0 - 7.0 * tmpDT;
    float tmpM =
            satelliteData->keps.MA + satelliteData->keps.MM * tmpT * (1.0 - 3.0 * tmpDT);    // Mean anomaly at YR,/ TN
    int tmpDR = (int) (tmpM / (2.0 * M_PI));        // Strip out whole no of revs
    tmpM = tmpM - tmpDR * 2.0 * M_PI;                // M now in range 0 - 2PI
    //currentSatelliteData.RN = satelliteData->keps.RV + tmpDR + 1;                   	// VK3KYY We don't need to know the Current orbit number

    // Solve M = EA - EC * sin(EA) for EA given M, by Newton's method
    float tmpEA = tmpM;                    // Initail solution
    float tmp;
    float tmpDNOM;
    float tmpC, tmpS;
    do {
        tmpC = cos(tmpEA);
        tmpS = sin(tmpEA);
        tmpDNOM = 1.0 - satelliteData->keps.EC * tmpC;
        tmp = (tmpEA - satelliteData->keps.EC * tmpS - tmpM) / tmpDNOM;    // Change EA to better resolution
        tmpEA = tmpEA - tmp;            // by this amount until converged
    } while (fabs(tmp) > 1.0E-5);

    // Distances
    float tmpA = satelliteData->keps.A0 * tmpKD;
    float tmpB = satelliteData->keps.b0 * tmpKD;
#if NEEDS_SATELLITE_LAT_LONG
    float tmpRS = tmpA * tmpDNOM;
#endif
    // Calculate satellite position and velocity in plane of ellipse
    float tmpSx = tmpA * (tmpC - satelliteData->keps.EC);
    float tmpVx = -tmpA * tmpS / tmpDNOM * satelliteData->keps.N0;
    float tmpSy = tmpB * tmpS;
    float tmpVy = tmpB * tmpC / tmpDNOM * satelliteData->keps.N0;

    float tmpAP = satelliteData->keps.WP + satelliteData->keps.WD * tmpT * tmpKDP;
    float tmpCWw = cos(tmpAP);
    float tmpSW = sin(tmpAP);
    float tmpRAAN = satelliteData->keps.RA + satelliteData->keps.QD * tmpT * tmpKDP;
    float tmpCO = cos(tmpRAAN);
    float tmpSO = sin(tmpRAAN);

    // Plane -> celestial coordinate transformation, [C] = [RAAN]*[IN]*[AP]
    float tmpCXx = tmpCWw * tmpCO - tmpSW * satelliteData->keps.CI * tmpSO;
    float tmpCXy = -tmpSW * tmpCO - tmpCWw * satelliteData->keps.CI * tmpSO;

    float tmpCYx = tmpCWw * tmpSO + tmpSW * satelliteData->keps.CI * tmpCO;
    float tmpCYy = -tmpSW * tmpSO + tmpCWw * satelliteData->keps.CI * tmpCO;

    float tmpCZx = tmpSW * satelliteData->keps.SI;
    float tmpCZy = tmpCWw * satelliteData->keps.SI;

    // Compute satellite's position vector, ANTenna axis unit vector
    // and velocity  in celestial coordinates. (Note: Sz = 0, Vz = 0)
    float tmpSATx = tmpSx * tmpCXx + tmpSy * tmpCXy;
    float tmpVELx = tmpVx * tmpCXx + tmpVy * tmpCXy;
    float tmpSATy = tmpSx * tmpCYx + tmpSy * tmpCYy;
    float tmpVELy = tmpVx * tmpCYx + tmpVy * tmpCYy;
    float tmpSATz = tmpSx * tmpCZx + tmpSy * tmpCZy;
    float tmpVELz = tmpVx * tmpCZx + tmpVy * tmpCZy;

    // Also express SAT, ANT, and VEL in geocentric coordinates
    float tmpGHAA = satelliteData->keps.GHAE + currentSatelliteData_WE * tmpT;        // GHA Aries at elaprsed time T
    tmpC = cos(-tmpGHAA);
    tmpS = sin(-tmpGHAA);
    tmpSx = tmpSATx * tmpC - tmpSATy * tmpS;
    tmpVx = tmpVELx * tmpC - tmpVELy * tmpS;
    tmpSy = tmpSATx * tmpS + tmpSATy * tmpC;
    tmpVy = tmpVELx * tmpS + tmpVELy * tmpC;

    float tmpRx = tmpSx - observerData.Ox;
    float tmpRy = tmpSy - observerData.Oy;
    float tmpRz = tmpSATz - observerData.Oz;

    float tmpR = sqrt(tmpRx * tmpRx + tmpRy * tmpRy + tmpRz * tmpRz);    /* Range Magnitute */

    // Normalize range vector
    tmpRx = tmpRx / tmpR;
    tmpRy = tmpRy / tmpR;
    tmpRz = tmpRz / tmpR;

    float tmpU = tmpRx * observerData.Ux + tmpRy * observerData.Uy + tmpRz * observerData.Uz;
    currentSatelliteData->elevation = rad2deg(asin(tmpU));

    if (predictionLevel == SATELLITE_PREDICTION_LEVEL_TIME_AND_ELEVATION_ONLY) {
        return;
    }

    float tmpE = tmpRx * observerData.Ex + tmpRy * observerData.Ey;
    float tmpN = tmpRx * observerData.Nx + tmpRy * observerData.Ny + tmpRz * observerData.Nz;

    currentSatelliteData->azimuth = rad2deg(satelliteAtnFn(tmpE, tmpN));
    currentSatelliteData->azimuthAsInteger = (int) (currentSatelliteData->azimuth + FLOAT_ROUNDING_CONSTANT);// round
    currentSatelliteData->elevationAsInteger = (currentSatelliteData->elevation < 0.0) ? ((int) (
            currentSatelliteData->elevation - FLOAT_ROUNDING_CONSTANT)) : ((int) (currentSatelliteData->elevation +
                                                                                  FLOAT_ROUNDING_CONSTANT));
    // Solve antenna vector along unit range vector, -r.a = cos(SQ)
    // SQ = deg(acos(-(Ax * Rx + Ay * Ry + Az * Rz)));

    if (predictionLevel == SATELLITE_PREDICTION_LEVEL_TIME_EL_AND_AZ) {
        return;
    }
    // else.. must be SATELLITE_PREDICTION_LEVEL_FULL

#if NEEDS_SATELLITE_LAT_LONG
    // Calculate sub-satellite Lat/Lon
    currentSatelliteData->longitude = rad2deg(satelliteAtnFn(tmpSy, tmpSx));		// Lon, + East
    currentSatelliteData->latitude = rad2deg(asin(tmpSATz / tmpRS));		// Lat, + North

    if (currentSatelliteData->longitude > 180.0 )
    {
           currentSatelliteData->longitude -= 360.0;			// -ve is degrees West
    }
#endif

    // Resolve Sat-Obs velocity vector along unit range vector. (VOz = 0)
    float rangeRate = (tmpVx - observerData.VOx) * tmpRx + (tmpVy - observerData.VOy) * tmpRy +
                      tmpVELz * tmpRz; // Range rate, km/sec
    float dopplerFactor = rangeRate / 299792.0;

    int rxDoppler = satelliteGetDoppler(dopplerFactor, satelliteData->rxFreq);
    int txDoppler = satelliteGetDoppler(dopplerFactor, satelliteData->txFreq);
    currentSatelliteData->rxFreq = satelliteData->rxFreq - rxDoppler;
    currentSatelliteData->txFreq = satelliteData->txFreq + txDoppler;
}

bool
satellitePredictNextPassFromDateTimeSecs(predictionStateMachineData_t *stateData, const satelliteData_t *satelliteData,
                                         time_t_custom startDateTimeSecs, time_t_custom limitDateTimeSecs,
                                         int maxIterations, satellitePass_t *nextPass) {
    satelliteResults_t currentSatelliteData;

    switch (stateData->state) {
        case PREDICTION_STATE_INIT_AOS:
            stateData->currentDateTimeSecs = startDateTimeSecs;
            stateData->timeStep = SATELLITE_PREDICTION_INITIAL_TIME_STEP;
            stateData->found = false;
            stateData->totalIterations = 0;
            stateData->foundStart = false;
            stateData->direction = 1;
            stateData->state = PREDICTION_STATE_FIND_AOS;
            nextPass->valid = PREDICTION_RESULT_NONE;
            nextPass->satelliteMaxElevation = -1;// not yet calculated

            //break;   deliberate drop through

        case PREDICTION_STATE_FIND_AOS:
            stateData->iterations = 0;

            do {
                stateData->currentDateTimeSecs += (stateData->timeStep * stateData->direction); // move  forward
                satelliteCalculateForDateTimeSecs(satelliteData, stateData->currentDateTimeSecs, &currentSatelliteData,
                                                  SATELLITE_PREDICTION_LEVEL_TIME_AND_ELEVATION_ONLY);

                if (!stateData->foundStart && currentSatelliteData.elevation >= 0) {
                    stateData->foundStart = true;
                }
                if (stateData->foundStart) {
                    if (stateData->timeStep == 1) {
                        if (currentSatelliteData.elevation >= 0) {
                            stateData->found = true;
                        } else {
                            stateData->direction = 1;
                        }
                    } else {
                        stateData->timeStep /= 2;
                        if (currentSatelliteData.elevation >= 0) {
                            stateData->direction = -1;
                        } else {
                            stateData->direction = 1;
                        }
                    }
                } else {
                    if (currentSatelliteData.elevation < -30) {
                        if (stateData->timeStep == SATELLITE_PREDICTION_INITIAL_TIME_STEP) {
                            stateData->timeStep = SATELLITE_PREDICTION_INITIAL_TIME_STEP * 4;
                        }
                    } else {
                        if (stateData->timeStep == (SATELLITE_PREDICTION_INITIAL_TIME_STEP * 4)) {
                            stateData->timeStep = SATELLITE_PREDICTION_INITIAL_TIME_STEP;
                        }
                    }
                }
                stateData->iterations++;
                stateData->totalIterations++;
            } while (!stateData->found &&
                     (stateData->iterations < maxIterations) &&
                     (stateData->totalIterations < MAX_TOTAL_ITERATIONS) &&
                     stateData->currentDateTimeSecs < limitDateTimeSecs);

            if (stateData->currentDateTimeSecs >= limitDateTimeSecs) {
                stateData->state = PREDICTION_STATE_LIMIT;
                return false;
            }

            if (!(stateData->iterations < maxIterations) || !(stateData->totalIterations < MAX_TOTAL_ITERATIONS)) {
                stateData->state = PREDICTION_STATE_ITERATION_LIMIT;
                return false;
            }

            if (stateData->found) {
                nextPass->satelliteAOS = stateData->currentDateTimeSecs;

                stateData->state = PREDICTION_STATE_INIT_LOS;
            }
            break;

        case PREDICTION_STATE_INIT_LOS:
            stateData->found = false;
            stateData->foundStart = false;
            stateData->timeStep = SATELLITE_PREDICTION_INITIAL_TIME_STEP;
            stateData->direction = 1;

            // deliberate drop through

        case PREDICTION_STATE_FIND_LOS:
            stateData->iterations = 0;

            do {
                stateData->currentDateTimeSecs += (stateData->timeStep * stateData->direction); // move  forward
                satelliteCalculateForDateTimeSecs(satelliteData, stateData->currentDateTimeSecs, &currentSatelliteData,
                                                  SATELLITE_PREDICTION_LEVEL_TIME_AND_ELEVATION_ONLY);

                if (!stateData->foundStart && currentSatelliteData.elevation < 0) {
                    stateData->foundStart = true;
                }
                if (stateData->foundStart) {
                    if (stateData->timeStep == 1) {
                        if (currentSatelliteData.elevation < 0) {
                            stateData->found = true;
                        } else {
                            stateData->direction = 1;
                        }
                    } else {
                        stateData->timeStep /= 2;
                        if (currentSatelliteData.elevation < 0) {
                            stateData->direction = -1;
                        } else {
                            stateData->direction = 1;
                        }
                    }

                }
                stateData->iterations++;

            } while (!stateData->found &&
                     (stateData->iterations < maxIterations) &&
                     (stateData->totalIterations < MAX_TOTAL_ITERATIONS) &&
                     stateData->currentDateTimeSecs < limitDateTimeSecs);

            if (stateData->currentDateTimeSecs >= limitDateTimeSecs) {
                stateData->state = PREDICTION_STATE_LIMIT;
                return false;
            }

            if (!(stateData->iterations < maxIterations) || !(stateData->totalIterations < MAX_TOTAL_ITERATIONS)) {
                stateData->state = PREDICTION_STATE_ITERATION_LIMIT;
                return false;
            }

            if (stateData->found) {
                nextPass->satelliteLOS = stateData->currentDateTimeSecs;

                nextPass->satellitePassDuration = nextPass->satelliteLOS - nextPass->satelliteAOS;

                //satelliteGetMaximumElevation(satelliteData , nextPass);// Use lazy calculation now

                stateData->state = PREDICTION_STATE_COMPLETE;
            }
            break;

        case PREDICTION_STATE_NONE:
        case PREDICTION_STATE_COMPLETE:
        case PREDICTION_STATE_LIMIT:
        case PREDICTION_STATE_ITERATION_LIMIT:
            return false;
            break;
    }

    return true;
}


uint16_t satelliteGetMaximumElevation(satelliteData_t *satelliteData, uint32_t passNumber) {
    float lastEl, halfPointElevation;
    satelliteResults_t resultsData;
    satellitePass_t *pass;
    time_t_custom dataTime;

// Step size of 1 does not seem to be needed even for passes which are directly overhead within less than 1.0 deg
#define MAX_ELE_FIND_STEP  2
#define MAX_ELE_MIN_STEP_CHANGE_DEG 0.01

    pass = &satelliteData->predictions.passes[passNumber];

    if (pass->satelliteMaxElevation >= 0) {
        return pass->satelliteMaxElevation;
    }

    dataTime = (pass->satelliteAOS + pass->satelliteLOS) / 2;// max height will be in middle of the pass... Probably
    satelliteCalculateForDateTimeSecs(satelliteData, dataTime, &resultsData,
                                      SATELLITE_PREDICTION_LEVEL_TIME_AND_ELEVATION_ONLY);

    pass->satelliteMaxElevation = (int16_t) (resultsData.elevation + FLOAT_ROUNDING_CONSTANT);

    halfPointElevation = resultsData.elevation;

    dataTime -= MAX_ELE_FIND_STEP;// try prior to mid point time

    satelliteCalculateForDateTimeSecs(satelliteData, dataTime, &resultsData,
                                      SATELLITE_PREDICTION_LEVEL_TIME_AND_ELEVATION_ONLY);

    if ((resultsData.elevation - halfPointElevation) > MAX_ELE_MIN_STEP_CHANGE_DEG) {
        do {
            lastEl = resultsData.elevation;
            pass->satelliteMaxElevation = (int16_t) (resultsData.elevation + FLOAT_ROUNDING_CONSTANT);
            dataTime -= MAX_ELE_FIND_STEP;
            satelliteCalculateForDateTimeSecs(satelliteData, dataTime, &resultsData,
                                              SATELLITE_PREDICTION_LEVEL_TIME_AND_ELEVATION_ONLY);

        } while ((resultsData.elevation - lastEl) > MAX_ELE_MIN_STEP_CHANGE_DEG);
    } else {
        dataTime += MAX_ELE_FIND_STEP *
                    2;// step in twice the direction because the previous test set the dateTime to one second before the mid time point.
        satelliteCalculateForDateTimeSecs(satelliteData, dataTime, &resultsData,
                                          SATELLITE_PREDICTION_LEVEL_TIME_AND_ELEVATION_ONLY);
        if ((resultsData.elevation - halfPointElevation) > MAX_ELE_MIN_STEP_CHANGE_DEG) {
            do {
                lastEl = resultsData.elevation;
                pass->satelliteMaxElevation = (int16_t) (resultsData.elevation + FLOAT_ROUNDING_CONSTANT);
                dataTime += MAX_ELE_FIND_STEP;
                satelliteCalculateForDateTimeSecs(satelliteData, dataTime, &resultsData,
                                                  SATELLITE_PREDICTION_LEVEL_TIME_AND_ELEVATION_ONLY);

            } while ((resultsData.elevation - lastEl) > MAX_ELE_MIN_STEP_CHANGE_DEG);
        }
    }

    return pass->satelliteMaxElevation;
}

static int currentlyPredictingSatellite = 0;
static predictionStateMachineData_t currentPrediction;
static bool hasSelectedSatellite = false;
typedef struct {

    uint32_t locationLat;// fixed point encoded as 1 sign bit, 8 bits integer, 23 bits as decimal
    uint32_t locationLon;// fixed point encoded as 1 sign bit, 8 bits integer, 23 bits as decimal
    uint8_t timezone;// Lower 7 bits are the timezone. 64 = UTC, values < 64 are negative TZ values.  Bit 8 is a flag which indicates TZ/UTC. 0 = UTC

} settingsStruct_t;
settingsStruct_t nonVolatileSettings;

double latLongFixedToDouble(uint32_t fixedVal) {
    // MS bit is the sign
    // Lower 10 bits are the fixed point to the right of the point.
    // Bits 15,14,13,12,11 are the integer part
    double inPart = (fixedVal & 0x7FFFFFFF) >> 23;
    double decimalPart = (fixedVal & 0x7FFFFF);
    if (fixedVal & 0x80000000) {
        inPart *= -1;// MS bit is set, so tha number is negative
        decimalPart *= -1;
    }
    return inPart + (decimalPart / ((double) LOCATION_DECIMAL_PART_MULIPLIER));
}

uint32_t doubleToLatLongFixed(double value) {
    int32_t integerPart = (int32_t) value; // 提取整数部分
    double decimalPart = value - integerPart; // 提取小数部分

    // 将小数部分乘以相应倍数，例如，若小数部分的精度为1/1000000，则乘以 1000000
    decimalPart *= LOCATION_DECIMAL_PART_MULIPLIER;

    // 将小数部分四舍五入到最接近的整数
    int32_t roundedDecimalPart = (int32_t) (0.5 + decimalPart);

    // 将整数部分和小数部分合并成一个整数，考虑符号位
    uint32_t fixedVal = (uint32_t) (roundedDecimalPart & 0x7FFFFF) | ((integerPart & 0x7FFFFFFF) << 23);

    // 如果原始值为负数，则设置符号位
    if (value < 0) {
        fixedVal |= 0x80000000;
    }

    return fixedVal;
}

static struct_codeplugChannel_t satelliteChannelData = {.rxFreq = 0};
static struct tm timeAndDate;
static satelliteResults_t currentSatelliteResults;

static void calculateActiveSatelliteData(bool forceFrequencyUpdate) {
    gmtime_r_Custom(&uiDataGlobal.dateTimeSecs, &timeAndDate);

    satelliteCalculateForDateTimeSecs(currentActiveSatellite, uiDataGlobal.dateTimeSecs, &currentSatelliteResults,
                                      SATELLITE_PREDICTION_LEVEL_FULL);

    uint32_t rxF = currentSatelliteResults.rxFreq / 10;
    uint32_t txF = currentSatelliteResults.txFreq / 10;
//    currentChannelData->txFreq = txF;// Tx freq can be updated immediately
    //TODO:将计算得到的接收频率和发射频率除以10，并更新当前频道数据的 txFreq（发射频率）。
    //如果强制更新频率（forceFrequencyUpdate 为真），或者当前接收频率与计算结果的接收频率相差超过500Hz（以10Hz分辨率计算），则更新当前频道数据的 rxFreq（接收频率）。

    // Handle VP for AOS and LOS
    if (currentSatelliteResults.elevation > 0)//如果卫星仰角大于0
    {
        //TODO:提示仰角大于0
    } else {

    }
    hasRecalculated = true;
}

/**
 * 这个函数处理卫星屏幕菜单。
 *
 * @param ev 包含事件数据的 uiEvent_t 结构体指针。
 * @param isFirstRun 表示是否是函数的第一次运行的标志。
 * @return 如果函数执行成功，则返回 MENU_STATUS_SUCCESS。
 */
time_t_custom mktime_custom(const struct tm *tb) {
    const int totalDays[] = {-1, 30, 58, 89, 119, 150, 180, 211, 242, 272, 303, 333, 364};

    time_t_custom t1;
    int days;

    days = totalDays[tb->tm_mon];
    if (!(tb->tm_year & 3) && (tb->tm_mon > 1)) {
        days++;
    }

    t1 = (tb->tm_year - (EPOCH_YEAR - 1900)) * DAYS_PER_YEAR + ((tb->tm_year - 1L) / 4) - 17 + days + tb->tm_mday;
    t1 = (t1 * HOURSPERDAY) + tb->tm_hour;
    t1 = (t1 * MINSPERHOUR) + tb->tm_min;
    t1 = (t1 * SECSPERMIN) + tb->tm_sec;

    return (time_t_custom) t1;
}
// 指示是否找到选定的过境
bool findSelectedPass = false;
// 选定的过境的 AOS（Acquisition of Signal）时间
uint32_t selectedPassAOS = 0;

// 计算给定卫星索引的预测数据
bool calculatePredictionsForSatelliteIndex(int satelliteIndex) {
    // 存储卫星计算结果的结构体
    satelliteResults_t results;
    // 获取当前卫星的指针
    satelliteData_t *satellite = &satelliteDataNative[satelliteIndex];

    // 检查当前时间是否在第一个预测过境的 AOS 和 LOS 之间
    if ((uiDataGlobal.dateTimeSecs >= satellite->predictions.passes[0].satelliteAOS) &&
        (uiDataGlobal.dateTimeSecs <= satellite->predictions.passes[0].satelliteLOS)) {

        // 设置卫星可见
        satellite->predictions.isVisible = true;
    } else {

        // 设置卫星不可见
        satellite->predictions.isVisible = false;
    }

    // 如果当前时间超过了第一个预测过境的 LOS，且当前不在预测过程中，则强制重新计算预测
    if ((satellite->predictions.numPasses > 0) &&
        (uiDataGlobal.dateTimeSecs > satellite->predictions.passes[0].satelliteLOS) &&
        !satellite->predictions.isPredicting) {
        // 重置标志位
        findSelectedPass = false;
        // 如果存在选定的过境，并且选定的 AOS 大于等于当前时间，则设置找到选定过境的标志位为真
        if (satellite->predictions.selectedPassNumber != 0) {
            selectedPassAOS = satellite->predictions.passes[satellite->predictions.selectedPassNumber].satelliteAOS;
            if (selectedPassAOS >= uiDataGlobal.dateTimeSecs) {
                findSelectedPass = true;
            }
        }

        // 查找过去的过境
        int passNum;
        for (passNum = 0; passNum < satellite->predictions.numPasses; passNum++) {
            if (!(uiDataGlobal.dateTimeSecs > satellite->predictions.passes[passNum].satelliteLOS)) {
                break;
            }
        }
        // 计算需要移动的过境数
        int numPassesToMove = satellite->predictions.numPasses - passNum;
        // 将剩余的预测过境向前移动
        memcpy(&satellite->predictions.passes[0], &satellite->predictions.passes[passNum],
               numPassesToMove * sizeof(satellitePass_t));

        // 计算需要清除的过境数
        int numPassesToClear = NUM_SATELLITE_PREDICTIONS - numPassesToMove;
        // 清除所有剩余的预测过境
        memset(&satellite->predictions.passes[numPassesToMove], 0x00, (numPassesToClear) * sizeof(satellitePass_t));

        // 更新预测过境的数量等参数
        satellite->predictions.numPasses = numPassesToMove;
        satellite->predictions.numPassBeingPredicted = numPassesToMove;
        satellite->predictions.listDisplayPassSearchStartIndex = 0;
        satellite->predictions.selectedPassNumber = 0;
        satellite->predictions.isPredicting = true;
        satellite->predictions.isVisible = false;
//        // 更新总预测卫星数量
//        numTotalSatellitesPredicted--;
    }

    // 如果当前预测过境的结果为无效，则开始预测下一个过境
    if (satellite->predictions.passes[satellite->predictions.numPassBeingPredicted].valid == PREDICTION_RESULT_NONE) {
        // 计算预测开始时间
        time_t_custom predictionStartTime;

        // 如果当前正在预测的过境为第一个，并且当前没有预测过程，则根据当前卫星的可见性设置预测开始时间
        if ((satellite->predictions.numPassBeingPredicted == 0) && (currentPrediction.state == PREDICTION_STATE_NONE)) {
            satelliteCalculateForDateTimeSecs(satellite, uiDataGlobal.dateTimeSecs, &results,
                                              SATELLITE_PREDICTION_LEVEL_TIME_AND_ELEVATION_ONLY);

            if (results.elevation < 0) {
                predictionStartTime = uiDataGlobal.dateTimeSecs;
            } else {
                predictionStartTime = uiDataGlobal.dateTimeSecs - (30 * 60);
            }
        }
            // 否则，设置预测开始时间为上一个过境的 AOS 后的 30 分钟
        else {
            predictionStartTime =
                    satellite->predictions.passes[satellite->predictions.numPassBeingPredicted - 1].satelliteAOS +
                    satellite->predictions.passes[satellite->predictions.numPassBeingPredicted -
                                                  1].satellitePassDuration + 30 * 60;
        }

        // 如果当前没有预测过程，则开始预测
        if ((currentPrediction.state == PREDICTION_STATE_NONE)) {
            currentPrediction.state = PREDICTION_STATE_INIT_AOS;
        }

        // 如果无法预测下一个过境，则将其结果标记为无效
        if (!satellitePredictNextPassFromDateTimeSecs(&currentPrediction, satellite, predictionStartTime,
                                                      (uiDataGlobal.dateTimeSecs + (24 * 60 * 60)), 500,
                                                      &satellite->predictions.passes[satellite->predictions.numPassBeingPredicted])) {
            satellite->predictions.passes[satellite->predictions.numPassBeingPredicted].valid = PREDICTION_RESULT_NONE;
        }

        // 根据当前预测状态处理结果
        switch (currentPrediction.state) {
            case PREDICTION_STATE_COMPLETE:
                if (satellite->predictions.passes[satellite->predictions.numPassBeingPredicted].satelliteAOS != 0) {
                    // 将预测结果标记为有效
                    satellite->predictions.passes[satellite->predictions.numPassBeingPredicted].valid = PREDICTION_RESULT_OK;
                    // 增加预测过境数量
                    satellite->predictions.numPasses++;
                    // 如果预测过境数量未超过限制，则更新选定过境的标号
                    if (satellite->predictions.numPasses < (NUM_SATELLITE_PREDICTIONS - 1)) {
                        if (findSelectedPass && (selectedPassAOS ==
                                                 satellite->predictions.passes[satellite->predictions.numPassBeingPredicted].satelliteAOS)) {
                            satellite->predictions.selectedPassNumber = satellite->predictions.numPassBeingPredicted;
                            findSelectedPass = false;
                        }
                        // 更新正在预测的过境标号
                        satellite->predictions.numPassBeingPredicted++;
                    } else {
                        // 如果预测过境数量超过限制，则标记最后一个过境为达到限制
                        satellite->predictions.numPasses--;
                        satellite->predictions.passes[satellite->predictions.numPassBeingPredicted].valid = PREDICTION_RESULT_LIMIT;
                        // 重置预测状态
                        currentPrediction.state = PREDICTION_STATE_NONE;
                        satellite->predictions.isPredicting = false;
                    }
                }
                // 重置预测状态
                currentPrediction.state = PREDICTION_STATE_NONE;
                break;

            case PREDICTION_STATE_ITERATION_LIMIT:
                // 处理迭代次数限制情况
                break;
            case PREDICTION_STATE_LIMIT:
                // 处理预测限制情况
                break;
            case PREDICTION_STATE_NONE:
            case PREDICTION_STATE_INIT_AOS:
            case PREDICTION_STATE_FIND_AOS:
            case PREDICTION_STATE_INIT_LOS:
            case PREDICTION_STATE_FIND_LOS:
                // 将预测结果标记为无效，并继续处理下一个过境
                satellite->predictions.passes[satellite->predictions.numPassBeingPredicted].valid = PREDICTION_RESULT_NONE;
                return false;
                break;
        }
    }
    // 返回 true 表示预测成功，否则表示预测尚未完成
    return true;
}
bool isFirstRun=true;
menuStatus_t menuSatelliteScreen() {
    // 如果是第一次运行，则进行初始化步骤
    if (isFirstRun) {
        // 初始化变量
        currentlyPredictingSatellite = 0;
        currentPrediction.state = PREDICTION_STATE_NONE;
        // 如果没有选择卫星
        if (!hasSelectedSatellite) {
            // 设置纬度
            nonVolatileSettings.locationLat = doubleToLatLongFixed(29.061166);
            nonVolatileSettings.locationLon = doubleToLatLongFixed(119.657234);
            struct tm buildDateTime;
            // 设置日期和时间
            memset(&buildDateTime, 0x00, sizeof(struct tm)); // 清除整个结构体
            buildDateTime.tm_year =time[0]+2000 - 1900; // 年份，从1900年开始
            buildDateTime.tm_mon = time[1] - 1; // 月份，从一月到十二月，对应0到11
            buildDateTime.tm_mday = time[2];     // 月份的第几天，从1到31
            buildDateTime.tm_hour =time[3];
            buildDateTime.tm_min =time[4];
            buildDateTime.tm_sec =time[5];
            uiDataGlobal.dateTimeSecs = mktime_custom(&buildDateTime);
            // 如果没有载入卫星轨道数据，则加载
            if (hasSatelliteKeps == 0) {
                loadKeps();
            }
            // 如果有卫星轨道数据
            if (hasSatelliteKeps) {
                // 设置观测者位置（高度参数设为零，因为在计算中似乎几乎没有影响）
                satelliteSetObserverLocation(
                        latLongFixedToDouble(nonVolatileSettings.locationLat),
                        latLongFixedToDouble(nonVolatileSettings.locationLon),
                        0);
            }
        }
        // 设置当前活动的卫星并更新屏幕
        currentActiveSatellite = &satelliteDataNative[currentlyPredictingSatellite];
        calculateActiveSatelliteData(true);
        if (hasSatelliteKeps) {

            // 对当前卫星进行预测，直到预测完成
            while (!calculatePredictionsForSatelliteIndex(currentlyPredictingSatellite)) {
            }
            while(1)
            show_uint32(123,0);
            // 获取正在预测的卫星
//            predictingSat = &satelliteDataNative[currentlyPredictingSatellite];
        }
        isFirstRun=false;
    }
    return MENU_STATUS_SUCCESS;
}
