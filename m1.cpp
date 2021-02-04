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

// global variable
// map data structure: nested vector
std::vector<std::vector<StreetIdx>> intersection_street_segments;

bool loadMap(std::string map_streets_database_filename) {
    bool load_successful = loadStreetsDatabaseBIN(map_name);  //Indicates whether the map has loaded
    //successfully
    if(load_successful) {
        std::cout << "loadMap: " << map_streets_database_filename << std::endl;

        //Create empty vector for each intersection
        //Complexity; O(n)
        intersection_street_segments.resize(getNumIntersections());

        //Iterate through all intersections
        //Complexity; O(k), where k is a small constant
        for(int intersection = 0; intersection < getNumIntersections(); intersection++) {
            //Load streetSegmentId for each intersection
            for(int i = 0; i < getNumIntersectionStreetSegment(intersection); ++i) {
                int ss_ids = getIntersectionStreetSegment(intersection, i);
                intersection_street_segments[intersection].push_back(ss_ids);
            }
        }
        //Overall Complexity: O(kn)


        load_successful = true;  //Make sure this is updated to reflect whether
        //loading the map succeeded or failed
        return true;
    } else
        return false;
}

void closeMap() {
    //Clean-up your map related data structures here
    closeStreetDatabase();
    
}

// Returns the distance between two (lattitude,longitude) coordinates in meters
// Speed Requirement --> moderate
double findDistanceBetweenTwoPoints(std::pair<LatLon, LatLon> points) {
    return 0;
}

// Returns the length of the given street segment in meters
// Speed Requirement --> moderate
double findStreetSegmentLength(StreetSegmentIdx street_segment_id) {
    return 0;
}

// Returns the travel time to drive from one end of a street segment in
// to the other, in seconds, when driving at the speed limit
// Note: (time = distance/speed_limit)
// Speed Requirement --> high

double findStreetSegmentTravelTime(StreetSegmentIdx street_segment_id) {
    return 0;
}

// Returns the nearest intersection to the given position
// Speed Requirement --> none
IntersectionIdx findClosestIntersection(LatLon my_position) {
    IntersectionIdx a;
    return a;
}

// Returns the street segments that connect to the given intersection
// Speed Requirement --> high
std::vector<StreetSegmentIdx> findStreetSegmentsOfIntersection(IntersectionIdx intersection_id) {
    /*
    // code provided in the tutorial: passed the function test but failed the performance test, need a better data structure
    std::vector<StreetSegmentIdx> ss_ids;
    for(int i = 0; i < getNumIntersectionStreetSegment(intersection_id); ++i) {
        int ss_id = getIntersectionStreetSegment(intersection_id, i);
        ss_ids.push_back(ss_id);
    }
    return ss_ids;
    */

    return intersection_street_segments[intersection_id];
}

// Returns the street names at the given intersection (includes duplicate
// street names in the returned vector)
// Speed Requirement --> high
std::vector<std::string> findStreetNamesOfIntersection(IntersectionIdx intersection_id) {
    std::vector<std::string> streetNamesOfIntersection;
    for(int intersection = 0; intersection < getNumIntersections(); intersection++){
        streetNamesOfIntersection.push_back(getStreetName(getStreetSegmentInfo(getIntersectionStreetSegment(intersection_id, intersection)).streetID));
        //getStreetName(StreetIdx streetIdx)
        //struct StreetSegmentInfo.streetID
        //streetSegmentInfo getStreetSegmentInfo(StreetSegmentIdx streetSegmentIdx)
        //StreetSegmentIdx getIntersectionStreetSegment(IntersectionIdx intersectionIdx, int segmentNumber);
    }
    return streetNamesOfIntersection;
}

// Returns all intersections reachable by traveling down one street segment
// from the given intersection (hint: you can't travel the wrong way on a
// 1-way street)
// the returned vector should NOT contain duplicate intersections
// Speed Requirement --> high
std::vector<IntersectionIdx> findAdjacentIntersections(IntersectionIdx intersection_id) {
    std::vector<IntersectionIdx> a;
    return a;
}

// Returns all intersections along the a given street
// Speed Requirement --> high
std::vector<IntersectionIdx> findIntersectionsOfStreet(StreetIdx street_id) {
    std::vector<IntersectionIdx> a;
    return a;
}

// Return all intersection ids at which the two given streets intersect
// This function will typically return one intersection id for streets
// that intersect and a length 0 vector for streets that do not. For unusual
// curved streets it is possible to have more than one intersection at which
// two streets cross.
// Speed Requirement --> high
std::vector<IntersectionIdx> findIntersectionsOfTwoStreets(std::pair<StreetIdx, StreetIdx> street_ids) {
    std::vector<IntersectionIdx> a;
    return a;
}

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
std::vector<StreetIdx> findStreetIdsFromPartialStreetName(std::string street_prefix) {
    std::vector<StreetIdx> a;
    return a;
}

// Returns the length of a given street in meters
// Speed Requirement --> high
double findStreetLength(StreetIdx street_id) {
    return 0;
}

// Return the smallest rectangle that contains all the intersections and
// curve points of the given street (i.e. the min,max lattitude
// and longitude bounds that can just contain all points of the street).
// Speed Requirement --> none
LatLonBounds findStreetBoundingBox(StreetIdx street_id) {
    LatLonBounds a;
    return a;
}

// Returns the nearest point of interest of the given name to the given position
// Speed Requirement --> none
POIIdx findClosestPOI(LatLon my_position, std::string POIname) {
    POIIdx a;
    return a;
}

// Returns the area of the given closed feature in square meters
// Assume a non self-intersecting polygon (i.e. no holes)
// Return 0 if this feature is not a closed polygon.
// Speed Requirement --> moderate
double findFeatureArea(FeatureIdx feature_id) {
    return 0;
}
