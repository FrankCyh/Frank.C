//
// globals.hpp
//Created by Frank on 2021/3/2
//
//Purpose of the program:
//

#ifndef globals_hpp
#define globals_hpp

#include "m1.h"  // "StreetsDatabaseAPI.h"
#include "m1_helper.h"
#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <math.h>
#include <map>
#include <set>
#include <string>

using std::vector;
using std::cin;
using std::cout;
using std::endl;
using std::multimap;
using std::set;
using std::pair;
using std::string;

extern vector<vector<StreetSegmentIdx>> intersection_streetSegments;

typedef double SmallSegmentsLength;
extern vector<vector<SmallSegmentsLength>> streetSegments_smallSegmentsLength;

extern vector<double> segment_length;

extern vector<double> segment_travel_time;

extern vector<vector<StreetSegmentIdx>> street_streetSegments;

extern vector<vector<IntersectionIdx>> street_intersections;

extern multimap<string, StreetIdx> partial_street_names;

void streetIntersection_to_streetSegment();
void streetSegments_to_smallSegments();
void streetSegments_to_segmentLength();
void streetSegments_to_segmentTravelTime();
void street_to_streetSegments();
void street_to_intersections();
void streetName_to_partialStreetNames();

#endif /* globals_hpp */
