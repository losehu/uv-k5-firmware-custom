#ifndef FATE_ECI_H
#define FATE_ECI_H
#include "astrotime.h"
#include <string.h>
#include "tle.h"



/**
 * Calculates the look position of the satellite from its
 * SGP data, the observer position, and the time.
 *
 * @param tle the TLE data that will be used
 * @param observer the observer location on the Earth
 * @param time the time which to determine the look
 * location
 * @return the look location expressed as an azimuth and
 * altitude
 */
look_result eci_to_look(tle_data *tle, lat_lon observer, gmst time);

/**
 * Calculates the sub-point (i.e. the point below the
 * satellite formed by a line from the satellite
 * intersecting at a right angle with a line tangent to the
 * surface of the Earth) from the given SGP data and time.
 *
 * @param tle the TLE data that will be used
 * @param time the time which to calculate the satellite
 * position on the Earth
 * @return a set of latitude and longitude coordinates
 */
lat_lon eci_to_lat_lon(tle_data *tle, gmst time);

#endif /* FATE_ECI_H */
