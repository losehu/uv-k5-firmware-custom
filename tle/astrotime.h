#ifndef FATE_ASTROTIME_H
#define FATE_ASTROTIME_H

/**
 * Julian Date represented as a double value
 */
typedef double jd;

/**
 * GMST represented as a radian measure in a double
 */
typedef double gmst;

/**
 * Obtains the Julian Date at the beginning of the given
 * year.
 *
 * @param year the year which to find the julian date
 */
jd jd_year_begin(int year);

/**
 * Converts the given Gregorian time measurements (i.e.
 * standard calendar measurements) into Julian Days.
 *
 * @param year the year which to convert
 * @param month the month
 * @param day the day
 * @param hour the hour in UTC
 * @param minute the minute in UTC
 * @param second the second in UTC
 * @return the Julian Date
 */
jd to_jd(int year, int month, int day, int hour, int minute, int second);

/**
 * Converts the given Julian Date into GMST.
 *
 * @param jd the Julian Date
 * @return the GMST time, in radians
 */
gmst to_gmst(jd jd);
#include <string.h>

#endif /* FATE_ASTROTIME_H */
