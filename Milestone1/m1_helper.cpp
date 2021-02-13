//
// Other functions.cpp
//Created by Frank on 2021/2/5
//
//Purpose of the program:
//

#include <stdio.h>
#include <math.h>
#include <utility>
#include <string>
using std::string;


#include "StreetsDatabaseAPI.h"
#include "m1.h"
#include "LatLon.h"

//Naming Convention: use snake style for helper functions

bool check_duplicate(std::vector<IntersectionIdx> adjacentIntersections, IntersectionIdx intersection_id) {
    bool no_duplicate = true;
    for(int i = 0; i < adjacentIntersections.size(); i++) {
        if(adjacentIntersections[i] == intersection_id)
            return false;
    }
    return no_duplicate;
}

// Remove all spaces in a string and convert all lower case letters to upper case
std::string modify_name(std::string name) {
    int len = name.length();
    // Lower case to upper case
    for(int i = 0; i < len; i++) {
        if(name[i] >= 'a' && name[i] <= 'z') {
            name[i] -= 32;
        }
    }
    // Remove space
    for(int i = 0; i < len; i++) {
        if(name[i] == ' ') {
            name.erase(i, 1);
            i--;
        }
    }
    return name;
}

// converts character array to string and returns it
string convert_to_string(char* a, int size)
{
    int i;
    string s = "";
    for (i = 0; i < size; i++) {
        s = s + a[i];
    }
    return s;
}

double compare_max_lat(double maxLat, LatLon point) {
    if (point.latitude() > maxLat) return point.latitude();
    else return maxLat;
}

double compare_min_lat(double minLat, LatLon point) {
    if (point.latitude() < minLat) return point.latitude();
    else return minLat;
}

double compare_max_lon(double maxLon, LatLon point) {
    if (point.longitude() > maxLon) return point.longitude();
    else return maxLon;
}

double compare_min_lon(double minLon, LatLon point) {
    if (point.longitude() < minLon) return point.longitude();
    else return minLon;
}
