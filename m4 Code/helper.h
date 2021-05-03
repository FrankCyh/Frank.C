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
#include "m3.h"
#include "m4.h"
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

void update_bounding_box(double& maxLat, double& minLat, double& maxLon, double& minLon, LatLon point);


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
vector<IntersectionIdx> find_shortest_path(IntersectionIdx startingPoint, IntersectionIdx endingPoint);

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
bool identify_map(std::string& identifier);

/*******************************************/
/***** Helper Function for Milestone 3 *****/
/*******************************************/

// Given a StreetSegmentIdx and an IntersectionIdx(which is oneEnd of this StreetSegment), return the otherEnd of the StreetSegment, can't travel against oneWay
IntersectionIdx getOtherEnd(IntersectionIdx thisEnd, StreetSegmentIdx thisStreetSeg);

// Given a StreetSegmentIdx and an IntersectionIdx(which is oneEnd of this StreetSegment), return the otherEnd of the StreetSegment, can travel against oneWay(used in Traceback function)
IntersectionIdx getOtherEnd_Traceback(IntersectionIdx thisEnd, StreetSegmentIdx thisStreetSeg);

// used for tracing back when finally find a path
vector<StreetSegmentIdx> TraceBack(const IntersectionIdx intersect_id_destination, bool finished);

// Given a IntersectionIdx, find all the streetSegment connected
vector<StreetSegmentIdx> getConnectEdges(IntersectionIdx intersection_id);

// Given a IntersectionIdx, find all the IntersectionIdx at the otherEnd of streetSegment connected
vector<IntersectionIdx> getOtherEnds(IntersectionIdx thisEnd);

// print the procedure in expanding the waves
void print_Expand_Delete(IntersectionIdx waveID, bool expand);


void update_highlighted_segments(std::vector<StreetSegmentIdx> segs);

// helper function to find travel direction
// return the direction vector from one point to another
std::pair<double, double> findDirectionVectorBetweenTwoPoints(std::pair<LatLon, LatLon> points);

// return 1 if go straight; 2 if turn slight left; 3 if turn slight right;
// 4 if turn left; 5 if turn right; 6 if turn around
int findTurnDirectionFromTwoVectors(std::pair<double, double> v_from, std::pair<double, double> v_to);

/*******************************************/
/***** Helper Function for Milestone 4 *****/
/*******************************************/

std::vector<CourierSubPath> findPath_MultipleDestination(
const IntersectionIdx intersect_id_start,
const vector<IntersectionIdx> intersect_id_destinations,
vector<double>& travelTime,
const double turn_penalty);

void findTravelTime_MultipleDestination(
const IntersectionIdx intersect_id_start,
const vector<IntersectionIdx> intersect_id_destinations,
vector<double>& travelTime,
const double turn_penalty);

double find_distance_between_intersections(IntersectionIdx id1, IntersectionIdx id2);

// OP2
std::vector<StreetSegmentIdx> findPathBetweenIntersections_Dikistra(
                  const IntersectionIdx intersect_id_start,
                  const IntersectionIdx intersect_id_destination,
                  const double turn_penalty);


vector<StreetSegmentIdx> Compute_shortest_path_time(vector<IntersectionIdx> shortest_path_m2);


#endif /* helper_h */

