//
// globals.cpp
//Created by Frank on 2021/3/2
//
//Purpose of the program:
//

#include "globals.h"

/* Hierarchy
Level 1:
 Intersection (number = getNumIntersections())
 variable type: vector<StreetSegmentIdx>
    Level 2:
    StreetSegmentIdx (number = getNumIntersectionStreetSegment(intersection_id)
    variable type: StreetSegmentIdx
*/
vector<vector<StreetSegmentIdx>> intersection_streetSegments;

/* Hierarchy
Level 1:
 StreetSegmentIdx (number = getNumIntersections())
 variable type: vector<SmallSegmentsLength>
    Level 2:
    SmallSegmentLength (number = getStreetSegmentInfo(street_segment_id).numCurvePoints)
    variable type: SmallSegmentsLength
*/
vector<vector<SmallSegmentsLength>> streetSegments_smallSegmentsLength;

/* Hierarchy
Level 1:
 streetSegmentIdx (number = getNumStreetSegments())
 variable type: double
 */
vector<double> segment_length;

/* Hierarchy
Level 1:
 streetSegmentIdx (number = getNumStreetSegments())
 variable type: double
 */
vector<double> segment_travel_time;

/* Hierarchy
Level 1:
 StreetIdx (number = getNumStreets())
 variable type: vector<StreetSegmentIdx>
    Level 2:
    StreetSegmentIdx (number UNKNOWN)
    variable type: StreetSegmentIdx
*/
vector<vector<StreetSegmentIdx>> street_streetSegments;

/* Hierarchy
Level 1:
 StreetIdx (number = getNumStreets())
 variable type: vector<IntersectionIdx>
    Level 2:
    IntersectionIdx (number UNKNOWN)
    variable type: IntersectionIdx
*/
vector<vector<IntersectionIdx>> street_intersections;

/* Hierarchy
Level 1:
 string (number = getNumStreets() * number of characters in each string name)
 variable type: pair<string, StreetIdx>
*/
multimap<string, StreetIdx> partial_street_names;

struct intersection_data {
    LatLon position;
    std::string name;
};

// streetIntersection -> streetSegment
void streetIntersection_to_streetSegment() {
    //Create empty vector for each intersection
    //Complexity: O(n)
    intersection_streetSegments.resize(getNumIntersections());

    //Iterate through all intersections
    //Complexity; O(k), where k is a small constant
    for(IntersectionIdx intersection = 0; intersection < getNumIntersections(); intersection++) {
        //Load streetSegmentId for each intersection
        for(int i = 0; i < getNumIntersectionStreetSegment(intersection); ++i) {
            StreetSegmentIdx ss_ids = getIntersectionStreetSegment(intersection, i);
            intersection_streetSegments[intersection].push_back(ss_ids);
        }
    }
    //Overall Complexity: O(kn)
}

// streetSegments -> smallSegments
void streetSegments_to_smallSegments() {
    streetSegments_smallSegmentsLength.resize(getNumStreetSegments());
    for(StreetSegmentIdx street_segment_id = 0; street_segment_id < getNumStreetSegments(); street_segment_id++) {
        int numCurvePoint = getStreetSegmentInfo(street_segment_id).numCurvePoints;
        //Load SmallSegmentLength for each StreetSegmentIdx
        for(IntersectionIdx intersection_id = -1; intersection_id < numCurvePoint; intersection_id++) {
            // find the IntersectionIdx of the from and to
            // Total endpoints = numCurvePoint + 2
            // Total segment = numCurvePoint + 1 (number of loop needed)
            LatLon seg_From, seg_To;
            if(intersection_id == -1)
                seg_From = getIntersectionPosition(getStreetSegmentInfo(street_segment_id).from);
            else
                seg_From = getStreetSegmentCurvePoint(street_segment_id, intersection_id);

            if(intersection_id == numCurvePoint - 1)
                seg_To = getIntersectionPosition(getStreetSegmentInfo(street_segment_id).to);
            else
                seg_To = getStreetSegmentCurvePoint(street_segment_id, intersection_id + 1);

            // Create a pair, add each small segment
            pair<LatLon, LatLon> pair_of_intersection_id(seg_From, seg_To);
            streetSegments_smallSegmentsLength[street_segment_id].push_back(findDistanceBetweenTwoPoints(pair_of_intersection_id));
            // push in total numCurvePoints + 1 items
        }
    }
}

// streetSegments -> segmentLength
void streetSegments_to_segmentLength() {
    segment_length.resize(getNumStreetSegments());
    for(int i = 0; i < getNumStreetSegments(); i++) {
        for(int j = 0; j < streetSegments_smallSegmentsLength[i].size(); j++)
            // add every small segment to calculate the length
            segment_length[i] += streetSegments_smallSegmentsLength[i][j];
    }
}

// streetSegments -> segmentTravelTime
void streetSegments_to_segmentTravelTime() {
    segment_travel_time.resize(getNumStreetSegments());
    for(int i = 0; i < getNumStreetSegments(); i++) {
        // travel time = segment_length[i] / speedLimit
        segment_travel_time[i] = segment_length[i] / getStreetSegmentInfo(i).speedLimit;
    }
}

// street -> streetSegments
void street_to_streetSegments() {
    street_streetSegments.resize(getNumStreets());
    for(StreetSegmentIdx street_segment_id = 0; street_segment_id < getNumStreetSegments(); street_segment_id++) {
        // Load each streetSegment and insert to the vector of corresponding streetID
        street_streetSegments[getStreetSegmentInfo(street_segment_id).streetID].push_back(street_segment_id);
    }
}


// street -> intersections
void street_to_intersections() {
    street_intersections.resize(getNumStreets());
    for(StreetIdx street_num = 0; street_num < getNumStreets(); street_num++) {
        for(int segment_num = 0; segment_num < street_streetSegments[street_num].size(); segment_num++) {
            // Add the to & from intersection of each segment to the return vector if not duplicate
            if(check_duplicate(street_intersections[street_num], getStreetSegmentInfo(street_streetSegments[street_num][segment_num]).from))
                street_intersections[street_num].push_back(getStreetSegmentInfo(street_streetSegments[street_num][segment_num]).from);
            if(check_duplicate(street_intersections[street_num], getStreetSegmentInfo(street_streetSegments[street_num][segment_num]).to))
                street_intersections[street_num].push_back(getStreetSegmentInfo(street_streetSegments[street_num][segment_num]).to);
        }
    }
}


// streetName -> paritialStreetNames
void streetName_to_partialStreetNames() {
    for(int street_id = 0; street_id < getNumStreets(); street_id++) {
        for(int length = 1; length < getStreetName(street_id).length(); length++) {
            char partialName[length + 2];  // +1 null character
            int end = (int)getStreetName(street_id).copy(partialName, length + 1, 0);
            // length: number of characters need to be copied
            // 0: position to start copy
            partialName[end] = '\0';
            // put the null character at the end
            string partialNameString = convert_to_string(partialName, length + 2);
            // make all Upper case and Delete space for comparing
            partialNameString = modify_name(partialNameString);
            pair<string, StreetIdx> toInsert(partialNameString, street_id);
            partial_street_names.insert(toInsert);
        }
    }
}

struct POI_data {
    string name;
    string type;
    LatLon position;
    OSMID OSM_id;
};

extern set<POI_data> POIs;

void load_POIs() {
    POIs.resize(getNumPointsOfInterest());
    for(int i = 0; i < getNumPointsOfInterest(); i++) {
        POI_data temp = {.name = getPOIName(i), .type = getPOIType(i), .position = getPOIPosition(i), .OSM_id = getPOIOSMNodeID(i));
        POIs.insert(temp);
    }
}
