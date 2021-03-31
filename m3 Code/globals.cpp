//
// globals.cpp
//Created by Frank on 2021/3/2
//
//Purpose of the program:
//

#include "globals.h"
#include "helper.h"

/*******************************************/
/* global variable created for Milestone 1 */
/*******************************************/

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
 number of variable = getNumStreets() * number of characters in each string name
 variable type: pair<string, StreetIdx>
*/
multimap<std::string, StreetIdx> partial_street_names;


/************** Initialization *************/

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

/*******************************************/
/* global variable created for Milestone 2 */
/*******************************************/

// average latitude is used a lot so we declare it global
double avg_lat = 0.;

/* Hierarchy
 number of variable = getNumIntersections()
 variable type: intersection_data (intersection structure)
*/
vector<intersection_data> intersections;

/* Hierarchy
 number of variable = getNumStreetSegments()
 variable type: segment_data (segment structure)
*/
vector<segment_data> segments;

/* Hierarchy
 number of variable = getNumPointsOfInterest()
 variable type: POI_data(created POI structure)
*/
vector<POI_data> POIs;

/* Hierarchy
 number of variable = getNumFeatures()
 variable type: feature_data(created feature structure)
*/
vector<feature_data> features;


/************** Initialization *************/

void load_segments() {
    // initialize structure vector
    segments.resize(getNumStreetSegments());
    for(StreetSegmentIdx i = 0; i < getNumStreetSegments(); ++i) {
        segments[i].name = getStreetName(getStreetSegmentInfo(i).streetID);
        segments[i].from_point = getStreetSegmentInfo(i).from;
        segments[i].to_point = getStreetSegmentInfo(i).to;
        segments[i].one_way = getStreetSegmentInfo(i).oneWay;
        segments[i].num_curve_points = getStreetSegmentInfo(i).numCurvePoints;
        segments[i].speed_limit = getStreetSegmentInfo(i).speedLimit;
        segments[i].street_id = getStreetSegmentInfo(i).streetID;
        segments[i].way_OSM_id = getStreetSegmentInfo(i).wayOSMID;

        double x_from, y_from, x_to, y_to;
        int curve_point_num = getStreetSegmentInfo(i).numCurvePoints;
        if(curve_point_num == 0) {
            // segment has no curve point
            x_from = x_from_lon(getIntersectionPosition(getStreetSegmentInfo(i).from).longitude());
            y_from = y_from_lat(getIntersectionPosition(getStreetSegmentInfo(i).from).latitude());
            x_to = x_from_lon(getIntersectionPosition(getStreetSegmentInfo(i).to).longitude());
            y_to = y_from_lat(getIntersectionPosition(getStreetSegmentInfo(i).to).latitude());

        } else if(curve_point_num == 1) {
            // segment has 1 curve point
            x_from = x_from_lon(getIntersectionPosition(getStreetSegmentInfo(i).from).longitude());
            y_from = y_from_lat(getIntersectionPosition(getStreetSegmentInfo(i).from).latitude());
            x_to = x_from_lon(getStreetSegmentCurvePoint(i, 0).longitude());
            y_to = y_from_lat(getStreetSegmentCurvePoint(i, 0).latitude());

        } else {
            // segment has at least 2 curve points
            x_from = x_from_lon(getStreetSegmentCurvePoint(i, curve_point_num / 2 - 1).longitude());
            y_from = y_from_lat(getStreetSegmentCurvePoint(i, curve_point_num / 2 - 1).latitude());
            x_to = x_from_lon(getStreetSegmentCurvePoint(i, curve_point_num / 2).longitude());
            y_to = y_from_lat(getStreetSegmentCurvePoint(i, curve_point_num / 2).latitude());
        }
        segments[i].tilt_angle = angle_between_two_points(x_from, y_from, x_to, y_to);
    }
}

void load_POIs() {
    // initialize structure vector
    POIs.resize(getNumPointsOfInterest());
    for(POIIdx i = 0; i < getNumPointsOfInterest(); ++i) {
        POIs[i].name = getPOIName(i);
        POIs[i].type = getPOIType(i);
        POIs[i].position = getPOIPosition(i);
        POIs[i].OSM_id = getPOIOSMNodeID(i);
    }
}

void load_features() {
    // initialize structure vector
    features.resize(getNumFeatures());
    for(FeatureIdx i = 0; i < getNumFeatures(); ++i) {
        features[i].name = getFeatureName(i);
        features[i].type = asString(getFeatureType(i));
        features[i].num_points = getNumFeaturePoints(i);
        features[i].OSM_id = getFeatureOSMID(i);
        features[i].area = findFeatureArea(i);
    }
}


/*******************************************/
/* global variable created for Milestone 3 */
/*******************************************/


std::vector<Node> NodeList;

void load_NodeList() {
    NodeList.resize(getNumIntersections());
    for(int i = 0; i < getNumIntersections(); i++) {
        NodeList.push_back(Node(i, NON_EXISTENT, INT8_MAX));
    }
}

priority_queue<WaveElem, std::vector<WaveElem>, CompareEstimatedTravelTime> Wavefront;

