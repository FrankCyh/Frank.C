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
#include <math.h>
using std::string;


bool check_duplicate(std::vector<IntersectionIdx> adjacentIntersections, IntersectionIdx intersection_id);

string modify_name(std::string name);

string convert_to_string(char* a, int size);

void update_bounding_box(double & maxLat, double & minLat, double & maxLon, double & minLon, LatLon point);

void ordinal_number(int i);

#endif /* m1_helper_h */
