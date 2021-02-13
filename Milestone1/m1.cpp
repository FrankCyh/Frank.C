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

// library & header file provided
#include <iostream>
#include "m1.h"
#include "StreetsDatabaseAPI.h"

// added library & header file
#include "m1_helper.cpp"
#include <math.h>
#include <utility>
#include <map>
#include <string>

using std::vector;
using std::pair;
using std::cin;
using std::cout;
using std::endl;
using std::multimap;
using std::string;


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
 variable type: vector<StreetSegmentIdx>
    Level 2:
    StreetSegmentIdx (number = getNumIntersectionStreetSegment(intersection_id)
    variable type: StreetSegmentIdx
*/
vector<vector<StreetSegmentIdx>> intersection_to_streetSegments;

/* Hierarchy
Level 1:
 StreetSegmentIdx (number = getNumIntersections())
 variable type: vector<SmallSegmentsLength>
    Level 2:
    SmallSegmentLength (number = getStreetSegmentInfo(street_segment_id).numCurvePoints)
    variable type: SmallSegmentsLength
*/
typedef double SmallSegmentsLength;
vector<vector<SmallSegmentsLength>> streetSegments_to_smallSegmentsLength;

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
vector<vector<StreetSegmentIdx>> street_to_streetSegments;

/* Hierarchy
Level 1:
 StreetIdx (number = getNumStreets())
 variable type: vector<IntersectionIdx>
    Level 2:
    IntersectionIdx (number UNKNOWN)
    variable type: IntersectionIdx
*/
vector<vector<IntersectionIdx>> street_to_intersections;

/* Hierarchy
Level 1:
 string (number = getNumStreets() * number of characters in each string name)
 variable type: pair<string, StreetIdx>
*/
multimap<string, StreetIdx> paritial_street_names;

bool loadMap(std::string map_streets_database_filename) {
    bool load_successful = loadStreetsDatabaseBIN(map_streets_database_filename);  //Indicates whether the map has loaded successfully
    if(load_successful) {
        cout << "loadMap: " << map_streets_database_filename << endl;


#pragma mark streetIntersection -> streetSegment
        //Create empty vector for each intersection
        //Complexity: O(n)
        intersection_to_streetSegments.resize(getNumIntersections());

        //Iterate through all intersections
        //Complexity; O(k), where k is a small constant
        for(IntersectionIdx intersection = 0; intersection < getNumIntersections(); intersection++) {
            //Load streetSegmentId for each intersection
            for(int i = 0; i < getNumIntersectionStreetSegment(intersection); ++i) {
                StreetSegmentIdx ss_ids = getIntersectionStreetSegment(intersection, i);
                intersection_to_streetSegments[intersection].push_back(ss_ids);
            }
        }
        //Overall Complexity: O(kn)


#pragma mark streetSegments -> smallSegments
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

#pragma mark streetSegments -> segmentLength
        segment_length.resize(getNumStreetSegments());
        for(int i = 0; i < getNumStreetSegments(); i++) {
            for(int j = 0; j < streetSegments_to_smallSegmentsLength[i].size(); j++)
                // add every small segment to calculate the length
                segment_length[i] += streetSegments_to_smallSegmentsLength[i][j];
        }

#pragma mark streetSegments -> segmentTravelTime
        segment_travel_time.resize(getNumStreetSegments());
        for(int i = 0; i < getNumStreetSegments(); i++) {
            // travel time = segment_length[i] / speedLimit
            segment_travel_time[i] = segment_length[i] / getStreetSegmentInfo(i).speedLimit;
        }

#pragma mark street -> streetSegments
        street_to_streetSegments.resize(getNumStreets());
        for(StreetSegmentIdx street_segment_id = 0; street_segment_id < getNumStreetSegments(); street_segment_id++) {
            // Load each streetSegment and insert to the vector of corresponding streetID
            street_to_streetSegments[getStreetSegmentInfo(street_segment_id).streetID].push_back(street_segment_id);
        }

#pragma mark street -> intersections
        street_to_intersections.resize(getNumStreets());
        for(StreetIdx street_num = 0; street_num < getNumStreets(); street_num++) {
            for(int segment_num = 0; segment_num < street_to_streetSegments[street_num].size(); segment_num++) {
                // Add the to&from intersection of each segment to the return vector if not duplicate. As the to and from are randomly assigned, we have to add both do and from and check dulplicate.
                if(check_duplicate(street_to_intersections[street_num], getStreetSegmentInfo(street_to_streetSegments[street_num][segment_num]).from))
                    street_to_intersections[street_num].push_back(getStreetSegmentInfo(street_to_streetSegments[street_num][segment_num]).from);
                if(check_duplicate(street_to_intersections[street_num], getStreetSegmentInfo(street_to_streetSegments[street_num][segment_num]).to))
                    street_to_intersections[street_num].push_back(getStreetSegmentInfo(street_to_streetSegments[street_num][segment_num]).to);
            }
        }

#pragma mark streetName -> paritialStreetNames
        for(int street_id = 0; street_id < getNumStreets(); street_id++) {
            for(int length = 1; length < +getStreetName(street_id).length(); length++) {
                char partialName[length + 1];  // +1 null character
                int end = (int)getStreetName(street_id).copy(partialName, length, 0);
                // length: number of characters need to be copied
                // 0: position to start copy
                partialName[end] = '\0';
                // put the null character at the end
                string partialNameString = convert_to_string(partialName, length + 1);
                // make all Upper case and Delete space for comparing
                partialNameString = modify_name(partialNameString);
                pair<string, StreetIdx> toInsert(partialNameString, street_id);
                paritial_street_names.insert(toInsert);
            }
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
    street_to_streetSegments.clear();
    street_to_intersections.clear();
    paritial_street_names.clear();

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


#pragma mark Street test 1 Pass
// Returns all intersections along the a given street
// Speed Requirement --> high
vector<IntersectionIdx> findIntersectionsOfStreet(StreetIdx street_id) {
    return street_to_intersections[street_id];
}

#pragma mark Street test 2 Pass
// Return all intersection ids at which the two given streets intersect
// This function will typically return one intersection id for streets
// that intersect and a length 0 vector for streets that do not. For unusual
// curved streets it is possible to have more than one intersection at which
// two streets cross.
// Speed Requirement --> high
vector<IntersectionIdx> findIntersectionsOfTwoStreets(pair<StreetIdx, StreetIdx> street_ids) {
    vector<IntersectionIdx> intersectionsOfTwoStreets;
    // Iterate through street_to_intersections of two streets to find common intersections
    for(int intersection_num_1 = 0; intersection_num_1 < street_to_intersections[street_ids.first].size(); intersection_num_1++) {
        for(int intersection_num_2 = 0; intersection_num_2 < street_to_intersections[street_ids.second].size(); intersection_num_2++) {
            if(street_to_intersections[street_ids.first][intersection_num_1] == street_to_intersections[street_ids.second][intersection_num_2])
                intersectionsOfTwoStreets.push_back(street_to_intersections[street_ids.first][intersection_num_1]);
        }
    }

    return intersectionsOfTwoStreets;
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
    vector<StreetIdx> street_ids_from_partial_street_name;

    if(street_prefix.length() == 0)
        return street_ids_from_partial_street_name;  // Return an empty vector if street_prefix is an empty string

    street_prefix = modify_name(street_prefix);

    auto itr1 = paritial_street_names.lower_bound(street_prefix);
    auto itr2 = paritial_street_names.upper_bound(street_prefix);

    while(itr1 != itr2) {
        street_ids_from_partial_street_name.push_back(itr1->second);
        itr1++;
    }
    /*
    pair<multimap<string, StreetIdx>::iterator, multimap<string, StreetIdx>::iterator> bound;
    bound = paritial_street_names.equal_range(street_prefix);
    for(multimap<string, StreetIdx>::iterator it = bound.first; it != bound.second; it++){
        street_ids_from_partial_street_name.push_back(it->second);
    }
    */

    return street_ids_from_partial_street_name;
}

#pragma mark Distance Time Test 4 Pass
// Returns the length of a given street in meters
// Speed Requirement --> high
double findStreetLength(StreetIdx street_id) {
    double streetLength = 0;
    // Iterate through street_to_streetSegments, add together lengths of all streetSegments of the street
    // Make use of function: findStreetSegmentLength
    for(int segment_num = 0; segment_num < street_to_streetSegments[street_id].size(); segment_num++) {
        streetLength += findStreetSegmentLength(street_to_streetSegments[street_id][segment_num]);
    }

    return streetLength;
}

#pragma mark Street Test 4
// Return the smallest rectangle that contains all the intersections and
// curve points of the given street (i.e. the min,max lattitude
// and longitude bounds that can just contain all points of the street).
// Speed Requirement --> none
LatLonBounds findStreetBoundingBox(StreetIdx street_id) {
    // Initialize max and min latitude and longitude to that of the first intersection of the first street segment
    double min_lat = getIntersectionPosition(getStreetSegmentInfo(street_to_streetSegments[street_id][0]).from).latitude();
    double max_lat = getIntersectionPosition(getStreetSegmentInfo(street_to_streetSegments[street_id][0]).from).latitude();
    double min_lon = getIntersectionPosition(getStreetSegmentInfo(street_to_streetSegments[street_id][0]).from).longitude();
    double max_lon = getIntersectionPosition(getStreetSegmentInfo(street_to_streetSegments[street_id][0]).from).longitude();
    
    for (int segment_num = 0; segment_num < street_to_streetSegments[street_id].size(); segment_num++) {
        // Compare segment.from to current min/max
        max_lat = compare_max_lat(max_lat, getIntersectionPosition(getStreetSegmentInfo(street_to_streetSegments[street_id][segment_num]).from));
        min_lat = compare_min_lat(min_lat, getIntersectionPosition(getStreetSegmentInfo(street_to_streetSegments[street_id][segment_num]).from));
        max_lon = compare_max_lon(max_lon, getIntersectionPosition(getStreetSegmentInfo(street_to_streetSegments[street_id][segment_num]).from));
        min_lon = compare_min_lon(min_lon, getIntersectionPosition(getStreetSegmentInfo(street_to_streetSegments[street_id][segment_num]).from));
    // Compare segment.to to current min/max
    max_lat = compare_max_lat(max_lat, getIntersectionPosition(getStreetSegmentInfo(street_to_streetSegments[street_id][segment_num]).to));
        min_lat = compare_min_lat(min_lat, getIntersectionPosition(getStreetSegmentInfo(street_to_streetSegments[street_id][segment_num]).to));
        max_lon = compare_max_lon(max_lon, getIntersectionPosition(getStreetSegmentInfo(street_to_streetSegments[street_id][segment_num]).to));
        min_lon = compare_min_lon(min_lon, getIntersectionPosition(getStreetSegmentInfo(street_to_streetSegments[street_id][segment_num]).to));
    // Compare all curve points of segment to current min/max
    for (int curve_point_num = 0; curve_point_num < getStreetSegmentInfo(street_to_streetSegments[street_id][segment_num]).numCurvePoints; curve_point_num++) {
        max_lat = compare_max_lat(max_lat, getStreetSegmentCurvePoint(street_to_streetSegments[street_id][segment_num], curve_point_num));
            min_lat = compare_min_lat(min_lat, getStreetSegmentCurvePoint(street_to_streetSegments[street_id][segment_num], curve_point_num));
            max_lon = compare_max_lon(max_lon, getStreetSegmentCurvePoint(street_to_streetSegments[street_id][segment_num], curve_point_num));
            min_lon = compare_min_lon(min_lon, getStreetSegmentCurvePoint(street_to_streetSegments[street_id][segment_num], curve_point_num));
    }
    }
    // Construct LatLon for point with min and max coordinate
    LatLon min_point(min_lat, min_lon);
    LatLon max_point(max_lat, max_lon);
    // Make LatLonBounds
    LatLonBounds streetBoundingBox;
    streetBoundingBox.min = min_point;
    streetBoundingBox.max = max_point;
    return streetBoundingBox;
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
    featureArea = featureArea > 0 ? featureArea : -featureArea;

    return featureArea;
}
