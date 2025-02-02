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
char *bwOptions[] = {"  25k", "12.5k", "6.25k"};
uint32_t currentFreq;
uint8_t DOPPLER_FLASH=0;
uint8_t  is_PTT=0;
uint8_t off_PTT=0;
uint8_t ENABLE_BTC=1;
#define ENABLE_PIC
sat_parm sat_get;
uint8_t local_dot_flag=0;
typedef enum {
    SELECT = 1,
    SATE = 2,
    TIME = 3,
    LOCAL=4,
    PIC=5
} Mode;
Mode before_mode;

uint8_t local_selected=1;
uint8_t local_inflag=0;
uint8_t BACK_ON=1;
uint8_t switch_mode=0;
Mode mode=SELECT;
satlist sate_info;
look_result look;
lat_lon sub_point;
lat_lon observer = { -0.1234567890123456789, 5.1234567890123456789, -50.1234567890123456789 };
uint8_t now_menu=1;

const uint8_t BITMAP_ARRAY_DOWN[5] =
{
    0b00000110,
    0b00011110,
    0b01111110,
    0b00011110,
    0b00000110
};
const uint8_t BITMAP_ARRAY_UP[5] =
{
    0b01100000,
    0b01111000,
    0b01111110,
    0b01111000,
    0b01100000
};
const uint8_t BITMAP_ARRAY_EMPTY_RIGHT[7] =
{
//                0b00000000,
    0b01111111,
    0b01000001,
    0b00100010,
    0b00100010,
    0b00010100,
    0b00010100,
    0b00001000
};
const uint8_t BITMAP_ARRAY_RIGHT[7] =
{
//                0b00000000,
    0b01111111,
    0b01111111,
    0b00111110,
    0b00111110,
    0b00011100,
    0b00011100,
    0b00001000
};
const uint8_t BITMAP_ARRAY_LEFT[7] =
{
//                0b00000000,
    0b00001000,
    0b00011100,
    0b00011100,
    0b00111110,
    0b00111110,
    0b01111111,
    0b01111111
};
#define NUM_PER_PAGE 6

/**
 * Designed to parse space-track.org title lines, they are
 * prefixed with "0 " so the max len would be 26 and not 24
 *
 * @param data the data to fill
 * @param title the title line
 * @return 0 if invalid length
 */
#define C 299792.458  // 光速 (km/s)
// 向量点积
double dot(vec a, vec b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// 向量减法 (a - b)
vec vec_sub(vec a, vec b) {
    vec result = {a.x - b.x, a.y - b.y, a.z - b.z};
    return result;
}

// 计算多普勒频移
uint32_t doppler_shift(double f, vec sat_pos, vec sat_vel, vec obs_pos, uint8_t is_uplink) {
    vec los = vec_sub(sat_pos, obs_pos);  // 视线方向向量 (Line of Sight)
    double los_mag = sqrt(dot(los, los)); // 视线向量模长
    vec los_unit = {los.x / los_mag, los.y / los_mag, los.z / los_mag}; // 归一化

    double vr = dot(sat_vel, los_unit); // 计算径向速度
    double f_shifted;
    if (is_uplink) {
        // 上行频率：卫星接近地面时频率增大，远离地面时频率减小
        f_shifted = f * (C + vr) / C; // 上行频率，卫星向观测点接近时 vr > 0，频率上升
    } else {
        // 下行频率：卫星接近地面时频率减小，远离地面时频率增大
        f_shifted = f * (C - vr) / C; // 下行频率，卫星远离地面时 vr > 0，频率上升
    }
    return (uint32_t)round(f_shifted);
}

uint8_t cal_sat(char line0[25],char line1[70],char line2[70], lat_lon observer,look_result *look,lat_lon *sub_point,uint8_t my_time[6])
{
    tle_data data;  // 直接在栈上创建一个tle_data

    if (tle_parse(line0, line1, line2, &data) == 0)   // 修改为传递指�?
        return 0;
    jd target = to_jd(my_time[0] + 2000, my_time[1],my_time[2], my_time[3], my_time[4], my_time[5] );
    *look = eci_to_look(&data, observer, target);
    *sub_point = eci_to_lat_lon(&data, target);
    sub_point->distance= sqrt((look->observer_eci.x-sub_point->sate_eci.x)*(look->observer_eci.x-sub_point->sate_eci.x) \
                              + (look->observer_eci.y-sub_point->sate_eci.y)*(look->observer_eci.y-sub_point->sate_eci.y) \
                              + (look->observer_eci.z-sub_point->sate_eci.z)*(look->observer_eci.z-sub_point->sate_eci.z) );

    return 1;
}

static int parse_title(tle_data *data, const char *title) {
    unsigned char len = (unsigned char) strlen(title);
    if (len > 26) {
        return 0;
    }

    /*  Cut the beginning "0 " and the newline */
    len -= 3;

    data->title_len = len;
    substr(title, 2, len,data->title);
    if (data->title == NULL) {
        return 0;
    }

    return 1;
}

static int compute_checksum(const char *str) {
    int total = 0;
    for (int i = 0; i < 68; i++) {
        char c = str[i];
        if (c == '-') {
            total += 1;
        } else if (c >= '0' && c <= '9') {
            total += c - '0';
        }
    }

    return total % 10;
}

static int parse_line1(tle_data *data, const char *line1) {
    int checksum = line1[68] - '0';
    if (compute_checksum(line1) != checksum) {
        // puts("Checksum failed");
        return 0;
    }

    /* sat_num */
    char sat_num_str[8];
    substr(line1, 2, 5,sat_num_str);


    if (strl(sat_num_str, (long *) &data->sat_num) == 0) {
        //free(sat_num_str);
        return 0;
    }


    /* class */
    data->class = line1[7];

    /* launch_yr */
    substr(line1, 9, 2,data->launch_yr );

    /* launch_num */
    char launch_num_str[5] ;
    substr(line1, 11, 3,launch_num_str);


    if (strl(launch_num_str, (long *) &data->launch_num) == 0) {
        //free(launch_num_str);
        return 0;
    }

    //free(launch_num_str);

    /* launch_piece */
    substr(line1, 14, 3,  data->launch_piece );
    if (data->launch_piece == NULL) {
        return 0;
    }
    data->launch_piece_len = (unsigned char) strlen(data->launch_piece);

    /* epoch_yr */
    substr(line1, 18, 2,data->epoch_yr);
    if (data->epoch_yr == NULL) {
        return 0;
    }

    /* epoch_day */
    char epoch_day_str[20];
    substr(line1, 20, 12,epoch_day_str);
    if (epoch_day_str == NULL) {
        return 0;
    }

    if (strd(epoch_day_str, &data->epoch_day) == 0) {
        //free(epoch_day_str);
        return 0;
    }

    //free(epoch_day_str);

    /* d_mean_motion */
    char d_mean_motion_str[20];
    substr(line1, 33, 10,d_mean_motion_str);

    if (strd(d_mean_motion_str, &data->d_mean_motion) == 0) {
        //free(d_mean_motion_str);
        return 0;
    }

    //free(d_mean_motion_str);

    /* dd_mean_motion */
    double dd_mean_motion_mul = 0;
    char dd_mean_motion_mul_str[20] ;
    substr(line1, 44, 6,dd_mean_motion_mul_str);
    if (dd_mean_motion_mul_str == NULL) {
        return 0;
    }

    if (strd(dd_mean_motion_mul_str, &dd_mean_motion_mul) == 0) {
        //free(dd_mean_motion_mul_str);
        return 0;
    }

    //free(dd_mean_motion_mul_str);
    dd_mean_motion_mul /= 100000;

    int dd_mean_motion_exp = 0;
    char dd_mean_motion_exp_str[20] ;
    substr(line1, 50, 2,dd_mean_motion_exp_str);
    if (dd_mean_motion_exp_str == NULL) {
        return 0;
    }

    if (strl(dd_mean_motion_exp_str, (long *) &dd_mean_motion_exp) == 0) {
        //free(dd_mean_motion_exp_str);
        return 0;
    }

    //free(dd_mean_motion_exp_str);
    data->dd_mean_motion = dd_mean_motion_mul * pow(10, dd_mean_motion_exp);

    /* drag */
    double drag_mul = 0;
    char drag_mul_str[20] ;
    substr(line1, 53, 6,drag_mul_str);
    if (drag_mul_str == NULL) {
        return 0;
    }

    if (strd(drag_mul_str, &drag_mul) == 0) {
        //free(drag_mul_str);
        return 0;
    }

    //free(drag_mul_str);
    drag_mul /= 100000;

    int drag_exp = 0;
    char drag_exp_str[20] ;
    substr(line1, 59, 2,drag_exp_str);
    if (drag_exp_str == NULL) {
        return 0;
    }

    if (strl(drag_exp_str, (long *) &drag_exp) == 0) {
        //free(drag_exp_str);
        return 0;
    }

    //free(drag_exp_str);
    data->drag = drag_mul * pow(10, drag_exp);

    /* ephemeris */
    data->ephemeris = (unsigned char) line1[62];

    /* element_num */
    char element_num_str[20] ;
    substr(line1, 64, 4,element_num_str);
    if (element_num_str == NULL) {
        return 0;
    }

    if (strl(element_num_str, (long *) &data->element_num) == 0) {
        //free(element_num_str);
        return 0;
    }

    //free(element_num_str);

    return 1;
}

static int parse_line2(tle_data *data, const char *line2) {
    int checksum = line2[68] - '0';
    if (compute_checksum(line2) != checksum) {
        //  puts("Checksum failed");
        return 0;
    }

    /* ignore satellite number */

    /* inclination */
    char inclination_str[20];
    substr(line2, 8, 8,inclination_str);
    if (inclination_str == NULL) {
        return 0;
    }

    if (strd(inclination_str, &data->inclination) == 0) {
        //free(inclination_str);
        return 0;
    }

    //free(inclination_str);

    /* r_node_ascension */
    char r_node_ascension_str[20] ;
    substr(line2, 17, 8,r_node_ascension_str);
    if (r_node_ascension_str == NULL) {
        return 0;
    }

    if (strd(r_node_ascension_str, &data->r_node_ascension) == 0) {
        //free(r_node_ascension_str);
        return 0;
    }

    //free(r_node_ascension_str);

    /* eccentricity */
    double eccentricity = 0;
    char eccentricity_str[20] ;
    substr(line2, 26, 7,eccentricity_str);
    if (eccentricity_str == NULL) {
        return 0;
    }

    if (strd(eccentricity_str, &eccentricity) == 0) {
        //free(eccentricity_str);
        return 0;
    }

    //free(eccentricity_str);
    data->eccentricity = eccentricity /= 10000000;

    /* perigee_arg */
    char perigee_arg_str[20] ;
    substr(line2, 34, 8,perigee_arg_str);
    if (perigee_arg_str == NULL) {
        return 0;
    }

    if (strd(perigee_arg_str, &data->perigee_arg) == 0) {
        //free(perigee_arg_str);
        return 0;
    }

    //free(perigee_arg_str);

    /* mean_anomaly */
    char mean_anomaly_str[20] ;
    substr(line2, 43, 8,mean_anomaly_str);
    if (mean_anomaly_str == NULL) {
        return 0;
    }

    if (strd(mean_anomaly_str, &data->mean_anomaly) == 0) {
        //free(mean_anomaly_str);
        return 0;
    }

    //free(mean_anomaly_str);

    /* rev_per_day */
    char rev_per_day_str[20] ;
    substr(line2, 52, 11,rev_per_day_str);
    if (rev_per_day_str == NULL) {
        return 0;
    }

    if (strd(rev_per_day_str, &data->rev_per_day) == 0) {
        //free(rev_per_day_str);
        return 0;
    }

    //free(rev_per_day_str);

    /* rev_num */
    char rev_num_str[20];
    substr(line2, 63, 5,rev_num_str);
    if (rev_num_str == NULL) {
        return 0;
    }

    if (strl(rev_num_str, (long *) &data->rev_num) == 0) {
        //free(rev_num_str);
        return 0;
    }

    //free(rev_num_str);

    return 1;
}

int tle_parse(const char *title, const char *line1, const char *line2, tle_data *data) {
//    if (parse_title(data, title) == 0) {
//        return 0;  // 解析失败
//    }

    if (parse_line1(data, line1) == 0) {
        return 0;  // 解析失败
    }

    if (parse_line2(data, line2) == 0) {
        return 0;  // 解析失败
    }

    return 1;  // 成功解析
}

void tle_print(FILE *stream, tle_data *data) {
    // fprintf(stream, "Title: %s\n", data->title);
    // fprintf(stream, "Satellite Number: %d\n", data->sat_num);
    // fprintf(stream, "Classifier: %c\n", data->class);
    // fprintf(stream, "Launch Year: %s\n", data->launch_yr);
    // fprintf(stream, "Launch Number: %d\n", data->launch_num);
    // fprintf(stream, "Launch Piece: %s\n", data->launch_piece);
    // fprintf(stream, "Epoch Year: %s\n", data->epoch_yr);
    // fprintf(stream, "Epoch Day: %f\n", data->epoch_day);
    // fprintf(stream, "Derivative of Mean Motion / 2: %f\n", data->d_mean_motion);
    // fprintf(stream, "2nd Derivative of Mean Motion / 6: %f\n", data->dd_mean_motion);
    // fprintf(stream, "B* Drag Term: %f\n", data->drag);
    // fprintf(stream, "Ephemeris Type: %c\n", data->ephemeris);
    // fprintf(stream, "Element Number: %d\n", data->element_num);
    // fprintf(stream, "Inclination: %f\n", data->inclination);
    // fprintf(stream, "Right Node Ascension: %f\n", data->r_node_ascension);
    // fprintf(stream, "Eccentricity: %f\n", data->eccentricity);
    // fprintf(stream, "Argument of Perigee: %f\n", data->perigee_arg);
    // fprintf(stream, "Mean Anomaly: %f\n", data->mean_anomaly);
    // fprintf(stream, "Revolutions Per Day: %f\n", data->rev_per_day);
    // fprintf(stream, "Revolutions: %d\n", data->rev_num);
}




void tle_free(tle_data *data) {
    //free(data->title);
    //free(data->launch_yr);
    //free(data->launch_piece);
    //free(data->epoch_yr);
    //free(data);
}


void draw_horizontal_line(int x,int y0, int y1) {
    for (int y = y0; y <= y1; y++) {
        if(x>=0 &&y >=0 &&x<128 &&y<64)
            // DrawPoint(x, y);

            if(y<8)     UI_DrawPixelBuffer(&gStatusLine, x, y, look.altitude>=0);
            else     UI_DrawPixelBuffer(gFrameBuffer, x, y-8, look.altitude>=0);

    }
}
void fill_circle(uint8_t center_x,uint8_t center_y)
{
    static bool drawed_line=false;
    float r=5;

    float start_x=center_x-r;
    float start_y=center_y;

    float num_point=1;
    for(float i=-r; i<=r; i+=num_point)
    {
        if(start_x+(r/4)>i+start_x+r || i+start_x+r>start_x+2*r-r/4)
            num_point=0.1;
        else
            num_point=1;
        //y=sqrt(30*30-x*x)
        float y=sqrt(r*r-i*i);
        draw_horizontal_line(round(i+start_x+r),round(start_y-y*2/3), round(start_y+y*2/3));
        DrawPoint(round(i+start_x+r),  round(start_y+y*2/3));
        DrawPoint(round(i+start_x+r),  round(start_y-y*2/3));

    }
}
//const char *line0 = "ISS (ZARYA)";
//const char *line1 = "1 25544U 98067A   25026.23381182  .00024447  00000+0  42619-3 0  9998";
//const char *line2 = "2 25544  51.6395 290.1622 0002159 133.7357  10.9595 15.50580823493146";
//uint8_t my_time[6]={25,1,27,8,28,42};
//look_result look;lat_lon sub_point;
//lat_lon observer = { 50, 50, 50 };
//cal_sat(line0,line1,line2,observer,&look,&sub_point,my_time);
void drawcircle(uint8_t center_x,uint8_t center_y,uint8_t r)
{
    uint8_t start_x=center_x-r;
    uint8_t start_y=center_y;

    // uint8_t r=45;
    float num_point=1;
    for(float i=-r; i<=r; i+=num_point)
    {
        if(start_x+(r/4)>i+start_x+r || i+start_x+r>start_x+2*r-r/4)
            num_point=0.1;
        else
            num_point=1;
        //y=sqrt(30*30-x*x)
        float y=sqrt(r*r-i*i);
        DrawPoint(i+start_x+r, round( start_y+y*2/3));
        DrawPoint(i+start_x+r, round( start_y-y*2/3));

    }

    for(int i=0; i<=r; i++)
    {
        DrawPoint(start_x+i+r,  start_y);
        DrawPoint(start_x-i+r,  start_y);

    }
    for(int i=0; i<=r; i++)
    {
        DrawPoint(start_x+r,  start_y-(i)*2/3);
        DrawPoint(start_x+r,  start_y+(i)*2/3);
    }

}





void Read_TLE(uint8_t num,sat_parm *now_sat)
{
    uint32_t base_add  =0x1E200+num*160;
    char c[160];
    int www=32;
    for(int i=0; i<160; i+=www) {
        EEPROM_ReadBuffer(base_add+i,c+i,www) ;
    }

    memcpy(now_sat->name,c,9);//卫星名字9B不包含结束0
    memcpy( now_sat->line1,c+9,69);//卫星TLE第一行 69B不包含结束0
    memcpy(now_sat->line2,c+9+69,69);//卫星TLE第二行 69B不包含结束0
    now_sat->TX_TONE=c[9+69+69]+(c[9+69+69+1]<<8);//上行发射亚音 uint16，低位在前高位在后
    now_sat->RX_TONE=c[9+69+69+2]+(c[9+69+69+3]<<8);//下行接受亚音 uint16，低位在前高位在后
    now_sat->TX_FREQ=c[9+69+69+4]+(c[9+69+69+5]<<8)+(c[9+69+69+6]<<16)+(c[9+69+69+7]<<24);// 上行发射频率/10 uint32，低位在前高位在后
    now_sat->RX_FREQ=c[9+69+69+8]+(c[9+69+69+9]<<8)+(c[9+69+69+10]<<16)+(c[9+69+69+11]<<24);// 下行接收频率/10 uint32，低位在前高位在后
    now_sat->line2[69]=0;
    now_sat->line1[69]=0;
    now_sat->name[9]=0;
}

uint8_t tle_check()
{

    sate_info.num=0;
    sat_parm now_sat;
    int cnt_num=0;
    for(int i=0; i < 45; i++)
    {
        cnt_num=i;

        Read_TLE(i,&now_sat);

        tle_data data;
        uint8_t judge=tle_parse( now_sat.name, now_sat.line1, now_sat.line2, &data);

        if(now_sat.TX_TONE>2541 || now_sat.RX_TONE>2541 || now_sat.RX_FREQ==0||now_sat.TX_FREQ>50000000 || now_sat.RX_FREQ>50000000)
            judge=0;
        if(judge) {
            sate_info.list[sate_info.num]=i;
            sate_info.num++;
        }
    }
    return sate_info.num;
}

void SELECT_DISPLAY() {

    char str[22] = {0};
    for (int i = 0; i < NUM_PER_PAGE; ++i) {
        uint8_t show_num = ((now_menu - 1) / NUM_PER_PAGE) * NUM_PER_PAGE + 1 + i;
        if (show_num > sate_info.num)
            break;
        char str1[14];
        EEPROM_ReadBuffer(0x1e200 + 160 * (sate_info.list[show_num- 1] ), str1, 9);
        str1[9] = 0;
        sprintf(str, "%02d.%s", show_num, str1);
        UI_PrintStringSmall(str, 8, 0, i);
    }
    if ((now_menu - 1) / NUM_PER_PAGE * NUM_PER_PAGE + 1 < sate_info.num) memcpy(&gStatusLine[110], BITMAP_ARRAY_DOWN, 5);
    if ((now_menu - 1) / NUM_PER_PAGE) memcpy(&gStatusLine[102], BITMAP_ARRAY_UP, 5);
    UI_PrintStringSmallBuffer("LoseHu DSF 0.1", gStatusLine);
    memcpy(&gFrameBuffer[(now_menu - 1) % NUM_PER_PAGE][0], BITMAP_ARRAY_RIGHT, sizeof(BITMAP_ARRAY_RIGHT));
    memcpy(&gFrameBuffer[(now_menu - 1) % NUM_PER_PAGE][120], BITMAP_ARRAY_LEFT, sizeof(BITMAP_ARRAY_LEFT));
    sprintf(str, "%02d/%02d", now_menu, sate_info.num);
    UI_PrintStringSmall(str, 0, 0, 6);

}

static void UpdateDBMax(bool inc) {
    uint8_t tmp = 10;


    if (inc && settings.dbMax < 10) {
        settings.dbMax += 1;
    } else if (!inc && settings.dbMax > tmp + settings.dbMin) {
        settings.dbMax -= 1;
    } else {
        return;
    }

    ClampRssiTriggerLevel();

    SYSTEM_DelayMs(20);
}

void SATE_DISPLAY()
{
    char String[40];
    uint8_t DATA_LINE=26;

    sprintf(String, "alt:%4d.%02d",(int)trunc(look.altitude),(abs((int)(look.altitude*100)))%100 );
    if (look.altitude<0&&look.altitude>-1&&(abs((int)(look.altitude*100)))%100!=0) String[6]='-';
    GUI_DisplaySmallest(String, 82, DATA_LINE + 15, false, true);

    sprintf(String, "%d km",(int)sub_point.distance );
    GUI_DisplaySmallest(String, 82+((127-82)-strlen(String)*4)/2, DATA_LINE + 23, false, true);
    char SHOW_TIME[6];
    memcpy(SHOW_TIME,my_time,6);
    if( ENABLE_BTC)
        utcToBeijingTime(SHOW_TIME) ;

    sprintf(String, "20%02d-%02d-%02d %02d:%02d:%02d ", SHOW_TIME[0], SHOW_TIME[1], SHOW_TIME[2], SHOW_TIME[3], SHOW_TIME[4], SHOW_TIME[5]);
    if( ENABLE_BTC)String[19]='B';
    else String[19]='U';
    String[20]=0;

    GUI_DisplaySmallest(String, 1, DATA_LINE + 23, false, true);







    // DrawF(fMeasure);//绘制频率
    uint8_t METER_PAD_LEFT = 3;
    uint8_t P_WIDTH = 120;
    uint8_t S_LINE = 25;
    uint8_t S_X = 4;
    uint8_t DBM_X = 22;
    P_WIDTH = 50;
    METER_PAD_LEFT = 70;
    S_LINE = 18;
    S_X = 58;
    DBM_X = 6;
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

    if ((monitorMode || IsPeakOverLevel()) ) {
        memset(gFrameBuffer[2] + DBM_X - 2, 0b11111110, 51);
        fill = false;
    }

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
    uint8_t SHOW_LINE=4;
    SHOW_LINE = 3;
    for (int i = 0, idx = 1; idx <= 4; ++i, ++idx) {

        offset = PAD_LEFT + i * CELL_WIDTH;
        if (menuState == idx) {
            for (int j = 0; j < CELL_WIDTH; ++j) {
                gFrameBuffer[SHOW_LINE][j + offset] = 0xFF;
                gFrameBuffer[SHOW_LINE + 1][j + offset] = 0xFF;
            }
        }
        DATA_LINE = row * 8 + 2;
        DATA_LINE -= 8;

        sprintf(String, "%s", registerSpecs[idx].name);
        GUI_DisplaySmallest(String, offset + 2, DATA_LINE, false,
                            menuState != idx);
        sprintf(String, "%u", GetRegMenuValue(idx));
        GUI_DisplaySmallest(String, offset + 2, DATA_LINE + 7, false,
                            menuState != idx);
    }





    bool flag = true;
    if (!isTransmitting) {
        sprintf(String, "%03u.%05u", sat_get.DOWN_LINK / 100000, sat_get.DOWN_LINK % 100000);
        UI_DisplayFrequency(String, 8, 0, false);
        sprintf(String, "UPLink:%4d.%05d", sat_get.UP_LINK / 100000, sat_get.UP_LINK % 100000);
    } else {
        sprintf(String, "%03u.%05u", sat_get.UP_LINK / 100000, sat_get.UP_LINK % 100000);
        UI_DisplayFrequency(String, 8, 0, false);
        memset(gFrameBuffer[5], 0x7f, 77);
        flag = false;
        sprintf(String, "DownLink:%4d.%05d", sat_get.DOWN_LINK / 100000,sat_get.DOWN_LINK % 100000);
    }
    GUI_DisplaySmallest(String, 1, DATA_LINE + 15, false, flag);

    sprintf(String, "%d/%d", settings.dbMin, settings.dbMax);

    //UI绘制状态栏
    memset(gStatusLine, 0x7f, 39);
    GUI_DisplaySmallest(sat_get.name, 2, 1, true, false);
    GUI_DisplaySmallest(String, 42 + (settings.dbMax > -100 ? 4 : 0), 1, true, true);

    sprintf(String, "%3s", gModulationStr[settings.modulationType]);
    GUI_DisplaySmallest(String, 42 + 38, 1, true, true);

    sprintf(String, "%s", bwOptions[settings.listenBw]);
    GUI_DisplaySmallest(String, 42 + 53 - (settings.listenBw == 0 ? 8 : 0), 1, true, true);


    // DrawPower();

    //頻率


}
void   PIC_DISPLAY()
{
    char String[20];
    drawcircle(40,32,39);
    drawcircle(40,32,26);
    drawcircle(40,32,13);
    float alt=look.altitude;
    float ano= look.azimuth;
    float r=(90-fabs(alt))*39/90;

    float center_x=40+(r*sin(ano*3.141592653589/180));
    float center_y=32-(r*cos(ano*3.141592653589/180)*2/3);
    fill_circle((uint8_t)round(center_x),(uint8_t)round(center_y));
    uint8_t start_text=85;
    uint8_t start_num=78;

    UI_PrintStringSmall("alt:", start_text, 0, 0);
    sprintf(String, "%4d.%02d", (int)trunc(look.altitude),abs((int)(look.altitude*100))%100);
    if (look.altitude<0&&look.altitude>-1&&(abs((int)(look.altitude*100)))%100!=0) String[2]='-';
    UI_PrintStringSmall(String, start_num, 0, 1);

    UI_PrintStringSmall("azi:", start_text, 0, 2);
    sprintf(String, "%4d.%02d", (int)trunc(look.azimuth),abs((int)(look.azimuth*100))%100);
    if (look.azimuth<0&&look.azimuth>-1&&abs((int)(look.azimuth*100))%100!=0) String[2]='-';
    UI_PrintStringSmall(String, start_num, 0, 3);

    UI_PrintStringSmall("dis:", start_text, 0, 4);
    sprintf(String, "%dkm",(int)sub_point.distance );
    UI_PrintStringSmall(String, start_num, 0, 5);

}



// 计算整数的位数
int countIntegerDigits(int num) {
    if (num == 0) return 1; // 0 的位数为 1
    int count = 0;
    while (num != 0) {
        num /= 10;
        count++;
    }
    return count;
}
void Write_LOCAL()
{

    if(observer.lat>=-90&&observer.lat<=90)        EEPROM_WriteBuffer(0x2c00, (uint8_t *)&observer.lat,8);
    if(observer.lon>=-180&&observer.lon<=180)           EEPROM_WriteBuffer(0x2c00+8,(uint8_t *)&observer.lon,8);
    if(observer.height>=-999999&& observer.height<=999999)              EEPROM_WriteBuffer(0x2c00+16,(uint8_t *)&observer.height,8);
}
void Read_LOCAL()
{
    uint8_t byte_array[24];

    EEPROM_ReadBuffer(0x2c00, (uint8_t *)&observer.lat,8);
    EEPROM_ReadBuffer(0x2c00+8,(uint8_t *)&observer.lon,8);
    EEPROM_ReadBuffer(0x2c00+16,(uint8_t *)&observer.height,8);

    if(!(observer.lat>=-90&&observer.lat<=90))  observer.lat=85;
    if(!(observer.lon>=-180&&observer.lon<=180))observer.lon=130;
    if(!(observer.height>=-999999&& observer.height<=999999))  observer.height=20;



}
// 将浮点数转换为字符串
void floatToString(double num, char *str) {
    int isNegative = 0; // 标记是否为负数
    if (num < 0) {
        isNegative = 1;
        num = -num; // 转换为正数处理
    }

    int integerPart = (int)num;  // 获取整数部分
    double fractionalPart = num - integerPart;  // 获取小数部分

    // 计算整数部分的位数
    int integerDigits = countIntegerDigits(integerPart);

    // 计算小数部分的精度
    int fractionalPrecision;
    if (integerPart == 0) {
        fractionalPrecision = 15; // 如果整数部分为 0，保留 16 位小数
    } else {
        fractionalPrecision = 15 - integerDigits; // 否则保留 16 - n 位小数
        if (fractionalPrecision < 0) fractionalPrecision = 0; // 确保精度不为负数
    }

    // 处理负数符号
    int i = 0;
    if (isNegative) {
        str[i++] = '-';
    }

    // 处理整数部分
    if (integerPart == 0) {
        str[i++] = '0';
    } else {
        int temp = integerPart;
        while (temp != 0) {
            str[i++] = (temp % 10) + '0';
            temp /= 10;
        }
    }

    // 反转整数部分
    int start = isNegative ? 1 : 0; // 如果有负号，反转从下标 1 开始
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }

    // 添加小数点
    str[i++] = '.';

    // 处理小数部分
    for (int j = 0; j < fractionalPrecision; j++) {
        fractionalPart *= 10;
        int digit = (int)fractionalPart;
        str[i++] = digit + '0';
        fractionalPart -= digit;
    }

    // 添加字符串结束符
    str[i] = '\0';
}
char local_string[20];
uint8_t local_string_index=0;
double lat_set,lon_set,alt_set;

void LOCAL_DISPLAY()
{
    char str1[25];

    UI_PrintStringSmallBuffer("Location", gStatusLine);

    UI_PrintStringSmall("Lat(deg):", 5, 0, 0);
    UI_PrintStringSmall("Lon(deg):", 5, 0, 2);
    UI_PrintStringSmall("Hight(m):", 5, 0, 4);
// if(!local_inflag){
    floatToString(observer.lat,str1);
    UI_PrintStringSmall(str1, 0, 0, 1);
    floatToString(observer.lon,str1);
    UI_PrintStringSmall(str1, 0, 0, 3);
    floatToString(observer.height,str1);
    UI_PrintStringSmall(str1, 0, 0, 5);
    if(local_inflag)
    {   memset(gFrameBuffer[local_selected*2-1],0,128);
        UI_PrintStringSmall(local_string, 0, 0, local_selected*2-1);
        UI_PrintStringSmall("INPUT", 10, 0, 6);

    }
    if(local_inflag)
        memcpy(&gFrameBuffer[ local_selected*2-2][70], BITMAP_ARRAY_RIGHT, sizeof(BITMAP_ARRAY_RIGHT));
    else
        memcpy(&gFrameBuffer[ local_selected*2-2][70], BITMAP_ARRAY_EMPTY_RIGHT, sizeof(BITMAP_ARRAY_RIGHT));

    sprintf(str1,"%02d/03",local_selected);
    UI_PrintStringSmall(str1, 90, 0, 6);

}
void TLE_UI()
{
    memset(gFrameBuffer, 0, sizeof(gFrameBuffer));
    memset(gStatusLine, 0, sizeof(gStatusLine));
//     if(mode==SELECT||mode==SATE
// #ifdef ENABLE_PIC
//             || mode==PIC
// #endif
//       )
    DrawPower();

    switch (mode)
    {
#ifdef ENABLE_PIC
    case PIC:
        PIC_DISPLAY();
        break;
#endif
    case SELECT:
        SELECT_DISPLAY();
        break;
    case SATE:
        SATE_DISPLAY();
        break;
    case TIME: {

        char stringshowed[20]="20xx-xx-xx";
        char show2[20]="xx:xx:xx";
        uint8_t table[12]= {2,3,5,6,8,9,0,1,3,4,6,7};
        for (uint8_t i = 0; i < 6; i++)
        {
            if(freqInputString[i]!='-'&&freqInputString[i]!=0)stringshowed[table[i]]=freqInputString[i];
            if(freqInputString[i+6]!='-'&&freqInputString[i+6]!=0)show2[table[i+6]]=freqInputString[i+6];

        }

        UI_PrintStringSmallBuffer("TimeSet", gStatusLine);

        UI_PrintStringSmall(stringshowed, 0, 127, 1);
        UI_PrintStringSmall(show2, 0, 127, 3);
        if(ENABLE_BTC) UI_PrintStringSmall("BTC", 5, 0, 6);
        else UI_PrintStringSmall("UTC", 5, 0, 6);
        break;
    }
    case LOCAL:
        LOCAL_DISPLAY();
        break;

    }
    ST7565_BlitFullScreen();
    ST7565_BlitStatusLine();
}
void LocalCheckSave()
{
    char *endptr;
    double num1=0;
    uint8_t flag=0;
    strd(local_string,&num1);
    if(local_selected==1&&num1>=-90&&num1<=90) observer.lat=num1,flag=1;
    else if(local_selected==2&&num1>=-180&&num1<=180)  observer.lon=num1,flag=1;
    else if(num1>=-999999&&num1<=999999)  observer.height=num1,flag=1;
    if(flag)    Write_LOCAL();


}
void INIT_LOCALINPUT()
{
    local_inflag=1;
    local_dot_flag=0;
    local_string_index=0;
    memset(local_string,0,sizeof(local_string));
}
void SELECT_KEY()
{
    switch (my_kbd.current)
    {
    // case KEY_5:
    //     before_mode=mode;
    //     mode=TIME;
    //     freqInputIndex = 0;
    //     freqInputDotIndex = 0;
    //     ResetFreqInput();
    //     break;
    case KEY_UP:
        now_menu-=1;
        if(now_menu<1 ) now_menu=sate_info.num;
        break;
    case KEY_DOWN:
        now_menu+=1;
        if(now_menu>sate_info.num ) now_menu=1;
        break;
    case KEY_MENU:
        switch_mode=1;
        break;
    }
}
void SATE_KEY()
{
    switch (my_kbd.current)
    {
    // case KEY_5:
    //     before_mode=mode;
    //     mode=TIME;
    //     freqInputIndex = 0;
    //     freqInputDotIndex = 0;
    //     ResetFreqInput();
    //     break;
    case KEY_0:
        settings.listenBw+=1 ;
        if(settings.listenBw==3)settings.listenBw=0;
//    BK4819_FILTER_BW_WIDE
        BK4819_SetFilterBandwidth(settings.listenBw, false);

        break;

    case KEY_1:
#ifdef ENABLE_PIC
        if(mode==PIC)  mode=SATE;
        else
#endif
            mode=PIC;
        break;
    case KEY_EXIT:
#ifdef ENABLE_PIC
        if(mode==PIC)
        {
            mode=SATE;
            break;
        }
#endif
        if(menuState)menuState=0;
        else {
            if(isTransmitting)
                ToggleTX(false);
            if(monitorMode||IsPeakOverLevel())
                ToggleRX(false);
            isTransmitting=false;
            monitorMode = false;
            lockAGC = false;
            //退出應用，進入選擇頁面
            mode=SELECT;
            // DeInitSpectrum();

        }
        break;
    case KEY_STAR:
        UpdateRssiTriggerLevel(true);
        break;
    case KEY_F:
        UpdateRssiTriggerLevel(false);
        break;
    // case KEY_3:
    //     UpdateDBMax(true);
    //     break;
    // case KEY_9:
    //     UpdateDBMax(false);
    //     break;

    case KEY_UP:
        if (menuState)
            SetRegMenuValue(menuState, true);

        break;
    case KEY_DOWN:
        if (menuState)
            SetRegMenuValue(menuState, false);

        break;
    case KEY_PTT:
        if(sat_get.TX_FREQ>1800000)
            ToggleTX(true);
        break;

    case KEY_MENU:
        if (menuState == ARRAY_SIZE(registerSpecs) - 1) {
            menuState = 1;
        } else {
            menuState++;
        }
        break;
    case KEY_SIDE1:
        if(!isTransmitting)
            monitorMode = !monitorMode;
        break;
    }
}
void TIME_KEY()
{
    switch (my_kbd.current)
    {
    case KEY_0...KEY_9:
        UpdateFreqInput(my_kbd.current);
        if(freqInputIndex==12) {
            char SHOW_TIME[6];
            SHOW_TIME[0]=10*(freqInputString[0]-'0')+(freqInputString[1]-'0');
            SHOW_TIME[1]=10*(freqInputString[2]-'0')+(freqInputString[3]-'0');
            SHOW_TIME[2]=10*(freqInputString[4]-'0')+(freqInputString[5]-'0');
            SHOW_TIME[3]=10*(freqInputString[6]-'0')+(freqInputString[7]-'0');
            SHOW_TIME[4]=10*(freqInputString[8]-'0')+(freqInputString[9]-'0');
            SHOW_TIME[5]=10*(freqInputString[10]-'0')+(freqInputString[11]-'0');
            if( ENABLE_BTC)
                beijingToUtcTime(SHOW_TIME) ;
            memcpy(my_time,SHOW_TIME,6);
            EEPROM_WriteBuffer(0x02BA0,my_time,6);

            RTC_Set();

            mode=before_mode;
        }
        break;
    case KEY_EXIT:
        if (freqInputIndex == 0) {
            mode=before_mode;
            break;
        }
        UpdateFreqInput(my_kbd.current);
        break;
        // case KEY_MENU:
        // if(freqInputIndex==10) {
        //     my_time[1]=10*(freqInputString[0]-'0')+(freqInputString[1]-'0');
        //     my_time[2]=10*(freqInputString[2]-'0')+(freqInputString[3]-'0');
        //     my_time[3]=10*(freqInputString[4]-'0')+(freqInputString[5]-'0');
        //     my_time[4]=10*(freqInputString[6]-'0')+(freqInputString[7]-'0');
        //     my_time[5]=10*(freqInputString[8]-'0')+(freqInputString[9]-'0');
        //     RTC_Set();
        // }
        // mode=before_mode;
        // break;
    }
}
void LOCAL_KEY()
{
    switch (my_kbd.current)
    {
    case KEY_UP:
    case KEY_DOWN:
        if(local_inflag==0) {

            local_selected+=my_kbd.current==KEY_UP?-1:1;
            if(local_selected==4)local_selected=1;
            if(local_selected==0)local_selected=3;
        }
        break;
    case KEY_0...KEY_9:
    case KEY_STAR:
    case KEY_F:
        if(!local_inflag)
        {
            INIT_LOCALINPUT();
        }
        if(local_string_index<18)//没满
        {
            if(local_string_index==0&&my_kbd.current!=KEY_STAR) {
                if(my_kbd.current==KEY_F) local_string[local_string_index]='-';
                else local_string[local_string_index]='0'+my_kbd.current;
            } else if(my_kbd.current==KEY_STAR&&local_dot_flag==0&&local_string_index!=0) {
                local_string[local_string_index]='.';
                local_dot_flag=1;
            } else if(my_kbd.current>=KEY_0&&my_kbd.current<=KEY_9) {
                local_string[local_string_index]='0'+my_kbd.current;

            } else break;
            local_string_index++;

        } else {
            //检查并保存
            LocalCheckSave();
            local_inflag=0;
        }
        break;
    case KEY_MENU:
        if(local_inflag)
        {
            //检查并保存
            LocalCheckSave();
            local_inflag=0;
        } else {

            INIT_LOCALINPUT();

        }
        // mode=before_mode;
        break;
    case KEY_EXIT:
        if(local_inflag&&local_string_index)
        {
            if(local_string_index)
            {
                local_string_index--;
                local_string[local_string_index]=0;
            } else {
                local_inflag=0;
            }
        } else if(local_inflag) {
            local_inflag=0;
        } else {
            mode=before_mode;
        }
        break;
    }
}
void TLE_KEY()
{
    if(my_kbd.current==KEY_SIDE2) {
        if(BACK_ON)
            PWM_PLUS0_CH0_COMP = 0<<2;
        else         PWM_PLUS0_CH0_COMP = 255<<2;
        BACK_ON=1-BACK_ON;
    }
    if(mode==SATE||mode==SELECT
#ifdef ENABLE_PIC
            ||  mode==PIC
#endif
      ) {
        switch (my_kbd.current)
        {
        case KEY_5:
            before_mode=mode;
            mode=TIME;
            freqInputIndex = 0;
            freqInputDotIndex = 0;
            ResetFreqInput();
            return;
        case KEY_4:
            before_mode=mode;
            mode=LOCAL;
            INIT_LOCALINPUT();
            local_inflag=0;
            return;
        case KEY_8:
            ENABLE_BTC=1-ENABLE_BTC;
            return;
        }

    }
    switch (mode) {
    case SELECT:
        SELECT_KEY();
        break;
#ifdef ENABLE_PIC
    case PIC:
#endif
    case SATE:
        SATE_KEY();
        break;
    case TIME:
        TIME_KEY();
        break;
    case LOCAL:
        LOCAL_KEY();
        break;
    }
}


void ERROR_DISPLAY() {
    memset(gFrameBuffer, 0, sizeof(gFrameBuffer));
    UI_PrintStringSmall("NO Satellite !!", 0, 127, 1);
    UI_PrintStringSmall("Write in:", 0, 127, 3);
    UI_PrintStringSmall("k5.vicicode.com", 0, 127, 4);
    UI_PrintStringSmall("WRITE OR EXIT", 0, 127, 6);

    ST7565_BlitFullScreen();
    while (1) {
        if (KEYBOARD_Poll() == KEY_EXIT) {
            while (KEYBOARD_Poll() == KEY_EXIT);
            return;
        }
        UART_READ();
    }
}


void TLE_PROCESS()
{
    if(mode==SELECT)
    {
        if(switch_mode==1)
        {
            switch_mode=0;
            Read_TLE(sate_info.list[ now_menu-1],&sat_get);

            mode=SATE;
            DOPPLER_FLASH=1;
        }

    }
    if(mode==SATE
#ifdef ENABLE_PIC
            || mode==PIC
#endif
      ) {
        if (  my_kbd.current == KEY_INVALID&&isTransmitting) {
            ToggleTX(false);
        }


        if (isListening ) {
            UpdateListening();
        } else {
            UpdateStill();
        }

        // RTC_Get();
        // if()
        if(DOPPLER_FLASH)
        {
            DOPPLER_FLASH=0;
            cal_sat(sat_get.name,sat_get.line1,sat_get.line2,observer,&look,&sub_point,my_time);
            if(sat_get.TX_FREQ>1800000)
                sat_get.UP_LINK= doppler_shift(sat_get.TX_FREQ, sub_point.sate_eci, sub_point.r_dot, look.observer_eci,10);

            sat_get.DOWN_LINK= doppler_shift(sat_get.RX_FREQ, sub_point.sate_eci, sub_point.r_dot, look.observer_eci,0);
            if (!isTransmitting&&currentFreq!=sat_get.RX_FREQ) {
                SetF(sat_get.DOWN_LINK);
                currentFreq = sat_get.DOWN_LINK;
            }
        }
    }
}

void TLE_Main()
{
    Read_LOCAL();
// while(1);
    isListening = true; // to turn off RX later
    // ToggleRX(true),
    ToggleRX(false); // hack to prevent noise when squelch off

    RADIO_SetModulation(BK4819_AF_FM);
    BK4819_SetFilterBandwidth(settings.listenBw = BK4819_FILTER_BW_WIDE, false);
    // RelaunchScan();
    // settings.listenBw = 0;
    // settings.modulationType = MODULATION_FM;
    // TuneToPeak();
    sat_get.DOWN_LINK= sat_get.RX_FREQ ;
    sat_get.UP_LINK= sat_get.TX_FREQ ;

    currentFreq= sat_get.DOWN_LINK;
    EEPROM_ReadBuffer(0x02BA0,my_time,6);
    // settings.dbMin = -130;
    // uint8_t INIT_TIME[6]= {25,1,27,8,28,42};
    // memcpy(my_time,INIT_TIME,6);
    RTC_Set();

    if(tle_check()==0)
    {


        if(sate_info.num>45 || sate_info.num==0) {
            ERROR_DISPLAY();
            NVIC_SystemReset();
        }
    }

    while(1)
    {
        TLE_PROCESS();
        TLE_UI();
        UART_READ();
        SYSTEM_DelayMs(10);

    }
}

void RTCHandler(void) {

    RTC_Get();
    DOPPLER_FLASH=1;


    RTC_IF |= (1 << 5);//清除中断标志位

}