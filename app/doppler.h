#ifndef _DOPPLER_
#define _DOPPLER_

#include "stdint.h"
#include "stdbool.h"

struct satellite_t {
    char name[10];
    uint8_t start_time[6];
    uint8_t end_time[6];
    uint16_t sum_time;
    uint16_t SEND_CTCSS;
    uint16_t RECV_CTCSS;
    uint32_t START_TIME_UNIX;
};
struct satellite_d {

    uint32_t UPLink;
    uint32_t DownLink;

};

void READ_DATA(int32_t time_diff, int32_t time_diff1);

void INIT_DOPPLER_DATA();

int32_t UNIX_TIME(uint8_t time2[6]);

extern struct satellite_d satellite_data;
extern bool DOPPLER_FLAG;
extern struct satellite_t satellite;
#endif