//
// globals.hpp
//Created by Frank on 2021/3/2
//


#ifndef globals_hpp
#define globals_hpp

#include "m1.h"  // "StreetsDatabaseAPI.h"
#include "helper.h"

#include <iostream>
using std::cin;
using std::cout;
using std::endl;

// Include all the container library in globals.h
#include <string>
using std::string;

#include <vector>
using std::vector;

#include <map>
using std::multimap;

#include <set>
using std::set;

#include <queue>
using std::queue;
using std::priority_queue;

#include <utility>
using std::pair;

#include <list>
using std::list;

#include <math.h>


// use keyword "extern" to make these global variables accessible throughout the files, declared elsewhere(in globals.cpp)

/*******************************************/
/* global variable created for Milestone 1 */
/*******************************************/
extern vector<vector<StreetSegmentIdx>> intersection_streetSegments;

typedef double SmallSegmentsLength;
extern vector<vector<SmallSegmentsLength>> streetSegments_smallSegmentsLength;

extern vector<double> segment_length;

extern vector<double> segment_travel_time;

extern vector<vector<StreetSegmentIdx>> street_streetSegments;

extern vector<vector<IntersectionIdx>> street_intersections;

extern multimap<std::string, StreetIdx> partial_street_names;


// function for Declaration of global variables
void streetIntersection_to_streetSegment();
void streetSegments_to_smallSegments();
void streetSegments_to_segmentLength();
void streetSegments_to_segmentTravelTime();
void street_to_streetSegments();
void street_to_intersections();
void streetName_to_partialStreetNames();


/*******************************************/
/* global variable created for Milestone 2 */
/*******************************************/

// intersection structure
struct intersection_data {
    LatLon position;
    std::string name;
    bool highlight = false;
};

extern std::vector<intersection_data> intersections;

// average latitude
extern double avg_lat;

// street segment structure
struct segment_data {
    std::string name;
    IntersectionIdx from_point, to_point;
    bool one_way;
    int num_curve_points;
    double speed_limit;
    StreetIdx street_id;

    int tilt_angle;  // manually set to int

    OSMID way_OSM_id;
    bool highlight = false;
};

extern std::vector<segment_data> segments;

// POI
struct POI_data {
    std::string name;
    std::string type;
    LatLon position;

    OSMID OSM_id;
    bool highlight = false;
};

extern std::vector<POI_data> POIs;

// feature
struct feature_data {
    std::string name;
    std::string type;
    int num_points;

    double area;

    TypedOSMID OSM_id;
    bool highlight = false;
};

extern std::vector<feature_data> features;

// function for Declaration of global variables
void load_segments();
void load_POIs();
void load_features();


/*******************************************/
/* global variable created for Milestone 3 */
/*******************************************/

#define NON_EXISTENT -1

class WaveElem {
    public:
    IntersectionIdx nodeID;
    StreetSegmentIdx edgeID;
    double travelTime;
    double estimatedTime;
    WaveElem(IntersectionIdx node, StreetSegmentIdx edge, double time, double estimated_time) {
        nodeID = node;
        edgeID = edge;
        travelTime = time;
        estimatedTime = estimated_time;
    }
};

struct Node {
    public:
//    IntersectionIdx nodeID;
    IntersectionIdx lastFromNodeID = NON_EXISTENT;
    StreetSegmentIdx reachingEdgeID = NON_EXISTENT;  // parent edge, intialize to NON_EXISTENT at the beginning
    double bestTime = INT32_MAX;  // initialize to INT8_MAX
//    Node() {
//        nodeID = 0;
//        reachingEdgeID = NON_EXISTENT;
//        bestTime = INT32_MAX;
//        lastFromNodeID = NON_EXISTENT;
//    }
//    Node(IntersectionIdx node_id) {
//        nodeID = node_id;
//        reachingEdgeID = NON_EXISTENT;
//        lastFromNodeID = NON_EXISTENT;
//        bestTime = INT32_MAX;
//    };
};

extern std::vector<Node> NodeList;

void load_NodeList();

void clear_NodeList();

// used in the priority queue
struct CompareEstimatedTravelTime {
    bool operator()(WaveElem const& element1, WaveElem const& element2) {
        // elelment arranged in increasing order of
        return element1.estimatedTime > element2.estimatedTime;
    }
};



void print_queue(std::priority_queue<WaveElem, std::vector<WaveElem>, CompareEstimatedTravelTime> queue);




#endif /* globals_hpp */
