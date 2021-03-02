//
// m1_helper.h
//Created by Frank on 2021/2/23
//
//Purpose of the program:
//

#ifndef m1_helper_h
#define m1_helper_h

#include "LatLon.h"
#include "m1.h"
#include "globals.h"

#include <string>
using std::string;


bool check_duplicate(std::vector<IntersectionIdx> adjacentIntersections, IntersectionIdx intersection_id);

string modify_name(std::string name);

string convert_to_string(char* a, int size);

double compare_max_lat(double maxLat, LatLon point);

double compare_min_lat(double minLat, LatLon point);

double compare_max_lon(double maxLon, LatLon point);

double compare_min_lon(double minLon, LatLon point);

#endif /* m1_helper_h */
