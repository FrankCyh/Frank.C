#include "m1.h"
#include "m3.h"
#include "StreetsDatabaseAPI.h"

#include "globals.h"
#include "helper.h"

// Returns the time required to travel along the path specified, in seconds.
// The path is given as a vector of street segment ids, and this function can
// assume the vector either forms a legal path or has size == 0.  The travel
// time is the sum of the length/speed-limit of each street segment, plus the
// given turn_penalty (in seconds) per turn implied by the path.  If there is
// no turn, then there is no penalty. Note that whenever the street id changes
// (e.g. going from Bloor Street West to Bloor Street East) we have a turn.
double computePathTravelTime(const std::vector<StreetSegmentIdx>& path, const double turn_penalty) {
    double time = 0;  // initialize to 0; pathTravelTime = 0 is the vector is empty

    for(int i = 0; i < path.size(); i++) {
        time += findStreetSegmentTravelTime(path[i]);

        // turn encountered: streetID is of last and this streetSegment is not the same
        if((i != 0) && (getStreetSegmentInfo(path[i]).streetID != getStreetSegmentInfo(path[i - 1]).streetID)) {
            time += turn_penalty;
        }
    }
    return time;
}  // not used in findPathBetweenIntersections(), only used to calculate the final time


// Returns a path (route) between the start intersection and the destination
// intersection, if one exists. This routine should return the shortest path
// between the given intersections, where the time penalty to turn right or
// left is given by turn_penalty (in seconds).  If no path exists, this routine
// returns an empty (size == 0) vector.  If more than one path exists, the path
// with the shortest travel time is returned. The path is returned as a vector
// of street segment ids; traversing these street segments, in the returned
// order, would take one from the start to the destination intersection.
std::vector<StreetSegmentIdx> findPathBetweenIntersections(
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
                LatLon node_position = getIntersectionPosition(otherEnds[i]);
                pair<LatLon, LatLon> position_pair(node_position, destination_position);
                double extra_time = findDistanceBetweenTwoPoints(position_pair) / 30;  // approximately the speed of highway
                double estimated_time = time + extra_time;

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
