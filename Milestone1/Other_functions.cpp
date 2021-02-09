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

std::vector<double> break_streetSegments_into_smallSegment(StreetSegmentIdx street_segment_id) {
    std::vector<double> small_segment;
//     Q: necessary to incorporate into the global variable? s
    /*
    typedef int SmallSegmentIdx;
    std::vector<std::vector<std::vector<SmallSegmentIdx>>> intersection_street_segments;
    */
    int numCurvePoint = getStreetSegmentInfo(street_segment_id).numCurvePoints;
    for(IntersectionIdx i = -1; i < numCurvePoint; i++) {
        // find the IntersectionIdx of the from and to
        // Total endpoints = numCurvePoint + 2
        // Total segment = numCurvePoint + 1 (number of loop needed)
        LatLon seg_From, seg_To;
        if(i == -1)
            seg_From = getIntersectionPosition(getStreetSegmentInfo(street_segment_id).from);
        else
            seg_From = getStreetSegmentCurvePoint(street_segment_id, i);

        if(i == numCurvePoint - 1)
            seg_To = getIntersectionPosition(getStreetSegmentInfo(street_segment_id).to);
        else
            seg_To = getStreetSegmentCurvePoint(street_segment_id, i + 1);
        // seg_From = seg_To? no operator =

        // Create a pair, add each small segment
        std::pair<LatLon, LatLon> pair_of_intersection_id(seg_From, seg_To);
        small_segment.push_back(findDistanceBetweenTwoPoints(pair_of_intersection_id));
    }
    return small_segment;
}

bool check_duplicate(std::vector<IntersectionIdx> adjacentIntersections, IntersectionIdx intersection_id){
    bool no_duplicate = true;
    for(int i = 0; i < adjacentIntersections.size(); i++){
        if(adjacentIntersections[i] == intersection_id)
            return false;
    }
    return no_duplicate;
}
