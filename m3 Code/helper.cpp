//
// Other functions.cpp
//Created by Frank on 2021/2/5
//
//Purpose of the program:
//

#include "helper.h"

//Naming Convention: use snake style for helper functions


/*******************************************/
/***** Helper Function for Milestone 1 *****/
/*******************************************/
bool check_duplicate(std::vector<IntersectionIdx> adjacentIntersections, IntersectionIdx intersection_id) {
    bool no_duplicate = true;
    for(int i = 0; i < adjacentIntersections.size(); i++) {
        if(adjacentIntersections[i] == intersection_id)
            return false;
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
std::set<IntersectionIdx> find_shortest_path(IntersectionIdx startingPoint, IntersectionIdx endingPoint) {
    std::set<IntersectionIdx> path;  // store the path
    vector<double> calculate_minimum_distance;  // used to calculate the closest intersection for each startingPoint
    LatLon endingPoint_position = getIntersectionPosition(endingPoint);
    int numOfTraversedIntersection = 0;
    while(startingPoint != endingPoint) {
        path.insert(startingPoint);  // starting point used as the intersection id right now
        calculate_minimum_distance.clear();  // clear for each new startingPoint
        cout << endl
             << "Number of street segment connected: " << getNumIntersectionStreetSegment(startingPoint) << endl;
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
            if(!check_duplicate_set(path, otherEnd)) {
                cout << "OtherEnd: " << otherEnd << endl;
                otherEnd_position = getIntersectionPosition(otherEnd);
                pair<LatLon, LatLon> pair_of_intersection_id(otherEnd_position, endingPoint_position);
                double distance = findDistanceBetweenTwoPoints(pair_of_intersection_id);
                cout << "Distance of " << i + 1 << "th segment: " << distance << endl;
                calculate_minimum_distance.push_back(distance);
            } else {
                numOfTraversedIntersection++;
                otherEnd_position = getIntersectionPosition(otherEnd);
                pair<LatLon, LatLon> pair_of_intersection_id(otherEnd_position, endingPoint_position);
                double distance = findDistanceBetweenTwoPoints(pair_of_intersection_id) * 100;  //avoid making it largest
                cout << i + 1 << "th segment have been traversed before" << endl;
                calculate_minimum_distance.push_back(distance);
                if(numOfTraversedIntersection == getNumIntersectionStreetSegment(startingPoint)) {
                    cout << "Unable to find. All street segments connected have been traversed" << endl;
                    return path;
                }
            }
        }
        // calculate the minimum distance
        double minimumDistance = *min_element(calculate_minimum_distance.begin(), calculate_minimum_distance.end());
        cout << "minimumDistance: " << minimumDistance << endl;
        int new_startingPoint_id = getIndex(calculate_minimum_distance, minimumDistance);
        cout << "The " << new_startingPoint_id + 1 << "th segment is the shortest segment to the destination" << endl;
        StreetSegmentIdx minimumDistance_streetSeg_id = getIntersectionStreetSegment(startingPoint, new_startingPoint_id);
        // calculate the new startingPoint
        if(getStreetSegmentInfo(minimumDistance_streetSeg_id).from == startingPoint)
            startingPoint = getStreetSegmentInfo(minimumDistance_streetSeg_id).to;
        else
            startingPoint = getStreetSegmentInfo(minimumDistance_streetSeg_id).from;
        cout << "New starting point is " << startingPoint << endl;
    }
    path.insert(endingPoint);
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
        cout << "No name found. Try another name" << endl;
        return streetIds_of_firstStreet;  // null vector
    } else if(streetIds_of_firstStreet.size() > 1) {
        for(int i = 0; i < streetIds_of_firstStreet.size(); i++) {
            cout << "The ";
            ordinal_number(i + 1);
            cout << "street found is " << getStreetName(streetIds_of_firstStreet[i]) << endl;
        }
        cout << "*********************************************" << endl
             << "Please select a street by entering the number:" << endl;
        // ask the user's designated street based on the parital name
        cin >> number;
        cout << endl;
        firstStreetIdx = streetIds_of_firstStreet[number - 1];
    } else
        firstStreetIdx = streetIds_of_firstStreet[0];


    // initialize a vector containing StreetIdx
    vector<StreetIdx> streetIds_of_secondStreet(findStreetIdsFromPartialStreetName(secondStreetName));
    if(streetIds_of_secondStreet.size() == 0) {
        cout << "No name found. Try another name" << endl;
        return streetIds_of_secondStreet;  // null vector
    } else if(streetIds_of_secondStreet.size() > 1) {
        for(int i = 0; i < streetIds_of_secondStreet.size(); i++) {
            cout << "The ";
            ordinal_number(i + 1);
            cout << "street found is " << getStreetName(streetIds_of_secondStreet[i]) << endl;
        }
        cout << "*********************************************" << endl
             << "Please select a street by entering the number:" << endl;
        // ask the user's designated street based on the parital name
        cin >> number;
        cout << endl;
        secondStreetIdx = streetIds_of_secondStreet[number - 1];
    } else
        secondStreetIdx = streetIds_of_secondStreet[0];

    return find_intersection_of_two_street(firstStreetIdx, secondStreetIdx);
}

/*****************************************/
/* Helper function of find_shortest_path */

// find intersections of two streets
vector<IntersectionIdx> find_intersection_of_two_street(StreetIdx firstIdx, StreetIdx secondIdx) {
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
}

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


IntersectionIdx getOtherEnd_Traceback(IntersectionIdx thisEnd, StreetSegmentIdx thisStreetSeg) {
    IntersectionIdx otherEnd;
    if(getStreetSegmentInfo(thisStreetSeg).from == thisEnd)
        otherEnd = getStreetSegmentInfo(thisStreetSeg).to;
    else
        otherEnd = getStreetSegmentInfo(thisStreetSeg).from;
    return otherEnd;
}


void update_highlighted_segments(std::vector<StreetSegmentIdx> segs) {
    for(int i = 0; i < segs.size(); ++i) {
        segments[segs[i]].highlight = true;
    }
}
