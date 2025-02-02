#include <math.h>
#include "eci.h"
#include "vec.h"
#include "sgp.h"
#include "util.h"
#include "tle.h"


const double f = 0.003352810665;
const double esq = 0.006694379991;
const double a = 6378.137;

vec lat_lon_to_eci(double lat, gmst theta,double h) {
    double C = 1 / sqrt(1 + f * (f - 2) * square(sin(lat)));
    double S = square(1 - f) * C;
    double r = a * C + h / 1000.0;  // 调整半径，考虑高度

    double x = r * cos(lat) * cos(theta);
    double y = r * cos(lat) * sin(theta);
    double z = (a * S + h / 1000.0) * sin(lat);

    vec result = {x, y, z};
    return result;
}

int tle_to_eci(sgp_result *result, tle_data *tle, jd time) {
    // Convert TLE epoch into jd so we can figure out the
    // minutes elapsed since the epoch until the target
    // time
    long epoch_yr;
    if (strl(tle->epoch_yr, &epoch_yr) == 0) {
        return 0;
    }
    static const int current_millenium = 2000;
    double tle_jd = jd_year_begin(current_millenium + epoch_yr) + tle->epoch_day;
    double minute_diff = (time - tle_jd) * 1440;
    if (minute_diff < 0) {
        return 0;
    }

    *result = sgp4(tle, minute_diff);
    return 1;
}

double clamp(double lat, double max) {
    double deg = to_degrees(lat);
    if (deg > max) {
        return -(max * 2 - deg);
    }

    if (deg < -max) {
        return deg + (max * 2);
    }

    return deg;
}

look_result eci_to_look(tle_data *tle, lat_lon observer, jd time) {
    double lat = to_radians(observer.lat);
    double lon = to_radians(observer.lon);
    gmst gmst = to_gmst(time);
    double theta = fmod(gmst + lon, 2 * M_PI);

    vec observer_eci = lat_lon_to_eci(lat, theta,observer.height);
    sgp_result sgp_eci;
    if (tle_to_eci(&sgp_eci, tle, time) == 0) {
        look_result fail = { 0 };
        return fail;
    }

    double rx = sgp_eci.r.x - observer_eci.x;
    double ry = sgp_eci.r.y - observer_eci.y;
    double rz = sgp_eci.r.z - observer_eci.z;
    double top_s = sin(lat) * cos(theta) * rx
                   + sin(lat) * sin(theta) * ry
                   - cos(lat) * rz;
    double top_e = -sin(theta) * rx + cos(theta) * ry;
    double top_z = cos(lat) * cos(theta) * rx
                   + cos(lat) * sin(theta) * ry
                   + sin(lat) * rz;
    double az = atan(-top_e / top_s);
    if (top_s > 0) {
        az = az + M_PI;
    }
    if (az < 0) {
        az = az + 2 * M_PI;
    }
    double rg = sqrt(rx * rx + ry * ry + rz * rz);
    double el = asin(top_z / rg);

    look_result result = {to_degrees(az), to_degrees(el), rg, observer_eci};
    return result;
}

lat_lon eci_to_lat_lon(tle_data *tle, gmst time) {
    gmst gmst = to_gmst(time);
    double theta = fmod(gmst, 2 * M_PI);

    sgp_result sgp_eci;
    if (tle_to_eci(&sgp_eci, tle, time) == 0) {
        lat_lon fail = { 0 };
        return fail;
    }

    double r = sqrt(square(sgp_eci.r.x) + square(sgp_eci.r.y));
    int rounds = 10;
    double dphi;
    double phi = atan2(sgp_eci.r.z, r);
    do {
        double phi_i = phi;
        double C = 1 / sqrt(1 - esq * square(sin(phi_i)));
        phi = atan2(sgp_eci.r.z + a * C * esq * sin(phi_i), r);
        dphi = fabs(phi - phi_i);
    } while (dphi > 1E-6 && --rounds > 0);
    double lon = atan2(sgp_eci.r.y, sgp_eci.r.x) - theta;

    lat_lon result = { clamp(phi, 90), clamp(lon, 180),0,sgp_eci.r ,0,sgp_eci.r_dot};
    return result;
}
