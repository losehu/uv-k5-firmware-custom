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
    uint16_t START_TIME_UNIX;
};
struct satellite_d
{
uint8_t AZ_I;
uint8_t AZ_F;
uint8_t EI_I;
uint8_t EI_F;
uint8_t SIGN;
uint32_t UPLink;
uint32_t DownLink;
uint16_t DIS_I;
uint8_t DIS_F;

};
void READ_DATA(int32_t time_diff, int32_t time_diff1) ;

void INIT_DOPPLER_DATA();
int32_t UNIX_TIME(uint8_t time2[6]) ;
extern struct satellite_d satellite_data;
extern uint8_t sate_state;

extern struct satellite_t satellite;
#endif