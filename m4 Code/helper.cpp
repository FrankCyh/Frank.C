//
// Other functions.cpp
//Created by Frank on 2021/2/5
//
//Purpose of the program:
//

#include "helper.h"
#include "StreetsDatabaseAPI.h"

//Naming Convention: use snake style for helper functions


/*******************************************/
/***** Helper Function for Milestone 1 *****/
/*******************************************/
bool check_duplicate(std::vector<IntersectionIdx> adjacentIntersections, IntersectionIdx intersection_id) {
    bool no_duplicate = false;
    for(int i = 0; i < adjacentIntersections.size(); i++) {
        if(adjacentIntersections[i] == intersection_id)
            return true;
    }
    return no_duplicate;
}

// Remove all spaces in a string and convert all lower case letters to upper case
/*std::string modify_name(std::string name) {
    int len = (int)name.length();
    // Lower case to upper case
    for(int i = 0; i < len; i++) {
        if(name[i] >= 'a' && name[i] <= 'z') {
            name[i] -= 32;
        }
    }
    // Remove space
    for(int i = 0; i < len; i++) {
        if(name[i] == ' ') {
            name.erase(i, 1);
            i--;
        }
    }
    return name;
}*/

// converts character array to string and returns it
std::string convert_to_string(char* a, int size) {
    int i;
    std::string s = "";
    for(i = 0; i < size; i++) {
        s = s + a[i];
    }
    return s;
}

// updates the current max and min latitude and longitude based on the input point
void update_bounding_box(double& maxLat, double& minLat, double& maxLon, double& minLon, LatLon point) {
    if(point.latitude() > maxLat)
        maxLat = point.latitude();
    if(point.latitude() < minLat)
        minLat = point.latitude();
    if(point.longitude() > maxLon)
        maxLon = point.longitude();
    if(point.longitude() < minLon)
        minLon = point.longitude();
}


/*******************************************/
/***** Helper Function for Milestone 2 *****/
/*******************************************/

// Convert between latlon coordinate and xy coordinate
// x coordinate in meters
double x_from_lon(float lon) {
    return lon * kDegreeToRadian * kEarthRadiusInMeters * std::cos(avg_lat * kDegreeToRadian);
}
// y coordinate in meters
double y_from_lat(float lat) {
    return lat * kDegreeToRadian * kEarthRadiusInMeters;
}
// inverse function
double lon_from_x(float x) {
    return x / (kDegreeToRadian * kEarthRadiusInMeters * std::cos(avg_lat * kDegreeToRadian));
}
// inverse function
double lat_from_y(float y) {
    return y / (kDegreeToRadian * kEarthRadiusInMeters);
}

// tilt angle in degrees (0~90 / 270~360) between two points
double angle_between_two_points(double x1, double y1, double x2, double y2) {
    if(x2 - x1 == 0)
        return 90;
    double slope = (y2 - y1) / (x2 - x1);
    double angle = atan(slope) / kDegreeToRadian;
    if(angle < 0)
        angle += 360;
    return angle;
}

// Given two IntersectionIdx, calculate the shortest path between the two intersection by returning a set of IntersectionIdx traversed
vector<IntersectionIdx> find_shortest_path(IntersectionIdx startingPoint, IntersectionIdx endingPoint) {
    vector<IntersectionIdx> path;  // store the path
    vector<double> calculate_minimum_distance;  // used to calculate the closest intersection for each startingPoint
    LatLon endingPoint_position = getIntersectionPosition(endingPoint);
    int numOfTraversedIntersection = 0;
    while(startingPoint != endingPoint) {
        path.push_back(startingPoint);  // starting point used as the intersection id right now
        calculate_minimum_distance.clear();  // clear for each new startingPoint
        /*
        cout << endl
             << "Number of street segment connected: " << getNumIntersectionStreetSegment(startingPoint) << endl;
        */
        numOfTraversedIntersection = 0;
        for(int i = 0; i < getNumIntersectionStreetSegment(startingPoint); i++) {
            StreetSegmentIdx streetSeg_id = getIntersectionStreetSegment(startingPoint, i);
            // return the other end of the street segment
            IntersectionIdx otherEnd = 0;
            if(getStreetSegmentInfo(streetSeg_id).from == startingPoint)
                otherEnd = getStreetSegmentInfo(streetSeg_id).to;
            else
                otherEnd = getStreetSegmentInfo(streetSeg_id).from;
            // calculate the distance from the other end to the endingPoint
            LatLon otherEnd_position;
            if(!check_duplicate(path, otherEnd)) {
                // cout << "OtherEnd: " << otherEnd << endl;
                otherEnd_position = getIntersectionPosition(otherEnd);
                pair<LatLon, LatLon> pair_of_intersection_id(otherEnd_position, endingPoint_position);
                double distance = findDistanceBetweenTwoPoints(pair_of_intersection_id);
                // cout << "Distance of " << i + 1 << "th segment: " << distance << endl;
                calculate_minimum_distance.push_back(distance);
            } else {
                numOfTraversedIntersection++;
                otherEnd_position = getIntersectionPosition(otherEnd);
                pair<LatLon, LatLon> pair_of_intersection_id(otherEnd_position, endingPoint_position);
                double distance = findDistanceBetweenTwoPoints(pair_of_intersection_id) * 100;  //avoid making it largest
                // cout << i + 1 << "th segment have been traversed before" << endl;
                calculate_minimum_distance.push_back(distance);
                if(numOfTraversedIntersection == getNumIntersectionStreetSegment(startingPoint)) {
                    cout << "Unable to find. All street segments connected have been traversed" << endl;
                    return path;
                }
            }
        }
        // calculate the minimum distance
        double minimumDistance = *min_element(calculate_minimum_distance.begin(), calculate_minimum_distance.end());
        // cout << "minimumDistance: " << minimumDistance << endl;
        int new_startingPoint_id = getIndex(calculate_minimum_distance, minimumDistance);
        // cout << "The " << new_startingPoint_id + 1 << "th segment is the shortest segment to the destination" << endl;
        StreetSegmentIdx minimumDistance_streetSeg_id = getIntersectionStreetSegment(startingPoint, new_startingPoint_id);
        // calculate the new startingPoint
        if(getStreetSegmentInfo(minimumDistance_streetSeg_id).from == startingPoint)
            startingPoint = getStreetSegmentInfo(minimumDistance_streetSeg_id).to;
        else
            startingPoint = getStreetSegmentInfo(minimumDistance_streetSeg_id).from;
        // cout << "New starting point is " << startingPoint << endl;
    }
    path.push_back(endingPoint);
    return path;
}

/*****************************************/
/* Helper function of find_shortest_path */
bool check_duplicate_set(set<int> set, int n) {
    int initial_size = (int)set.size();
    set.insert(n);
    return initial_size == set.size();
}

// find index of of the index of an element in the vector with the element's value
IntersectionIdx getIndex(vector<double> v, double K) {
    auto it = find(v.begin(), v.end(), K);

    // If element was found
    if(it != v.end()) {
        // calculating the index of K
        int index = it - v.begin();
        return index;
    } else
        return -1;
}
/****************************************/


// find intersections from streetName input
vector<IntersectionIdx> find_intersection_of_two_streetName(string firstStreetName, string secondStreetName) {
    // initialize a vector containing StreetIdx
    vector<StreetIdx> streetIds_of_firstStreet = findStreetIdsFromPartialStreetName(firstStreetName);
    StreetIdx firstStreetIdx, secondStreetIdx;
    int number;
    if(streetIds_of_firstStreet.size() == 0) {
        cout << "No name found for the 1st street! Try another name." << endl;
        return streetIds_of_firstStreet;  // null vector
    } else if(streetIds_of_firstStreet.size() > 1) {
        for(int i = 0; i < streetIds_of_firstStreet.size(); i++) {
            cout << "The ";
            ordinal_number(i + 1);
            cout << "street found is " << getStreetName(streetIds_of_firstStreet[i]) << endl;
        }
        cout << "*********************************************" << endl
             << "Please select your 1st street by entering the number above:" << endl;
        // ask the user's designated street based on the parital name
        cin >> number;

        std::string random;
        getline(cin, random);

        cout << endl;
        firstStreetIdx = streetIds_of_firstStreet[number - 1];
    } else
        firstStreetIdx = streetIds_of_firstStreet[0];


    // initialize a vector containing StreetIdx
    vector<StreetIdx> streetIds_of_secondStreet(findStreetIdsFromPartialStreetName(secondStreetName));
    if(streetIds_of_secondStreet.size() == 0) {
        cout << "No name found for the 2nd street! Try another name." << endl;
        return streetIds_of_secondStreet;  // null vector
    } else if(streetIds_of_secondStreet.size() > 1) {
        for(int i = 0; i < streetIds_of_secondStreet.size(); i++) {
            cout << "The ";
            ordinal_number(i + 1);
            cout << "street found is " << getStreetName(streetIds_of_secondStreet[i]) << endl;
        }
        cout << "*********************************************" << endl
             << "Please select your 2nd street by entering the number above:" << endl;
        // ask the user's designated street based on the parital name
        cin >> number;

        std::string random;
        getline(cin, random);

        cout << endl;
        secondStreetIdx = streetIds_of_secondStreet[number - 1];
    } else
        secondStreetIdx = streetIds_of_secondStreet[0];

    return find_intersection_of_two_street(firstStreetIdx, secondStreetIdx);
}

/*****************************************/
/* Helper function of find_shortest_path */

// find intersections of two streets
/* 1st version: NOT use global data structure 'street_intersections,'
   therefore we comment out corresponding load function in LoadMap(),
   save a lot of time during LoadMap()*/
vector<IntersectionIdx> find_intersection_of_two_street(StreetIdx firstIdx, StreetIdx secondIdx) {
    vector<IntersectionIdx> allFirstStreetIntersections;
    vector<IntersectionIdx> allSecondStreetIntersections;
    vector<IntersectionIdx> result;

    for(int i = 0; i < getNumStreetSegments(); ++i) {
        StreetIdx id = getStreetSegmentInfo(i).streetID;
        if(id == firstIdx) {
            allFirstStreetIntersections.push_back(getStreetSegmentInfo(i).from);
            allFirstStreetIntersections.push_back(getStreetSegmentInfo(i).to);
        }
        if(id == secondIdx) {
            allSecondStreetIntersections.push_back(getStreetSegmentInfo(i).from);
            allSecondStreetIntersections.push_back(getStreetSegmentInfo(i).to);
        }
    }

    // traverse the two vectors to find the common intersection index
    for(auto i = allFirstStreetIntersections.begin(); i != allFirstStreetIntersections.end(); ++i) {
        for(auto j = allSecondStreetIntersections.begin(); j != allSecondStreetIntersections.end(); ++j) {
            if(*i == *j)
                result.push_back(*i);
        }
    }
    if(result.size() == 0)
        cout << "No common intersection between the two streets "
             << getStreetName(firstIdx) << " and " << getStreetName(secondIdx) << endl;
    return result;
}

/* 2nd version: use global data structure 'street_intersections,'
   CANNOT comment out corresponding load function in LoadMap(),
   take a lot of time during LoadMap()*/
/*vector<IntersectionIdx> find_intersection_of_two_street(StreetIdx firstIdx, StreetIdx secondIdx) {
    vector<IntersectionIdx> firstStreet = street_intersections[firstIdx];
    vector<IntersectionIdx> secondStreet = street_intersections[secondIdx];
    vector<IntersectionIdx> result;
    // traverse the two vectors to find the common intersection index
    for(auto i = firstStreet.begin(); i != firstStreet.end(); i++) {
        for(auto j = secondStreet.begin(); j != secondStreet.end(); j++) {
            if(*i == *j)
                result.push_back(*i);
        }
    }
    if(result.size() == 0)
        cout << "No common intersection between the two streets "
             << getStreetName(firstIdx) << " and " << getStreetName(secondIdx) << endl;
    return result;
}*/

void ordinal_number(int i) {
    int remainder = i - floor(i / 10) * 10;
    if(remainder == 1 && i != 11)
        cout << i << "st ";
    else if(remainder == 2 && i != 12)
        cout << i << "nd ";
    else
        cout << i << "th ";
}

bool identify_map(std::string& identifier) {
    bool map_exists = false;

    if(identifier == "Beijing, China") {
        map_exists = true;
        identifier = "/cad2/ece297s/public/maps/beijing_china.streets.bin";
    }
    if(identifier == "Cairo, Egypt") {
        map_exists = true;
        identifier = "/cad2/ece297s/public/maps/cairo_egypt.streets.bin";
    }
    if(identifier == "Cape-Town, South-Africa") {
        map_exists = true;
        identifier = "/cad2/ece297s/public/maps/cape-town_south-africa.streets.bin";
    }
    if(identifier == "Golden-Horseshoe, Canada") {
        map_exists = true;
        identifier = "/cad2/ece297s/public/maps/golden-horseshoe_canada.streets.bin";
    }
    if(identifier == "Hamilton, Canada") {
        map_exists = true;
        identifier = "/cad2/ece297s/public/maps/hamilton_canada.streets.bin";
    }
    if(identifier == "Hong-Kong, China") {
        map_exists = true;
        identifier = "/cad2/ece297s/public/maps/hong-kong_china.streets.bin";
    }
    if(identifier == "Iceland") {
        map_exists = true;
        identifier = "/cad2/ece297s/public/maps/iceland.streets.bin";
    }
    if(identifier == "Interlaken, Switzerland") {
        map_exists = true;
        identifier = "/cad2/ece297s/public/maps/interlaken_switzerland.streets.bin";
    }
    if(identifier == "London, England") {
        map_exists = true;
        identifier = "/cad2/ece297s/public/maps/london_england.streets.bin";
    }
    if(identifier == "Moscow, Russia") {
        map_exists = true;
        identifier = "/cad2/ece297s/public/maps/moscow_russia.streets.bin";
    }
    if(identifier == "New-Delhi, India") {
        map_exists = true;
        identifier = "/cad2/ece297s/public/maps/new-delhi_india.streets.bin";
    }
    if(identifier == "New-York, USA") {
        map_exists = true;
        identifier = "/cad2/ece297s/public/maps/new-york_usa.streets.bin";
    }
    if(identifier == "Rio-De-Janeiro, Brazil") {
        map_exists = true;
        identifier = "/cad2/ece297s/public/maps/rio-de-janeiro_brazil.streets.bin";
    }
    if(identifier == "Saint-Helena") {
        map_exists = true;
        identifier = "/cad2/ece297s/public/maps/saint-helena.streets.bin";
    }
    if(identifier == "Singapore") {
        map_exists = true;
        identifier = "/cad2/ece297s/public/maps/singapore.streets.bin";
    }
    if(identifier == "Sydney, Australia") {
        map_exists = true;
        identifier = "/cad2/ece297s/public/maps/sydney_australia.streets.bin";
    }
    if(identifier == "Tehran, Iran") {
        map_exists = true;
        identifier = "/cad2/ece297s/public/maps/tehran_iran.streets.bin";
    }
    if(identifier == "Tokyo, Japan") {
        map_exists = true;
        identifier = "/cad2/ece297s/public/maps/tokyo_japan.streets.bin";
    }
    if(identifier == "Toronto, Canada") {
        map_exists = true;
        identifier = "/cad2/ece297s/public/maps/toronto_canada.streets.bin";
    }

    return map_exists;
}

/*****************************************/

/*******************************************/
/***** Helper Function for Milestone 3 *****/
/*******************************************/

// Given a StreetSegmentIdx and an IntersectionIdx(which is oneEnd of this StreetSegment), return the otherEnd of the StreetSegment, can't travel against oneWay
IntersectionIdx getOtherEnd(IntersectionIdx thisEnd, StreetSegmentIdx thisStreetSeg) {
    IntersectionIdx otherEnd;
    if(getStreetSegmentInfo(thisStreetSeg).from == thisEnd)
        otherEnd = getStreetSegmentInfo(thisStreetSeg).to;
    else if(!getStreetSegmentInfo(thisStreetSeg).oneWay)
        otherEnd = getStreetSegmentInfo(thisStreetSeg).from;
    else
        otherEnd = (StreetSegmentIdx)NON_EXISTENT;
    return otherEnd;
}

// Given a StreetSegmentIdx and an IntersectionIdx(which is oneEnd of this StreetSegment), return the otherEnd of the StreetSegment, can travel against oneWay(used in Traceback function)
IntersectionIdx getOtherEnd_Traceback(IntersectionIdx thisEnd, StreetSegmentIdx thisStreetSeg) {
    IntersectionIdx otherEnd;
    if(getStreetSegmentInfo(thisStreetSeg).from == thisEnd)
        otherEnd = getStreetSegmentInfo(thisStreetSeg).to;
    else
        otherEnd = getStreetSegmentInfo(thisStreetSeg).from;
    return otherEnd;
}

// used for tracing back when finally find a path
vector<StreetSegmentIdx> TraceBack(const IntersectionIdx intersect_id_destination, bool finished) {
    list<StreetSegmentIdx> path;

    Node* currentNode = &NodeList[intersect_id_destination];
    IntersectionIdx currentNode_id = intersect_id_destination;

    while(currentNode->reachingEdgeID != NON_EXISTENT) {  // haven't reached the starting point
        StreetSegmentIdx previousEdge = currentNode->reachingEdgeID;
        path.push_front(previousEdge);
        // assign new current node
        currentNode_id = getOtherEnd_Traceback(currentNode_id, currentNode->reachingEdgeID);
        currentNode = &NodeList[currentNode_id];
    }

    vector<StreetSegmentIdx> result(path.begin(), path.end());
    if(finished)
        NodeList.clear();
    return result;
}

// Given a IntersectionIdx, find all the streetSegment connected
vector<StreetSegmentIdx> getConnectEdges(IntersectionIdx intersection_id) {
    vector<IntersectionIdx> result;
    int num_street_seg_connected = getNumIntersectionStreetSegment(intersection_id);
    IntersectionIdx otherEnd;
    for(int i = 0; i < num_street_seg_connected; i++) {
        StreetSegmentIdx connectEdge = getIntersectionStreetSegment(intersection_id, i);
        otherEnd = getOtherEnd(intersection_id, connectEdge);
        if(otherEnd != NON_EXISTENT)
            result.push_back(connectEdge);
    }
    return result;
}

// Given a IntersectionIdx, find all the IntersectionIdx at the otherEnd of streetSegment connected
vector<IntersectionIdx> getOtherEnds(IntersectionIdx thisEnd) {
    vector<IntersectionIdx> result;
    int num_street_seg_connected = getNumIntersectionStreetSegment(thisEnd);
    IntersectionIdx otherEnd;
    for(int i = 0; i < num_street_seg_connected; i++) {
        StreetSegmentIdx street_seg_connected = getIntersectionStreetSegment(thisEnd, i);
        otherEnd = getOtherEnd(thisEnd, street_seg_connected);
        if(otherEnd != NON_EXISTENT)
            result.push_back(otherEnd);
    }
    return result;
}

// print the procedure in expanding the waves
void print_Expand_Delete(IntersectionIdx waveID, bool expand) {
    cout << endl
         << "Wave at Intersection " << waveID;
    if(expand)
        cout << " is expanded" << endl;
    else
        cout << " is deleted" << endl;
}


void update_highlighted_segments(std::vector<StreetSegmentIdx> segs) {
    for(int i = 0; i < segs.size(); ++i) {
        segments[segs[i]].highlight = true;
    }
}


// helper function to find travel direction

// return the direction vector from one point to another
std::pair<double, double> findDirectionVectorBetweenTwoPoints(std::pair<LatLon, LatLon> points) {
    double x1 = x_from_lon(points.first.longitude());
    double y1 = y_from_lat(points.first.latitude());
    double x2 = x_from_lon(points.second.longitude());
    double y2 = y_from_lat(points.second.latitude());

    double len = pow(((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1)), 0.5);
    double dx = (x2 - x1) / len;
    double dy = (y2 - y1) / len;

    return std::make_pair(dx, dy);
}

// return 1 if go straight; 2 if turn slight left; 3 if turn slight right;
// 4 if turn left; 5 if turn right; 6 if turn around
int findTurnDirectionFromTwoVectors(std::pair<double, double> v_from, std::pair<double, double> v_to) {
    // decide whether the turn is slight
    double dot_product = v_from.first * v_to.first + v_from.second * v_to.second;
    // decide left / right turn
    double cross_product = v_from.first * v_to.second - v_from.second * v_to.first;

    // refer to hand-written note
    if(dot_product >= 0.0) {
        if(cross_product >= -0.342 && cross_product <= 0.342) {
            return 1;
        } else if(cross_product > 0.342 && cross_product < 0.9397) {
            return 2;
        } else if(cross_product > -0.9397 && cross_product < -0.342) {
            return 3;
        } else if(cross_product >= 0.9397 && cross_product <= 1.0) {
            return 4;
        } else {
            return 5;
        }
    } else {
        if(cross_product >= 0.5 && cross_product <= 1.0) {
            return 4;
        } else if(cross_product >= -1.0 && cross_product <= -0.5) {
            return 5;
        } else {
            return 6;
        }
    }
}


/*******************************************/
/***** Helper Function for Milestone 4 *****/
/*******************************************/


std::vector<CourierSubPath> findPath_MultipleDestination(
const IntersectionIdx intersect_id_start,
const vector<IntersectionIdx> intersect_id_destinations,
vector<double>& travelTime,
const double turn_penalty) {
    CourierSubPath result_array[(int)intersect_id_destinations.size()];
    // use the array to keep the order of input destination and output CourierSubPath consistent

    travelTime.resize((int)intersect_id_destinations.size());

    int NumDestinationLeft = (int)intersect_id_destinations.size();

    // initialize and clear global variable NodeList each time the function findPathBetweenIntersections() is called
    NodeList.resize(getNumIntersections());

    // use priority_queue to store WaveElems, arrange in increasing order of TravelTime
    priority_queue<WaveElem, std::vector<WaveElem>, CompareTravelTime> Wavefront;

    // Initialize firstwave starting at IntersectionIdx intersect_id_start
    WaveElem firstwave = WaveElem(intersect_id_start, NON_EXISTENT, 0, 0);
    Wavefront.push(firstwave);

    while(!Wavefront.empty()) {
        WaveElem wave = Wavefront.top();  // get next WaveElem in the priority queue
        Wavefront.pop();  // remove top WaveElem

        // currentNode points to the list -> write to the NodeList
        Node* currentNode = &NodeList[wave.nodeID];

        // check if a better path exist, if there is, expand the wave, else, delete this wave
        if(wave.travelTime < currentNode->bestTime) {
            // print_Expand_Delete(wave.nodeID, true);  // print the procedure

            /******** Update currentNode ********/
            currentNode->reachingEdgeID = wave.edgeID;  // update reachingEdge; first node is NON_EXISTENT
            currentNode->bestTime = wave.travelTime;  // update bestTime

            /******** Find the Destination ********/
            for(int i = 0; i < intersect_id_destinations.size(); i++) {
                if(wave.nodeID == intersect_id_destinations[i]) {
                    NumDestinationLeft--;
                    CourierSubPath Subpath = { intersect_id_start, wave.nodeID, TraceBack(intersect_id_destinations[i], true) };
                    travelTime[i] = wave.travelTime;
                    result_array[i] = Subpath;
                    // Find all destination
                    if(NumDestinationLeft == 0) {
                        // Convert from array to vector
                        std::vector<CourierSubPath> result(result_array, result_array + (int)intersect_id_destinations.size());
                        return result;
                    }
                }
            }

            /******** Update Waves at each OutEdge of currentNode ********/
            vector<IntersectionIdx> otherEnds = getOtherEnds(wave.nodeID);
            vector<StreetSegmentIdx> otherEdges = getConnectEdges(wave.nodeID);
            for(int i = 0; i < otherEnds.size(); i++) {
                Node* toNode = &NodeList[otherEnds[i]];

                if(currentNode->lastFromNodeID == otherEnds[i])
                    continue;  // if toNode comes from the same Node last time, then it couldn't be the shortest path -> improve CPU time
                toNode->lastFromNodeID = wave.nodeID;

                // calculate travelTime of the wave
                double time = currentNode->bestTime + findStreetSegmentTravelTime(otherEdges[i]);
                if(currentNode->reachingEdgeID != NON_EXISTENT) {
                    bool changeName = (getStreetSegmentInfo(otherEdges[i]).streetID != getStreetSegmentInfo(currentNode->reachingEdgeID).streetID);
                    if(changeName)  // whether there is a turn
                        time += turn_penalty;
                }

                // In findPath_MultipleDestination, Dijkstra's Algorithm is used, so estimated_time is not used, just the same as time
                double estimated_time = time;

                // push the new waves to the priority queue
                Wavefront.push(WaveElem(otherEnds[i], otherEdges[i], time, estimated_time));
            }
            // print_queue(Wavefront);
        }
        //else
        //    print_Expand_Delete(wave.nodeID, false); // print the procedure
    }

    // Corner Case: path not found
    if(Wavefront.size() == 0) {
        NodeList.clear();
        std::vector<CourierSubPath> result(result_array, result_array + (int)intersect_id_destinations.size());
        return result;
    }
}


double find_distance_between_intersections(IntersectionIdx id1, IntersectionIdx id2) {  //
    LatLon pos1 = getIntersectionPosition(id1);
    LatLon pos2 = getIntersectionPosition(id2);
    pair<LatLon, LatLon> pos_pair = std::make_pair(pos1, pos2);
    return findDistanceBetweenTwoPoints(pos_pair);
}


void findTravelTime_MultipleDestination(
const IntersectionIdx intersect_id_start,
const vector<IntersectionIdx> intersect_id_destinations,
vector<double>& travelTime,
const double turn_penalty) {
    // CourierSubPath result_array[(int)intersect_id_destinations.size()];
    // use the array to keep the order of input destination and output CourierSubPath consistent

    travelTime.resize((int)intersect_id_destinations.size());

    int NumDestinationLeft = (int)intersect_id_destinations.size();

    // initialize and clear global variable NodeList each time the function findPathBetweenIntersections() is called
    std::vector<Node> local_NodeList;
    local_NodeList.resize(getNumIntersections());

    // use priority_queue to store WaveElems, arrange in increasing order of TravelTime
    priority_queue<WaveElem, std::vector<WaveElem>, CompareTravelTime> Wavefront;

    // Initialize firstwave starting at IntersectionIdx intersect_id_start
    WaveElem firstwave = WaveElem(intersect_id_start, NON_EXISTENT, 0, 0);
    Wavefront.push(firstwave);

    while(!Wavefront.empty()) {
        WaveElem wave = Wavefront.top();  // get next WaveElem in the priority queue
        Wavefront.pop();  // remove top WaveElem

        // currentNode points to the list -> write to the NodeList
        Node* currentNode = &local_NodeList[wave.nodeID];

        // check if a better path exist, if there is, expand the wave, else, delete this wave
        if(wave.travelTime < currentNode->bestTime) {
            // print_Expand_Delete(wave.nodeID, true);  // print the procedure

            /******** Update currentNode ********/
            currentNode->reachingEdgeID = wave.edgeID;  // update reachingEdge; first node is NON_EXISTENT
            currentNode->bestTime = wave.travelTime;  // update bestTime

            /******** Find the Destination ********/
            for(int i = 0; i < intersect_id_destinations.size(); i++) {
                if(wave.nodeID == intersect_id_destinations[i]) {
                    NumDestinationLeft--;
                    // CourierSubPath Subpath = { intersect_id_start, wave.nodeID, TraceBack(intersect_id_destinations[i], true) };
                    travelTime[i] = wave.travelTime;
                    // result_array[i] = Subpath;
                    // Find all destination
                    if(NumDestinationLeft == 0) {
                        // Convert from array to vector
                        // std::vector<CourierSubPath> result(result_array, result_array + (int)intersect_id_destinations.size());
                        return;
                    }
                }
            }

            /******** Update Waves at each OutEdge of currentNode ********/
            vector<IntersectionIdx> otherEnds = getOtherEnds(wave.nodeID);
            vector<StreetSegmentIdx> otherEdges = getConnectEdges(wave.nodeID);
            for(int i = 0; i < otherEnds.size(); i++) {
                Node* toNode = &local_NodeList[otherEnds[i]];

                if(currentNode->lastFromNodeID == otherEnds[i])
                    continue;  // if toNode comes from the same Node last time, then it couldn't be the shortest path -> improve CPU time
                toNode->lastFromNodeID = wave.nodeID;

                // calculate travelTime of the wave
                double time = currentNode->bestTime + findStreetSegmentTravelTime(otherEdges[i]);
                if(currentNode->reachingEdgeID != NON_EXISTENT) {
                    bool changeName = (getStreetSegmentInfo(otherEdges[i]).streetID != getStreetSegmentInfo(currentNode->reachingEdgeID).streetID);
                    if(changeName)  // whether there is a turn
                        time += turn_penalty;
                }

                // In findPath_MultipleDestination, Dijkstra's Algorithm is used, so estimated_time is not used, just the same as time
                double estimated_time = time;

                // push the new waves to the priority queue
                Wavefront.push(WaveElem(otherEnds[i], otherEdges[i], time, estimated_time));
            }
            // print_queue(Wavefront);
        }
        //else
        //    print_Expand_Delete(wave.nodeID, false); // print the procedure
    }

    // Corner Case: path not found
    if(Wavefront.size() == 0) {
        local_NodeList.clear();
        // std::vector<CourierSubPath> result(result_array, result_array + (int)intersect_id_destinations.size());
        return;
    }
}


// OP 2
std::vector<StreetSegmentIdx> findPathBetweenIntersections_Dikistra(
const IntersectionIdx intersect_id_start,
const IntersectionIdx intersect_id_destination,
const double turn_penalty) {
    // initialize and clear global variable NodeList each time the function findPathBetweenIntersections() is called
    NodeList.resize(getNumIntersections());

    // use priority_queue to store WaveElems, arrange in increasing order of EstimatedTravelTime
    priority_queue<WaveElem, std::vector<WaveElem>, CompareEstimatedTravelTime> Wavefront;

    // calculate the position of destination for later calculation of estimated time
    LatLon destination_position = getIntersectionPosition(intersect_id_destination);

    // Initialize firstwave starting at IntersectionIdx intersect_id_start
    WaveElem firstwave = WaveElem(intersect_id_start, NON_EXISTENT, 0, 0);
    Wavefront.push(firstwave);


    while(!Wavefront.empty()) {
        WaveElem wave = Wavefront.top();  // get next WaveElem in the priority queue
        Wavefront.pop();  // remove top WaveElem

        // currentNode points to the list -> write to the NodeList
        Node* currentNode = &NodeList[wave.nodeID];

        // check if a better path exist, if there is, expand the wave, else, delete this wave
        if(wave.travelTime < currentNode->bestTime) {
            // print_Expand_Delete(wave.nodeID, true);  // print the procedure

            /******** Update currentNode ********/
            currentNode->reachingEdgeID = wave.edgeID;  // update reachingEdge; first node is NON_EXISTENT
            currentNode->bestTime = wave.travelTime;  // update bestTime

            /******** Draw Waves ********/
            // vector<IntersectionIdx> wavePath = TraceBack(wave.nodeID, false);
            // Draw function

            /******** Find the Destination ********/
            if(wave.nodeID == intersect_id_destination) {
                return TraceBack(intersect_id_destination, true);
            }

            /******** Update Waves at each OutEdge of currentNode ********/
            vector<IntersectionIdx> otherEnds = getOtherEnds(wave.nodeID);
            vector<StreetSegmentIdx> otherEdges = getConnectEdges(wave.nodeID);
            for(int i = 0; i < otherEnds.size(); i++) {
                Node* toNode = &NodeList[otherEnds[i]];

                if(currentNode->lastFromNodeID == otherEnds[i])
                    continue;  // if toNode comes from the same Node last time, then it couldn't be the shortest path -> improve CPU time
                toNode->lastFromNodeID = wave.nodeID;

                // calculate travelTime of the wave
                double time = currentNode->bestTime + findStreetSegmentTravelTime(otherEdges[i]);
                if(currentNode->reachingEdgeID != NON_EXISTENT) {
                    bool changeName = (getStreetSegmentInfo(otherEdges[i]).streetID != getStreetSegmentInfo(currentNode->reachingEdgeID).streetID);
                    if(changeName)  // whether there is a turn
                        time += turn_penalty;
                }

                // calculate estimatedTime of the wave
                /*
                LatLon node_position = getIntersectionPosition(otherEnds[i]);
                pair<LatLon, LatLon> position_pair(node_position, destination_position);
                double extra_time = findDistanceBetweenTwoPoints(position_pair) / 30;  // approximately the speed of highway
                */
                double estimated_time = time;

                // push the new waves to the priority queue
                Wavefront.push(WaveElem(otherEnds[i], otherEdges[i], time, estimated_time));
            }
            // print_queue(Wavefront);
        }
        //else
        //    print_Expand_Delete(wave.nodeID, false); // print the procedure
    }

    // Corner Case: path not found
    if(Wavefront.size() == 0) {
        NodeList.clear();
        return { 0 };
    }
}


vector<StreetSegmentIdx> Compute_shortest_path_time(vector<IntersectionIdx> shortest_path_m2) {
    vector<StreetSegmentIdx> result;
    
    for(int i = 0; i < shortest_path_m2.size(); i++) {
        int num_street_seg_connected = getNumIntersectionStreetSegment(shortest_path_m2[i]);
        IntersectionIdx otherEnd;
        for(int j = 0; j < num_street_seg_connected; j++) {
            StreetSegmentIdx street_seg_connected = getIntersectionStreetSegment(shortest_path_m2[i], j);
            if(getStreetSegmentInfo(street_seg_connected).from == shortest_path_m2[i])
                otherEnd = getStreetSegmentInfo(street_seg_connected).to;
            else
                otherEnd = getStreetSegmentInfo(street_seg_connected).from;
            if(otherEnd == shortest_path_m2[i + 1]){
                result.push_back(street_seg_connected);
//                cout << street_seg_connected << " ";
            }
        }
    }
    return result;
}
