//
// Created by RUPC on 2024/1/30.
//

#ifndef PICTURE_PY_DOPPLER_H
#define PICTURE_PY_DOPPLER_H
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "driver/keyboard.h"
#define NUM_SATELLITES 1
#define NUM_SATELLITE_PREDICTIONS NUM_SATELLITES
#define ONEPPM 1.0e-6
#define M_PI 3.14159265358979323846
#define deg2rad(deg) M_PI / 180.0 * deg
#define rad2deg(rad) rad * 180.0 / M_PI

// Days in a year
#define SATELLITE_YM  365.25f

// Tropical year, days
#define SATELLITE_YT 365.2421970


#define FLOAT_ROUNDING_CONSTANT 0.4999999


// WGS-84 Earth Ellipsoid
#define 	satData_RE  6378.137f
#define 	satData_FL  (1.0 / 298.257224)
#define 	currentSatelliteData_RP (satData_RE * (1.0 - satData_FL))
#define		currentSatelliteData_XX  (satData_RE * satData_RE)
#define		currentSatelliteData_ZZ  (currentSatelliteData_RP * currentSatelliteData_RP)

// YM = 365.25;		Mean year, days
// YT = 365.2421970;	Tropical year, days


// Earth's rotation rate, rads/whole day
#define		currentSatelliteData_WW  (2 * M_PI / SATELLITE_YT)

// Earth's rotation rate, rads/day
#define		currentSatelliteData_WE  (2 * M_PI + currentSatelliteData_WW)

// Earth's rotation rate, rads/sec
#define		currentSatelliteData_W0  (currentSatelliteData_WE / 86400)

// Earth's gravitational constant km^3/s^2
#define currentSatelliteData_GM  3.986E5

// 2nd Zonal coeff, Earth's gravity Field
#define currentSatelliteData_J2  1.08263E-3



// Sideral and solar data. Never needs changing. Valid to year 2000+

// GHAA, Year YG, Jan 0.0
#define currentSatelliteData_YG  2010

#define currentSatelliteData_G0  99.5578
// MA Sun and rate, deg, deg/day
#define currentSatelliteData_MAS0  356.4485
#define currentSatelliteData_MASD  0.98560028

// Sun's equation of center terms
#define currentSatelliteData_EQC1  0.03341
#define currentSatelliteData_EQC2  0.00035
#define LOCATION_DECIMAL_PART_MULIPLIER 1000000
typedef struct
{
    char name[16];
    uint32_t rxFreq;
    uint32_t txFreq;
    uint8_t chMode;
    uint8_t libreDMR_Power;
    uint8_t txRefFreq;
    uint8_t tot;
    uint8_t totRekey;
    uint8_t admitCriteria;
    uint8_t rssiThreshold;
    uint8_t scanList;
    uint16_t rxTone;
    uint16_t txTone;
    uint8_t voiceEmphasis;
    uint8_t txSignaling;
    uint8_t LibreDMR_flag1; // was unmuteRule. 0x80: Optional DMRID sets, 0x40: no beep, 0x20: no Eco, 0x10: OutOfBand(MD9600 only, never saved in codeplug)
    uint8_t rxSignaling;    // +--
    uint8_t artsInterval;   // | These 3 bytes were repurposed for optional DMRID
    uint8_t encrypt;        // +--
    uint8_t rxColor;
    uint8_t rxGroupList;
    uint8_t txColor;
    uint8_t emgSystem;
    uint16_t contact;
    uint8_t flag1;// lower 3 bits TA Tx control
    uint8_t flag2; // bits... 0x40 = TS
    uint8_t flag3;// bits... 0x20 = DisableAllLeds
    uint8_t flag4;// bits... 0x80 = Power, 0x40 = Vox, 0x20 = ZoneSkip (AutoScan), 0x10 = AllSkip (LoneWoker), 0x08 = AllowTalkaround, 0x04 = OnlyRx, 0x02 = Channel width, 0x01 = Squelch
    uint16_t VFOoffsetFreq;
    uint8_t VFOflag5;// upper 4 bits are the step frequency 2.5,5,6.25,10,12.5,25,30,50kHz
    uint8_t sql;// Does not seem to be used in the official firmware and seems to be always set to 0
    uint8_t NOT_IN_CODEPLUG_flag; // bit 0x01 = vfo channel
    uint8_t padding;
    uint32_t magicNumber;

} struct_codeplugChannel_t;



typedef enum
{
    MENU_STATUS_SUCCESS     = 0,
    MENU_STATUS_ERROR       = (1 << 0),
    MENU_STATUS_LIST_TYPE   = (1 << 1),
    MENU_STATUS_INPUT_TYPE  = (1 << 2),
    MENU_STATUS_FORCE_FIRST = (1 << 3)
} menuStatus_t;
typedef enum
{
    SATELLITE_PREDICTION_LEVEL_TIME_AND_ELEVATION_ONLY, SATELLITE_PREDICTION_LEVEL_TIME_EL_AND_AZ, SATELLITE_PREDICTION_LEVEL_FULL
} satellitePredictionLevel_t;
typedef enum satellitePreductionState { PREDICTION_STATE_NONE, PREDICTION_STATE_INIT_AOS, PREDICTION_STATE_FIND_AOS, PREDICTION_STATE_INIT_LOS, PREDICTION_STATE_FIND_LOS, PREDICTION_STATE_COMPLETE, PREDICTION_STATE_LIMIT, PREDICTION_STATE_ITERATION_LIMIT } satellitePreductionState_t;
typedef struct
{
    float		elevation;			// Elevaton
    float		azimuth;			// Azimuth
    int			elevationAsInteger;
    int			azimuthAsInteger;
#if NEEDS_SATELLITE_LAT_LONG
    float		longitude;			// Lon, + East
	float		latitude;			// Lat, + North
#endif
    uint32_t 	rxFreq;				// resultant rxFreq corrected for Doppler
    uint32_t 	txFreq;				// resultant txFreq corrected for Doppler
} satelliteResults_t;
typedef struct
{
    float		Ex;
    float		Ey;
    float		Ny;
    float		Nx;
    float		Nz;
    float		Ox;
    float		Oy;
    float		Oz;
    float		Ux;		// Observer's unit vectors UP EAST and NORTH in GEOCENTRIC coords.
    float		Uy;
    float		Uz;
    float		VOy;
    float		VOx;
    float		VOz;
    float		LatInRadians;
    float		LonInRadians;
    float		HeightInKilometers;
} satelliteObserver_t;
typedef unsigned int time_t_custom;     /* date/time in unix secs past 1-Jan-70 */
typedef enum {PREDICTION_RESULT_NONE, PREDICTION_RESULT_OK, PREDICTION_RESULT_LIMIT, PREDICTION_RESULT_TIMEOUT } predictionResult_t;
typedef struct
{
    time_t_custom	currentDateTimeSecs;
    int 			timeStep;
    bool 			found;
    int 			iterations;
    int 			totalIterations;
    bool 			foundStart;
    int 			direction;
    satellitePreductionState_t state;
} predictionStateMachineData_t;

typedef struct
{
    char 		TLE_Name[16];
    char 		TLE_Line1[70];
    char 		TLE_Line2[70];
    uint32_t	rxFreq;
    uint32_t	txFreq;
    uint16_t	txCTCSS;
    uint16_t	armCTCSS;
} codeplugSatelliteData_t;
typedef struct
{
    float 		DE;
    float		TE_FloatPart;		// Epoch time (days)
    float		SI;		// Sin Inclination (deg)
    float		CI;		// Cos Inclination (deg)
    float		RA;		// R.A.A.N (deg)
    float		EC;		// Eccentricity
    float		WP;		// Arg perifee (deg)
    float		MA;		// Mean anomaly (rev/d)
    float		MM;		// Mean motion (rev/d)
    float 		N0; 	// MM / 86400.0;			// Mean motion rads/s
    float		A0;		// pow(currentSatelliteData_GM / kepData->N0 / kepData->N0, 1.0 / 3.0);	// Semi major axis km
    float		b0;		// Semi minor axis (km)
    float		M2;		// Decay rate (rev/d/d)
    long		RV;		// Orbit number
    float		ALAT;		// Sat attitude (deg) 0 = nominal
    float		QD;		// Node precession rate, rad/day
    float		WD;		// Perigee precession rate, rad/day
    float		DC;		// Drag coeff. (Angular momentum rate)/(Ang mom)  s^-1
    float		GHAE;		// GHA Aries, epoch

} satelliteKeps_t;
typedef struct
{
    time_t_custom				satelliteAOS;
    time_t_custom				satelliteLOS;
    int16_t						satelliteMaxElevation;
    time_t_custom				satellitePassDuration;
    predictionResult_t			valid;
} satellitePass_t;
typedef struct
{
    uint32_t numPasses;
    uint32_t numPassBeingPredicted;
    uint32_t listDisplayPassSearchStartIndex;
    satellitePass_t passes[NUM_SATELLITE_PREDICTIONS];
    uint32_t selectedPassNumber;
    bool 	isPredicting;
    bool	isVisible;
} satellitePredictions_t;
typedef struct
{
    char 		name[17];
    satelliteKeps_t keps;

    uint32_t	rxFreq;
    uint32_t	txFreq;
    uint16_t	txCTCSS;
    uint16_t	armCTCSS;

    satellitePredictions_t predictions;
} satelliteData_t;
typedef struct
{

    time_t_custom		dateTimeSecs;// Epoch (00:00:00 UTC, January 1, 1970)

} uiDataGlobal_t;
struct tm
{
    int tm_sec;  /*秒，正常范围0-59， 但允许至61*/
    int tm_min;  /*分钟，0-59*/
    int tm_hour; /*小时， 0-23*/
    int tm_mday; /*日，即一个月中的第几天，1-31*/
    int tm_mon;  /*月， 从一月算起，0-11*/
    int tm_year;  /*年， 从1900至今已经多少年*/
//    int tm_wday; /*星期，一周中的第几天， 从星期日算起，0-6*/
//    int tm_yday; /*从今年1月1日到目前的天数，范围0-365*/
//    int tm_isdst; /*日光节约时间的旗标*/
};
extern satelliteData_t satelliteDataNative[NUM_SATELLITES];// Store native format Keps for each satellite
menuStatus_t menuSatelliteScreen() ;

#endif //PICTURE_PY_DOPPLER_H
