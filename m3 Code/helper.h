//
// helper.h
//Created by Frank on 2021/2/23
//
//Purpose of the program:
//

#ifndef helper_h
#define helper_h

#include "LatLon.h"
#include "m1.h"
#include "globals.h"

#include <bits/stdc++.h>

#include <string>
using std::string;
#include <vector>
using std::vector;
#include <set>
using std::set;

/*******************************************/
/***** Helper Function for Milestone 1 *****/
/*******************************************/
bool check_duplicate(std::vector<IntersectionIdx> adjacentIntersections, IntersectionIdx intersection_id);

/*std::string modify_name(std::string name);*/

std::string convert_to_string(char* a, int size);

void update_bounding_box(double & maxLat, double & minLat, double & maxLon, double & minLon, LatLon point);


/*******************************************/
/***** Helper Function for Milestone 2 *****/
/*******************************************/

// Convert between latlon coordinate and xy coordinate
double x_from_lon(float lon);
double y_from_lat(float lat);
double lon_from_x(float x);
double lat_from_y(float y);

// tilt angle in degrees (0~90 / 270~360) between two points
double angle_between_two_points(double x1, double y1, double x2, double y2);

// Find shortest path between two intersetions given two IntersectionIdx
std::set<IntersectionIdx> find_shortest_path(IntersectionIdx startingPoint, IntersectionIdx endingPoint);

// check if value n is already in the set
bool check_duplicate_set(set<int> set, int n);

// get the index of value K in the vector v
IntersectionIdx getIndex(vector<double> v, double K);

// find intersections from streetName input
vector<IntersectionIdx> find_intersection_of_two_streetName(std::string firstStreetName, std::string secondStreetName);

// helper for find_intersection_of_two_streetName: find intersections from street input
vector<IntersectionIdx> find_intersection_of_two_street(StreetIdx firstIdx, StreetIdx secondIdx);

// output helper: print ordinal number
void ordinal_number(int i);

// identify map_path from map name input
bool identify_map(std::string & identifier);

/*******************************************/
/***** Helper Function for Milestone 3 *****/
/*******************************************/

//void print_queue(std::priority_queue<WaveElem, std::vector<WaveElem>, CompareEstimatedTravelTime> queue);

vector<StreetSegmentIdx> TraceBack(const IntersectionIdx intersect_id_destination, bool finished);

vector <IntersectionIdx> getOtherEnds(IntersectionIdx thisEnd);

IntersectionIdx getOtherEnd(IntersectionIdx thisEnd, StreetSegmentIdx thisStreetSeg);

vector <StreetSegmentIdx> getConnectEdges(IntersectionIdx intersection_id);

IntersectionIdx getOtherEnd_Traceback(IntersectionIdx thisEnd, StreetSegmentIdx thisStreetSeg);

void update_highlighted_segments(std::vector<StreetSegmentIdx> segs);

#endif /* helper_h */
