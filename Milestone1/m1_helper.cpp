//
// Other functions.cpp
//Created by Frank on 2021/2/5
//
//Purpose of the program:
//

#include <stdio.h>
#include <math.h>
#include <utility>


#include "StreetsDatabaseAPI.h"
#include "m1.h"
#include "LatLon.h"

//Naming Convention: use snake style for helper functions


bool check_duplicate(std::vector<IntersectionIdx> adjacentIntersections, IntersectionIdx intersection_id){
    bool no_duplicate = true;
    for(int i = 0; i < adjacentIntersections.size(); i++){
        if(adjacentIntersections[i] == intersection_id)
            return false;
    }
    return no_duplicate;
}
