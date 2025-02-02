#ifndef FATE_TLE_H
#define FATE_TLE_H

#include <stdio.h>
#include "vec.h"
#include "stdint.h"
// typedef unsigned char uint8_t;
// typedef unsigned short uint16_t;
// typedef unsigned int uint32_t;
//#include "eci.h"
/**
 * A year represented as a 2-digit number
 */
typedef char *year;
/**
 * Represents the result of calculating the look position
 * for an observer to a satellite, where azimuth is the
 * clockwise rotation from true north and the altitude is
 * the degrees inclination above the observation plane.
 * Units are in degrees.
 */
typedef struct {
    double azimuth;
    double altitude;
    double range;
    vec observer_eci;
} look_result;

/**
 * Represents a location on the Earth with latitude and
 * longitude coordinates. Positive north and east. Units
 * are in degrees.
 */
typedef struct {
    double lat;
    double lon;
    double height;
    vec sate_eci; /* position */
    double distance ;
        vec r_dot; /* position */

} lat_lon;

typedef struct {
    /* TITLE LINE */
    char title[20];
    unsigned char title_len;

    /* LINE 1 */

    int sat_num;
    char class;

    char launch_yr[40];
    long launch_num;
    char launch_piece[20];
    unsigned char launch_piece_len;

    char  epoch_yr[40];
    double epoch_day;

    double d_mean_motion; /* n_0_dot / 2 */
    double dd_mean_motion; /* n_0_dot_dot / 6 */

    double drag;

    unsigned char ephemeris;
    long  element_num;
    /* checksum */

    /* LINE 2 */

    double inclination; /* i_0 */
    double r_node_ascension; /* omega_0 */
    double eccentricity; /* e_0 */
    double perigee_arg; /* omega_0 */
    double mean_anomaly; /* M_0 */
    double rev_per_day; /* n_0 */
    int rev_num;
    /* checksum */
} tle_data;
typedef struct {
    uint8_t num;//总数
    uint8_t list[45];//第几个可用
} satlist;
typedef struct {
    char name[10];
    char line1[70];
    char line2[70];
    uint16_t TX_TONE;
    uint16_t RX_TONE;
    uint32_t TX_FREQ;
    uint32_t RX_FREQ;
    uint32_t UP_LINK;
    uint32_t DOWN_LINK;
} sat_parm;
/**
 * Parses and TLE-formatted data using the 3 line format
 * that includes the title line.
 *
 * @param title the title
 * @param line1 the first line
 * @param line2 the second line
 * @return the TLE parsed data, or NULL on failure
 */
int tle_parse(const char *title, const char *line1, const char *line2, tle_data *data) ;
/**
 * Prints the given TLE data to the given stream.
 *
 * @param data the data to print
 */
void tle_print(FILE *stream, tle_data *data);

/**
 * Relinquishes memory allocated to hold the tle_data
 * struct.
 *
 * @param data the data struct to free
 */
uint8_t tle_check();
void Read_TLE(uint8_t num,sat_parm *now_sat);

void tle_free(tle_data *data);
uint8_t cal_sat(char line0[25],char line1[70],char line2[70], lat_lon observer,look_result *look,lat_lon *sub_point,uint8_t my_time[6]);
extern satlist sate_info;
void TLE_Main();
 void TLE_KEY();
extern uint8_t is_PTT;
extern sat_parm sat_get;
extern uint8_t off_PTT;
extern sat_parm sat_get;
void floatToString(double num, char *str) ;

#endif /* FATE_TLE_H */
