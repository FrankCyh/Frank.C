//
// Other functions.cpp
//Created by Frank on 2021/2/5
//
//Purpose of the program:
//

#include "m1_helper.h"
#include <bits/stdc++.h>

//Naming Convention: use snake style for helper functions

#pragma mark LoadMap Functions

bool check_duplicate(std::vector<IntersectionIdx> adjacentIntersections, IntersectionIdx intersection_id) {
    bool no_duplicate = true;
    for(int i = 0; i < adjacentIntersections.size(); i++) {
        if(adjacentIntersections[i] == intersection_id)
            return false;
    }
    return no_duplicate;
}

// Remove all spaces in a string and convert all lower case letters to upper case
string modify_name(std::string name) {
    int len = name.length();
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
}

// converts character array to string and returns it
string convert_to_string(char* a, int size) {
    int i;
    string s = "";
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

// Given two IntersectionIdx, calculate the shortest path between the two intersection by returning a set of IntersectionIdx traversed
std::set<IntersectionIdx> find_shortest_path(IntersectionIdx startingPoint, IntersectionIdx endingPoint) {
    std::set<IntersectionIdx> path;  // store the path
    vector<double> calculate_minimum_distance;  // used to calculate the closest intersection for each startingPoint
    LatLon endingPoint_position = getIntersectionPosition(endingPoint);
    while(startingPoint != endingPoint) {
        path.insert(startingPoint);  // starting point used as the intersection id right now
        calculate_minimum_distance.clear();  // clear for each new startingPoint
        cout << endl
             << "Number of street segment connected: " << getNumIntersectionStreetSegment(startingPoint) << endl;
        if(getNumIntersectionStreetSegment(startingPoint) == 1) {
            cout << "Unable to find: dead end." << endl;
            break;
        }
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
                otherEnd_position = getIntersectionPosition(otherEnd);
                pair<LatLon, LatLon> pair_of_intersection_id(otherEnd_position, endingPoint_position);
                double distance = findDistanceBetweenTwoPoints(pair_of_intersection_id) * 100;  //avoid making it largest
                cout << i + 1 << "th segment have been traversed before" << endl;
                calculate_minimum_distance.push_back(distance);
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

vector<IntersectionIdx> find_intersection_of_two_street(StreetIdx firstIdx, StreetIdx secondIdx) {
    vector<IntersectionIdx> firstStreet = street_intersections[firstIdx];
    vector<IntersectionIdx> secondStreet = street_intersections[secondIdx];
    vector<IntersectionIdx> result;
    for(auto i = firstStreet.begin(); i != firstStreet.end(); i++) {
        for(auto j = secondStreet.begin(); j != secondStreet.end(); j++) {
            if(*i == *j)
                result.push_back(*i);
        }
    }
    if(result.size() == 0)
        cout << "No common intersection between the two streets " << getStreetName(firstIdx) << " and " << getStreetName(secondIdx) << endl;
    return result;
}

vector<IntersectionIdx> find_intersection_of_two_streetName(string firstStreetName, string secondStreetName) {
    vector<StreetIdx> streetIds_of_firstStreet = findStreetIdsFromPartialStreetName(firstStreetName);
    StreetIdx firstStreetIdx, secondStreetIdx;
    int number;
    if(streetIds_of_firstStreet.size() == 0) {
        cout << "No name found. Try another name" << endl;
        return streetIds_of_firstStreet; // null vector
    } else if(streetIds_of_firstStreet.size() > 1) {
        for(int i = 0; i < streetIds_of_firstStreet.size(); i++) {
            cout << "The ";
            ordinal_number(i + 1);
            cout << "street found is " << getStreetName(streetIds_of_firstStreet[i]) << endl;
        }
        cout << "*********************************************" << endl
             << "Please select a street by entering the number:" << endl;
        cin >> number;
        firstStreetIdx = streetIds_of_firstStreet[number - 1];
    } else if(streetIds_of_firstStreet.size() == 1)
        firstStreetIdx = streetIds_of_firstStreet[0];
    
    
    vector<StreetIdx> streetIds_of_secondStreet = findStreetIdsFromPartialStreetName(secondStreetName);
    if(streetIds_of_secondStreet.size() == 0) {
        cout << "No name found. Try another name" << endl;
        return streetIds_of_secondStreet; // null vector
    } else if(streetIds_of_secondStreet.size() > 1) {
        for(int i = 0; i < streetIds_of_secondStreet.size(); i++) {
            cout << "The ";
            ordinal_number(i + 1);
            cout << "street found is " << getStreetName(streetIds_of_secondStreet[i]);
        }
        cout << "*********************************************" << endl
             << "Please select a street by entering the number:" << endl;
        cin >> number;
        firstStreetIdx = streetIds_of_secondStreet[number - 1];
    } else if(streetIds_of_secondStreet.size() == 1)
        secondStreetIdx = streetIds_of_secondStreet[0];
    
    find_intersection_of_two_street(firstStreetIdx,secondStreetIdx);
}

void ordinal_number(int i) {
    int remainder = i - floor(i / 10) * 10;
    if(remainder == 1 && i != 11)
        cout << "st ";
    else if(remainder == 2 && i != 12)
        cout << "2nd ";
    else
        cout << i << "th";
}

findStreetIdsFromPartialStreetName("Pa");

// put in m1_helper.h
void ordinal_number(int i);
vector<IntersectionIdx> find_intersection_of_two_streetName(string firstStreetName, string secondStreetName);
