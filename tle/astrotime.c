#include "astrotime.h"
#include "util.h"

jd jd_year_begin(int year) {
    year -= 1;
    long a = year / 100;
    long b = 2 - a + (a / 4);
    return (long) (365.25 * year) + (long) (30.6001 * 14) + 1720994.5 + b;
}

jd to_jd(int year, int month, int day, int hour, int minute, int second) {
    double jd_start_day = jd_year_begin(year);

    static const int day_accum[2][13] = {
            {0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
            {0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}
    };
    int leap = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
    int jd_day = day_accum[leap][month] + day;

    static const int seconds_per_day = 24 * 3600;
    int total_offset_seconds = hour * 3600 + minute * 60 + second;
    double jd_off = (double) total_offset_seconds / seconds_per_day;

    return jd_start_day + jd_day + jd_off;
}

gmst to_gmst(jd jd) {
    double time = frac(jd + 0.5);
    jd = jd - time;
    double rel_date = (jd - 2451545.0) / 36525;
    double gmst = 24110.54841 + rel_date * (8640184.812866 + rel_date * (0.093104 - rel_date * 6.2E-6));
    gmst = fmod(gmst + 86400.0 * 1.00273790934 * time, 86400.0);
    return 2 * M_PI * gmst / 86400.0;
}
