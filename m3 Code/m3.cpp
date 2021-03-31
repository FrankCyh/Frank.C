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
    double time = 0;

    for(int i = 0; i < path.size(); i++) {
        // calculate the time needed to travel from the start to the end of the i_th street segment
        time += findStreetSegmentTravelTime(path[i]);

        // calculate the time needed if a turn is encountered
        if((i != 0) && (getStreetSegmentInfo(path[i]).streetID != getStreetSegmentInfo(path[i - 1]).streetID)) {
            time += turn_penalty;
        }
    }
    return time;
}  // only used when finally find the shortest path


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
    // calculate the position of destination for calculation of estimated time
    LatLon destination_position = getIntersectionPosition(intersect_id_destination);

    WaveElem firstwave = WaveElem(intersect_id_start, NON_EXISTENT, 0, 0);
    Wavefront.push(firstwave);

    while(!Wavefront.empty()) {
        WaveElem wave = Wavefront.top();  // get next node in the priority queue
        Wavefront.pop();  // remove the mode

        Node* currentNode = & NodeList[wave.nodeID];
        // currentNode points to the list -> write to the NodeList

        if(wave.travelTime < currentNode->bestTime) {  // check if a better path exist, if not, skip this wave
            currentNode->reachingEdgeID = wave.edgeID;
            currentNode->bestTime = wave.travelTime;

            // find the destination
            if(currentNode->nodeID == intersect_id_destination)
                return TraceBack(intersect_id_destination);

            // for each reachingEdge of the current Node
            vector<IntersectionIdx> otherEnds = getOtherEnds(currentNode->nodeID);
            for(int i = 0; i < otherEnds.size(); i++) {
                /*** update the node structure ***/
                Node* toNode = & NodeList[otherEnds[i]];  // Get from Nodelist, write to corresponding intersection index
                // update the reaching edge of each new Nodes
                toNode->reachingEdgeID = getIntersectionStreetSegment(currentNode->nodeID, i);

                /*** Create new waves ***/
                // calculate travelTime of the wave
                double time = currentNode->bestTime + findStreetSegmentTravelTime(toNode->reachingEdgeID);

                // calculate estimatedTime of the wave
                LatLon node_position = getIntersectionPosition(toNode->nodeID);
                pair<LatLon, LatLon> position_pair(node_position, destination_position);
                double extra_time = findDistanceBetweenTwoPoints(position_pair) / 30;
                double estimated_time = time + extra_time;

                // push the new waves to the priority queue
                Wavefront.push(WaveElem((toNode->nodeID), toNode->reachingEdgeID, time, estimated_time));
            }
        }
        // add to the top
    }
    return TraceBack(intersect_id_destination);
}
