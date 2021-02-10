/*
 * Copyright 2021 University of Toronto
 *
 * Permission is hereby granted, to use this software and associated
 * documentation files (the "Software") in course work at the University
 * of Toronto, or for personal use. Other uses are prohibited, in
 * particular the distribution of the Software either publicly or to third
 * parties.
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <iostream>
#include "m1.h"
#include "StreetsDatabaseAPI.h"

#include "m1_helper.cpp"
#include <math.h>

using std::vector;
using std::pair;
using std::cin;
using std::cout;
using std::endl;


// loadMap will be called with the name of the file that stores the "layer-2"
// map data accessed through StreetsDatabaseAPI: the street and intersection
// data that is higher-level than the raw OSM data).
// This file name will always end in ".streets.bin" and you
// can call loadStreetsDatabaseBIN with this filename to initialize the
// layer 2 (StreetsDatabase) API.
// If you need data from the lower level, layer 1, API that provides raw OSM
// data (nodes, ways, etc.) you will also need to initialize the layer 1
// OSMDatabaseAPI by calling loadOSMDatabaseBIN. That function needs the
// name of the ".osm.bin" file that matches your map -- just change
// ".streets" to ".osm" in the map_streets_database_filename to get the proper
// name.

// global variable, read only in functions other than load/closeMap
// need to free it in closeMap()

/* Hierarchy
Level 1:
 Intersection (number = getNumIntersections())
    Level 2:
    StreetSegmentIdx (number = getNumIntersectionStreetSegment(intersection_id)
        variable type: StreetSegmentIdx
*/
vector<vector<StreetSegmentIdx>> intersection_to_streetSegments;

/* Hierarchy
Level 1:
 StreetSegmentIdx (number = getNumIntersections())
    Level 2:
    SmallSegmentLength (number = getStreetSegmentInfo(street_segment_id).numCurvePoints)
        variable type: SmallSegmentsLength
*/
typedef double SmallSegmentsLength;
vector<vector<SmallSegmentsLength>> streetSegments_to_smallSegmentsLength;

/* Hierarchy
Level 1: index streetSegmentIdx
 variable type: double
 */
vector<double> segment_length;

/* Hierarchy
Level 1: index streetSegmentIdx
 variable type: double
 */
vector<double> segment_travel_time;

bool loadMap(std::string map_streets_database_filename) {
    bool load_successful = loadStreetsDatabaseBIN(map_streets_database_filename);  //Indicates whether the map has loaded successfully
    if(load_successful) {
        cout << "loadMap: " << map_streets_database_filename << endl;


#pragma mark streetIntersection → streetSegment
        //Create empty vector for each intersection
        //Complexity: O(n)
        intersection_to_streetSegments.resize(getNumIntersections());

        //Iterate through all intersections
        //Complexity; O(k), where k is a small constant
        for(IntersectionIdx intersection = 0; intersection < getNumIntersections(); intersection++) {
            //Load streetSegmentId for each intersection


            for(int i = 0; i < getNumIntersectionStreetSegment(intersection); ++i) {
                StreetSegmentIdx ss_ids = getIntersectionStreetSegment(intersection, i);

                /*
                cout << "Intersection NO. " << i << " :" << endl;
                
                */
                intersection_to_streetSegments[intersection].push_back(ss_ids);
                /*
                cout << ss_ids << " ";
                */
            }

            /*
            cout << endl;
            */
        }
        //Overall Complexity: O(kn)


#pragma mark streetSegments → smallSegments
        streetSegments_to_smallSegmentsLength.resize(getNumStreetSegments());
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
                streetSegments_to_smallSegmentsLength[street_segment_id].push_back(findDistanceBetweenTwoPoints(pair_of_intersection_id));
                // push in total numCurvePoints + 1 items
            }
        }

#pragma mark streetSegments → segmentLength
        segment_length.resize(getNumStreetSegments());
        for(int i = 0; i < getNumStreetSegments(); i++) {
            for(int j = 0; j < streetSegments_to_smallSegmentsLength[i].size(); j++)
                segment_length[i] += streetSegments_to_smallSegmentsLength[i][j];
        }

#pragma mark streetSegments → segmentTravelTime
        segment_travel_time.resize(getNumStreetSegments());
        for(int i = 0; i < getNumStreetSegments(); i++) {
            segment_travel_time[i] += segment_length[i] / getStreetSegmentInfo(i).speedLimit;
        }
    }
    return load_successful;
}

void closeMap() {
    // clear the global variables
    intersection_to_streetSegments.clear();
    streetSegments_to_smallSegmentsLength.clear();
    segment_length.clear();
    segment_travel_time.clear();

    //Clean-up your map related data structures here
    // Not pass valgrind, memory leak for global variables
    closeStreetDatabase();
}

#pragma mark Distance Time Test 1 Pass
// Returns the distance between two (lattitude,longitude) coordinates in meters
// Speed Requirement --> moderate
double findDistanceBetweenTwoPoints(pair<LatLon, LatLon> points) {
    double average_lat = (points.first.latitude() + points.second.latitude()) / 2 * kDegreeToRadian;
    double x1 = points.first.longitude() * kDegreeToRadian * cos(average_lat), y1 = points.first.latitude() * kDegreeToRadian;

    double x2 = points.second.longitude() * kDegreeToRadian * cos(average_lat), y2 = points.second.latitude() * kDegreeToRadian;

    return kEarthRadiusInMeters * sqrt(pow((x2 - x1), 2) + pow((y2 - y1), 2));
}

#pragma mark Distance Time Test 2 Pass
// Returns the length of the given street segment in meters
// Speed Requirement --> moderate
double findStreetSegmentLength(StreetSegmentIdx street_segment_id) {
    //    double streetSegmentLength = 0;
    //    for(int small_segment_id = 0; small_segment_id < streetSegments_to_smallSegmentsLength[street_segment_id].size(); small_segment_id++) {
    //        streetSegmentLength += streetSegments_to_smallSegmentsLength[street_segment_id][small_segment_id];
    //    }

    return segment_length[street_segment_id];
}

#pragma mark Distance Time Test 3 Pass
// Pass Function but Failed Performance

// Returns the travel time to drive from one end of a street segment in
// to the other, in seconds, when driving at the speed limit
// Note: (time = distance/speed_limit)
// Speed Requirement --> high
double findStreetSegmentTravelTime(StreetSegmentIdx street_segment_id) {
    return segment_travel_time[street_segment_id];
}

#pragma mark Spatial Test 1 Pass
// Returns the nearest intersection to the given position
// Speed Requirement --> none
IntersectionIdx findClosestIntersection(LatLon my_position) {
    // find each intersection nearby
    double distance, shortest_distance;
    pair<LatLon, LatLon> first_segment(my_position, getIntersectionPosition(0));
    shortest_distance = findDistanceBetweenTwoPoints(first_segment);
    IntersectionIdx closestIntersection = 0;
    for(IntersectionIdx i = 1; i < getNumIntersections(); i++) {
        pair<LatLon, LatLon> segment(my_position, getIntersectionPosition(i));
        distance = findDistanceBetweenTwoPoints(segment);
        if(distance < shortest_distance) {
            shortest_distance = distance;
            closestIntersection = i;
        }
    }
    return closestIntersection;
}

#pragma mark Intersection test 1 Pass
// Returns the street segments that connect to the given intersection
// Speed Requirement --> high
vector<StreetSegmentIdx> findStreetSegmentsOfIntersection(IntersectionIdx intersection_id) {
    return intersection_to_streetSegments[intersection_id];
}

#pragma mark Intersection test 2 Pass
// Returns the street names at the given intersection (includes duplicate
// street names in the returned vector)
// Speed Requirement --> high
vector<std::string> findStreetNamesOfIntersection(IntersectionIdx intersection_id) {
    vector<std::string> streetNamesOfIntersection;
    for(int intersection = 0; intersection < getNumIntersectionStreetSegment(intersection_id); intersection++) {
        streetNamesOfIntersection.push_back(getStreetName(getStreetSegmentInfo(getIntersectionStreetSegment(intersection_id, intersection)).streetID));
        //getStreetName(StreetIdx streetIdx)
        //struct StreetSegmentInfo.streetID
        //streetSegmentInfo getStreetSegmentInfo(StreetSegmentIdx streetSegmentIdx)
        //StreetSegmentIdx getIntersectionStreetSegment(IntersectionIdx intersectionIdx, int segmentNumber);
    }
    return streetNamesOfIntersection;
}

#pragma mark Intersection test 3 Pass
// Returns all intersections reachable by traveling down one street segment
// from the given intersection (hint: you can't travel the wrong way on a
// 1-way street)
// the returned vector should NOT contain duplicate intersections
// Speed Requirement --> high
vector<IntersectionIdx> findAdjacentIntersections(IntersectionIdx intersection_id) {
    vector<IntersectionIdx> adjacentIntersections;
    // iterate through each street segment connected to the intersection,
    for(int segment_num = 0; segment_num < getNumIntersectionStreetSegment(intersection_id); segment_num++) {
        // the segment is not oneWay
        if(getStreetSegmentInfo(getIntersectionStreetSegment(intersection_id, segment_num)).oneWay == false) {
            // determine whether the IntersectionIdx is from or to in the StreetSegmentInfo structure, push the other
            if(intersection_id == getStreetSegmentInfo(getIntersectionStreetSegment(intersection_id, segment_num)).to && check_duplicate(adjacentIntersections, getStreetSegmentInfo(getIntersectionStreetSegment(intersection_id, segment_num)).from))
                adjacentIntersections.push_back(getStreetSegmentInfo(getIntersectionStreetSegment(intersection_id, segment_num)).from);
            else if(check_duplicate(adjacentIntersections, getStreetSegmentInfo(getIntersectionStreetSegment(intersection_id, segment_num)).to))
                adjacentIntersections.push_back(getStreetSegmentInfo(getIntersectionStreetSegment(intersection_id, segment_num)).to);
        }
        // the segment is oneWay, but the route is not blocked and the intersection is not duplicate
        else if((getStreetSegmentInfo(getIntersectionStreetSegment(intersection_id, segment_num)).from == intersection_id) && check_duplicate(adjacentIntersections, getStreetSegmentInfo(getIntersectionStreetSegment(intersection_id, segment_num)).to))
            adjacentIntersections.push_back(getStreetSegmentInfo(getIntersectionStreetSegment(intersection_id, segment_num)).to);
    }
    return adjacentIntersections;
}


#pragma mark Street test 1
// Returns all intersections along the a given street
// Speed Requirement --> high
vector<IntersectionIdx> findIntersectionsOfStreet(StreetIdx street_id) {
    vector<IntersectionIdx> a;
    return a;
}

#pragma mark Street test 2
// Return all intersection ids at which the two given streets intersect
// This function will typically return one intersection id for streets
// that intersect and a length 0 vector for streets that do not. For unusual
// curved streets it is possible to have more than one intersection at which
// two streets cross.
// Speed Requirement --> high
vector<IntersectionIdx> findIntersectionsOfTwoStreets(pair<StreetIdx, StreetIdx> street_ids) {
    vector<IntersectionIdx> a;
    return a;
}

#pragma mark Street Test 3
// Returns all street ids corresponding to street names that start with the
// given prefix
// The function should be case-insensitive to the street prefix.
// The function should ignore spaces.
//  For example, both "bloor " and "BloOrst" are prefixes to
// "Bloor Street East".
// If no street names match the given prefix, this routine returns an empty
// (length 0) vector.
// You can choose what to return if the street prefix passed in is an empty
// (length 0) string, but your program must not crash if street_prefix is a
// length 0 string.
// Speed Requirement --> high
vector<StreetIdx> findStreetIdsFromPartialStreetName(std::string street_prefix) {
    vector<StreetIdx> a;
    return a;
}

#pragma mark Distance Time Test 4
// Returns the length of a given street in meters
// Speed Requirement --> high
double findStreetLength(StreetIdx street_id) {
    return 0;
}

#pragma mark Street Test 4
// Return the smallest rectangle that contains all the intersections and
// curve points of the given street (i.e. the min,max lattitude
// and longitude bounds that can just contain all points of the street).
// Speed Requirement --> none
LatLonBounds findStreetBoundingBox(StreetIdx street_id) {
    LatLonBounds a;
    return a;
}

#pragma mark Spatial Test 2 Pass
// Returns the nearest point of interest of the given name to the given position
// Speed Requirement --> none
POIIdx findClosestPOI(LatLon my_position, std::string POIname) {
    double distance, shortest_distance;

    // Find the First POIname
    POIIdx first_found;
    POIIdx closestPOI;
    for(first_found = 0; first_found < getNumPointsOfInterest(); first_found++) {
        if(getPOIName(first_found) == POIname) {
            pair<LatLon, LatLon> first_segment(my_position, getPOIPosition(first_found));
            shortest_distance = findDistanceBetweenTwoPoints(first_segment);
            closestPOI = first_found;
            break;
        }
    }

    // Compare shortest_distance to Other POI, find the closestPOI
    for(int i = first_found + 1; i < getNumPointsOfInterest(); i++) {
        if(getPOIName(i) == POIname) {
            pair<LatLon, LatLon> segment(my_position, getPOIPosition(i));
            distance = findDistanceBetweenTwoPoints(segment);
            if(distance < shortest_distance) {
                shortest_distance = distance;
                closestPOI = i;
            }
        }
    }
    return closestPOI;
}

#pragma mark Distance Time Test 5 Pass
// Returns the area of the given closed feature in square meters
// Assume a non self-intersecting polygon (i.e. no holes)
// Return 0 if this feature is not a closed polygon.
// Speed Requirement --> moderate
double findFeatureArea(FeatureIdx feature_id) {
    // add and deduct with respect to the y-axis

    double featureArea = 0;
    int numFeaturePoints = getNumFeaturePoints(feature_id);

    if(numFeaturePoints == 1)
        return 0;  // single point feature, area = 0

    LatLon pointFirst = getFeaturePoint(feature_id, 0);
    LatLon pointLast = getFeaturePoint(feature_id, numFeaturePoints - 1);
    if(pointFirst.latitude() != pointLast.latitude() && pointFirst.longitude() != pointLast.longitude())
        return 0;  // not a closed polygon, return 0

    // calculate area
    LatLon point1, point2;
    double y1, y2, x1, x2, average_lat;
    for(int feature_point_id = 0; feature_point_id < numFeaturePoints - 1; feature_point_id++) {
        point1 = getFeaturePoint(feature_id, feature_point_id);
        point2 = getFeaturePoint(feature_id, feature_point_id + 1);

        y1 = point1.latitude() * kDegreeToRadian;
        y2 = point2.latitude() * kDegreeToRadian;
        average_lat = (y1 + y2) / 2;
        x1 = point1.longitude() * kDegreeToRadian * cos(average_lat);
        x2 = point2.longitude() * kDegreeToRadian * cos(average_lat);

        // total area += trapezoid formed by the segment and the y axis
        featureArea += ((y1 - y2) * kEarthRadiusInMeters) * ((x1 + x2) * kEarthRadiusInMeters) / 2;  // (y1 - y2) = height, (x1 + x2) = sum of the base
    }

    //flip if negative
    featureArea = featureArea < 0 ? featureArea : -featureArea;

    return featureArea;
}
