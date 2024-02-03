#ifndef _DOPPLER_
#define _DOPPLER_
#include "stdint.h"
struct satellite_t
{
    char name[10];
    uint8_t start_time[6];
    uint8_t end_time[6];
    uint16_t sum_time;
    uint16_t SEND_CTCSS;
    uint16_t RECV_CTCSS;

};
void INIT_DOPPLER_DATA();
int32_t TIME_DIFF(uint8_t time1[6],uint8_t time2[6]);

extern struct satellite_t satellite;
#endif