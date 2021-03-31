//
// globals.hpp
//Created by Frank on 2021/3/2
//


#ifndef globals_hpp
#define globals_hpp

#include "m1.h"  // "StreetsDatabaseAPI.h"
#include "m1_helper.h"

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

#include <utility>
using std::pair;

#include <math.h>


// use keyword "extern" to make these global variables accessible throughout the files, declared elsewhere(in globals.cpp)
extern vector<vector<StreetSegmentIdx>> intersection_streetSegments;

typedef double SmallSegmentsLength;
extern vector<vector<SmallSegmentsLength>> streetSegments_smallSegmentsLength;

extern vector<double> segment_length;

extern vector<double> segment_travel_time;

extern vector<vector<StreetSegmentIdx>> street_streetSegments;

extern vector<vector<IntersectionIdx>> street_intersections;

extern multimap<string, StreetIdx> partial_street_names;

extern struct intersection_data;

// function for Declaration of global variables
void streetIntersection_to_streetSegment();
void streetSegments_to_smallSegments();
void streetSegments_to_segmentLength();
void streetSegments_to_segmentTravelTime();
void street_to_streetSegments();
void street_to_intersections();
void streetName_to_partialStreetNames();

#endif /* globals_hpp */
