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
#include "m1.h"


// added library & header file
#include "helper.h"
#include "globals.h"


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

bool loadMap(std::string map_streets_database_filename) {
    bool load_successful = loadStreetsDatabaseBIN(map_streets_database_filename);  //Indicates whether the map has loaded successfully
    if(load_successful) {
        cout << "loadMap: " << map_streets_database_filename << endl;

        // Load structure using functions calls in Globals.cpp
        streetIntersection_to_streetSegment();
        streetSegments_to_smallSegments();
        streetSegments_to_segmentLength();
        streetSegments_to_segmentTravelTime();
        street_to_streetSegments();
        street_to_intersections();
        streetName_to_partialStreetNames();
        
        // Milestone 3
        // load_NodeList();
    }
    return load_successful;
}

void closeMap() {
    // clear the global variables
    intersection_streetSegments.clear();
    streetSegments_smallSegmentsLength.clear();
    segment_length.clear();
    segment_travel_time.clear();
    street_streetSegments.clear();
    street_intersections.clear();
    partial_street_names.clear();
    
    // Milestone 3
    // NodeList.clear();

    closeStreetDatabase();
}

/*** Distance Time Test 1 Pass ***/
// Returns the distance between two (lattitude,longitude) coordinates in meters
// Speed Requirement --> moderate
double findDistanceBetweenTwoPoints(pair<LatLon, LatLon> points) {
    double average_lat = (points.first.latitude() + points.second.latitude()) / 2 * kDegreeToRadian;

    double x1 = points.first.longitude() * kDegreeToRadian * cos(average_lat), y1 = points.first.latitude() * kDegreeToRadian;
    double x2 = points.second.longitude() * kDegreeToRadian * cos(average_lat), y2 = points.second.latitude() * kDegreeToRadian;

    return kEarthRadiusInMeters * sqrt(pow((x2 - x1), 2) + pow((y2 - y1), 2));
}

/*** Distance Time Test 2 Pass ***/
// Returns the length of the given street segment in meters
// Speed Requirement --> moderate
double findStreetSegmentLength(StreetSegmentIdx street_segment_id) {
    return segment_length[street_segment_id];
}

/*** Distance Time Test 3 Pass ***/
// Returns the travel time to drive from one end of a street segment in
// to the other, in seconds, when driving at the speed limit
// Note: (time = distance/speed_limit)
// Speed Requirement --> high
double findStreetSegmentTravelTime(StreetSegmentIdx street_segment_id) {
    return segment_travel_time[street_segment_id];
}

 /*** Spatial Test 1 Pass ***/
// Returns the nearest intersection to the given position
// Speed Requirement --> none
IntersectionIdx findClosestIntersection(LatLon my_position) {
    // find each intersection nearby
    double distance, shortest_distance;
    pair<LatLon, LatLon> first_segment(my_position, getIntersectionPosition(0));
    shortest_distance = findDistanceBetweenTwoPoints(first_segment);

    // traverse through all the intersection, find shortest distance
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

 /*** Intersection test 1 Pass ***/
// Returns the street segments that connect to the given intersection
// Speed Requirement --> high
vector<StreetSegmentIdx> findStreetSegmentsOfIntersection(IntersectionIdx intersection_id) {
    return intersection_streetSegments[intersection_id];
}

 /*** Intersection test 2 Pass ***/
// Returns the street names at the given intersection (includes duplicate
// street names in the returned vector)
// Speed Requirement --> high
vector<std::string> findStreetNamesOfIntersection(IntersectionIdx intersection_id) {
    vector<std::string> streetNamesOfIntersection;
    for(int intersection = 0; intersection < getNumIntersectionStreetSegment(intersection_id); intersection++) {
        streetNamesOfIntersection.push_back(getStreetName(getStreetSegmentInfo(getIntersectionStreetSegment(intersection_id, intersection)).streetID));
    }
    return streetNamesOfIntersection;
}

 /*** Intersection test 3 Pass ***/
// Returns all intersections reachable by traveling down one street segment
// from the given intersection (hint: you can't travel the wrong way on a
// 1-way street)
// the returned vector should NOT contain duplicate intersections
// Speed Requirement --> high
vector<IntersectionIdx> findAdjacentIntersections(IntersectionIdx intersection_id) {
    set<IntersectionIdx> adjacentIntersections;
    // iterate through each street segment connected to the intersection,
    for(int segment_num = 0; segment_num < getNumIntersectionStreetSegment(intersection_id); segment_num++) {
        // declare streetSegmentIdx variable for simplicity
        auto segmentInfo = getStreetSegmentInfo(getIntersectionStreetSegment(intersection_id, segment_num));
        // the segment is not oneWay
        if(segmentInfo.oneWay == false) {
            // determine whether the IntersectionIdx is from or to in the StreetSegmentInfo structure, push the other
            if(intersection_id == segmentInfo.to)  // no need to check dulplicate for set, for implementation using vectors: check_duplicate(adjacentIntersections, segmentInfo.from)
                adjacentIntersections.insert(segmentInfo.from);
            else
                adjacentIntersections.insert(segmentInfo.to);
        }
        // the segment is oneWay, but the route is not blocked and the intersection is not duplicate
        else if(segmentInfo.from == intersection_id)
            adjacentIntersections.insert(segmentInfo.to);
    }
    vector<IntersectionIdx> return_vec(adjacentIntersections.begin(), adjacentIntersections.end());
    return return_vec;
}


 /*** Street test 1 Pass ***/
// Returns all intersections along the a given street
// Speed Requirement --> high
vector<IntersectionIdx> findIntersectionsOfStreet(StreetIdx street_id) {
    return street_intersections[street_id];
}

 /*** Street test 2 Pass ***/
// Return all intersection ids at which the two given streets intersect
// This function will typically return one intersection id for streets
// that intersect and a length 0 vector for streets that do not. For unusual
// curved streets it is possible to have more than one intersection at which
// two streets cross.
// Speed Requirement --> high
vector<IntersectionIdx> findIntersectionsOfTwoStreets(pair<StreetIdx, StreetIdx> street_ids) {
    vector<IntersectionIdx> intersectionsOfTwoStreets;
    // Iterate through street_to_intersections of two streets to find common intersections
    for(int intersection_num_1 = 0; intersection_num_1 < street_intersections[street_ids.first].size(); intersection_num_1++) {
        for(int intersection_num_2 = 0; intersection_num_2 < street_intersections[street_ids.second].size(); intersection_num_2++) {
            if(street_intersections[street_ids.first][intersection_num_1] == street_intersections[street_ids.second][intersection_num_2])
                intersectionsOfTwoStreets.push_back(street_intersections[street_ids.first][intersection_num_1]);
        }
    }

    return intersectionsOfTwoStreets;
}

 /*** Street Test 3 Pass Function fail Performance ***/
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
    // use set to avoid duplicate
    std::set<StreetIdx> street_ids_from_partial_street_name;
    //street_ids_from_partial_street_name.resize(1);
    if(street_prefix.length() == 0) {
        return {};  // Return an empty vector if street_prefix is an empty string
    }
    
    
    
    // modify_name
    int len = (int)street_prefix.length();
    // Lower case to upper case
    for(int i = 0; i < len; i++) {
        if(street_prefix[i] >= 'a' && street_prefix[i] <= 'z') {
            street_prefix[i] -= 32;
        }
    }
    // Remove space
    street_prefix.erase(remove(street_prefix.begin(), street_prefix.end(), ' '), street_prefix.end());
    
    
    
    // find the beginning and ending of traversing
    auto it_start = partial_street_names.lower_bound(street_prefix);
    // add the last character of the prefix by 1, e.g. "abc" -> "abd", so that we can know where to stop traversing
    *street_prefix.rbegin() = *street_prefix.rbegin() + 1;
    auto it_end = partial_street_names.lower_bound(street_prefix);

    // push into a set
    for(multimap<std::string, StreetIdx>::iterator it = it_start; it != it_end; it++) {
        street_ids_from_partial_street_name.insert(it->second);
    }

    // convert back to a vector
    std::vector<StreetIdx> return_vec(street_ids_from_partial_street_name.begin(), street_ids_from_partial_street_name.end());
    return return_vec;

    /* Previous Solution (incorrect)
    vector<StreetIdx> street_ids_from_partial_street_name;

    if(street_prefix.length() == 0)
        return street_ids_from_partial_street_name;  // Return an empty vector if street_prefix is an empty string

    street_prefix = modify_name(street_prefix);
    cout << street_prefix << endl;

    auto itr1 = partial_street_names.lower_bound(street_prefix);
    auto itr2 = partial_street_names.upper_bound(street_prefix);

    for(multimap<std::string, StreetIdx>::iterator it = itr1; it != itr2; it++) {
        street_ids_from_partial_street_name.push_back(it->second);
        cout << it->second << it->first << endl;
    }
    return street_ids_from_partial_street_name;
    // Equal range is not suitable, because it checks for exact value
    //      pair<multimap<string, StreetIdx>::iterator, multimap<string, StreetIdx>::iterator> bound;
    //      bound = paritial_street_names.equal_range(street_prefix);
    //      for(multimap<string, StreetIdx>::iterator it = bound.first; it != bound.second; it++){
    //            street_ids_from_partial_street_name.push_back(it->second);
    //      }
    */
}

/*** Distance Time Test 4 Pass ***/
// Returns the length of a given street in meters
// Speed Requirement --> high
double findStreetLength(StreetIdx street_id) {
    double streetLength = 0;
    // Iterate through street_streetSegments, add together lengths of all streetSegments of the street
    // Make use of function: findStreetSegmentLength
    for(int segment_num = 0; segment_num < street_streetSegments[street_id].size(); segment_num++) {
        streetLength += findStreetSegmentLength(street_streetSegments[street_id][segment_num]);
    }

    return streetLength;
}

 /*** Street Test 4 Pass ***/
// Return the smallest rectangle that contains all the intersections and
// curve points of the given street (i.e. the min,max lattitude
// and longitude bounds that can just contain all points of the street).
// Speed Requirement --> none
LatLonBounds findStreetBoundingBox(StreetIdx street_id) {
    // Initialize max and min latitude and longitude to that of the first intersection of the first street segment
    LatLon first_point = getIntersectionPosition(getStreetSegmentInfo(street_streetSegments[street_id][0]).from);
    double max_lat = first_point.latitude();
    double min_lat = first_point.latitude();
    double max_lon = first_point.longitude();
    double min_lon = first_point.longitude();

    for(int segment_num = 0; segment_num < street_streetSegments[street_id].size(); segment_num++) {
        // Compare segment.from to current min/max
        LatLon from_point = getIntersectionPosition(getStreetSegmentInfo(street_streetSegments[street_id][segment_num]).from);
        update_bounding_box(max_lat, min_lat, max_lon, min_lon, from_point);
        // Compare segment.to to current min/max
        LatLon to_point = getIntersectionPosition(getStreetSegmentInfo(street_streetSegments[street_id][segment_num]).to);
        update_bounding_box(max_lat, min_lat, max_lon, min_lon, to_point);
        // Compare all curve points of segment to current min/max
        for(int curve_point_num = 0; curve_point_num < getStreetSegmentInfo(street_streetSegments[street_id][segment_num]).numCurvePoints; curve_point_num++) {
            LatLon curve_point = getStreetSegmentCurvePoint(street_streetSegments[street_id][segment_num], curve_point_num);
            update_bounding_box(max_lat, min_lat, max_lon, min_lon, curve_point);
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

 /*** Spatial Test 2 Pass ***/
// Returns the nearest point of interest of the given name to the given position
// Speed Requirement --> none
POIIdx findClosestPOI(LatLon my_position, std::string POIname) {
    double distance = 0, shortest_distance = 0;

    // Find the First POIname
    POIIdx first_found;
    POIIdx closestPOI = 0;
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

 /*** Distance Time Test 5 Pass ***/
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
