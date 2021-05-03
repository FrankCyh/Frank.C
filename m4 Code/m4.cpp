#include "m1.h"
#include "m3.h"
#include "m4.h"

#include "StreetsDatabaseAPI.h"

#include "globals.h"
#include "helper.h"


std::vector<CourierSubPath> travelingCourier(
const std::vector<DeliveryInf>& deliveries,
const std::vector<int>& depots,
const float turn_penalty) {
    int num_deliveries = (int)deliveries.size();

    // initialize a vector to save whether a pickUp / dropOff intersection has been visited

    std::vector<bool> visited_pickUp;
    std::vector<bool> visited_dropOff;
    visited_pickUp.resize(num_deliveries);
    visited_dropOff.resize(num_deliveries);
    for(int i = 0; i < num_deliveries; ++i) {
        visited_pickUp[i] = false;
        visited_dropOff[i] = false;
    }

    // pre-compute all shortest travel time and all shortest sub path among pickUps and dropOffs
    // first half of vector is pickUp, second half of vector is dropOff

    std::vector<std::vector<double>> all_shortestTravelTime;
    
    std::vector<IntersectionIdx> all_pickOrDrop_intersections;

    for(int i = 0; i < num_deliveries; ++i) {
        all_pickOrDrop_intersections.push_back(deliveries[i].pickUp);
    }
    for(int i = 0; i < num_deliveries; ++i) {
        all_pickOrDrop_intersections.push_back(deliveries[i].dropOff);
    }

    all_shortestTravelTime.resize(num_deliveries * 2);

    #pragma omp parallel for
    for(int i = 0; i < num_deliveries; ++i) {
        std::vector<double> shortestTravelTime;

        findTravelTime_MultipleDestination(deliveries[i].pickUp, all_pickOrDrop_intersections, shortestTravelTime, turn_penalty);
        all_shortestTravelTime[i] = shortestTravelTime;
    }

    #pragma omp parallel for
    for(int i = 0; i < num_deliveries; ++i) {
        std::vector<double> shortestTravelTime;

        findTravelTime_MultipleDestination(deliveries[i].dropOff, all_pickOrDrop_intersections, shortestTravelTime, turn_penalty);
        all_shortestTravelTime[i + num_deliveries] = shortestTravelTime;
    }
    


    //// find starting depot and first pickUp with the shortest distance ////

    int start_depot_index = 0;  // important info: start depot
    int first_pickUp_index = 0;
    double shortest_first_distance = find_distance_between_intersections(depots[0], deliveries[0].pickUp);

    for(int i = 0; i < depots.size(); ++i) {
        for(int j = 0; j < num_deliveries; ++j) {
            double first_distance = find_distance_between_intersections(depots[i], deliveries[j].pickUp);
            if(first_distance < shortest_first_distance) {
                shortest_first_distance = first_distance;
                start_depot_index = i;
                first_pickUp_index = j;
            }
        }
    }

    // use a vector of delivery index in deliveries[] vector to store the delivery order
    // the first int is the index in deliveries[]; the second bool is true if pickUp, is false if dropOff !!!
    std::vector<pair<int, bool>> delivery_order_in_index;

    // update index result with the first pickUp
    delivery_order_in_index.push_back(std::make_pair(first_pickUp_index, true));
    // update visited pickUp with the first pickUp
    for(int i = 0; i < num_deliveries; ++i) {
        if(deliveries[i].pickUp == deliveries[first_pickUp_index].pickUp) {
            visited_pickUp[i] = true;
        }
    }


    //// find path among pickUps and dropOffs ////

    int packages_to_deliver = num_deliveries;  // -1 whenever a delivery dropOff
    while(packages_to_deliver != 0) {
        double shortest_middle_distance = DBL_MAX;

        IntersectionIdx previous_intersection;
        bool previous_is_pickUp;
        int previous_index = delivery_order_in_index[delivery_order_in_index.size() - 1].first;
        if(delivery_order_in_index[delivery_order_in_index.size() - 1].second == true) {
            previous_intersection = deliveries[previous_index].pickUp;
            previous_is_pickUp = true;
        } else {
            previous_intersection = deliveries[previous_index].dropOff;
            previous_is_pickUp = false;
        }

        int nearest_pickOrDrop_index = NON_EXISTENT;

        bool is_pickUp;  // true if the nearest found point is for pickUp; false if is for dropOff

        // iterate through legal pickUp
        for(int i = 0; i < num_deliveries; ++i) {
            if(visited_pickUp[i] == false) {  // if has not been picked up yet

                double middle_distance;
                if(previous_is_pickUp) {  // from pickUp to pickUp
                    middle_distance = all_shortestTravelTime[previous_index][i];
                } else {  // from dropOff to pickUp
                    middle_distance = all_shortestTravelTime[previous_index + num_deliveries][i];
                }

                //double middle_distance = find_distance_between_intersections(previous_intersection, deliveries[i].pickUp);

                if(middle_distance < shortest_middle_distance) {
                    shortest_middle_distance = middle_distance;
                    nearest_pickOrDrop_index = i;
                    is_pickUp = true;
                }
            }
        }

        // iterate through legal dropOff
        for(int i = 0; i < num_deliveries; ++i) {
            if(visited_pickUp[i] == true && visited_dropOff[i] == false) {  // if has been picked up already AND has not been dropped off yet

                double middle_distance;
                if(previous_is_pickUp) {  // from pickUp to dropOff
                    middle_distance = all_shortestTravelTime[previous_index][i + num_deliveries];
                } else {  // from dropOff to dropOff
                    middle_distance = all_shortestTravelTime[previous_index + num_deliveries][i + num_deliveries];
                }


                //double middle_distance = find_distance_between_intersections(previous_intersection, deliveries[i].pickUp);
                if(middle_distance < shortest_middle_distance) {
                    shortest_middle_distance = middle_distance;
                    nearest_pickOrDrop_index = i;
                    is_pickUp = false;
                }
            }
        }

        // update index result with the nearest legal pickUp or dropOff
        delivery_order_in_index.push_back(std::make_pair(nearest_pickOrDrop_index, is_pickUp));

        // update visited pickUp / visited dropOff with the nearest legal pickUp or dropOff
        if(is_pickUp) {  // the nearest point is pickUp
            for(int i = 0; i < num_deliveries; ++i) {
                if(deliveries[i].pickUp == deliveries[nearest_pickOrDrop_index].pickUp) {
                    visited_pickUp[i] = true;
                }
            }
        } else {  // the nearest point is dropOff
            for(int i = 0; i < num_deliveries; ++i) {
                if(deliveries[i].dropOff == deliveries[nearest_pickOrDrop_index].dropOff && visited_pickUp[i] == true && visited_dropOff[i] == false) {
                    visited_dropOff[i] = true;
                    packages_to_deliver--;  // decrease packages_to_deliver by 1 !!!
                }
            }
        }
    }

    //for (int i = 0; i < delivery_order_in_index.size(); ++i) std::cout << delivery_order_in_index[i].first << ' ';//debug use


    //// find the nearest destination depot ////

    // the final delivery intersection must be dropOff
    IntersectionIdx final_dropOff_intersection = deliveries[delivery_order_in_index[delivery_order_in_index.size() - 1].first].dropOff;

    int dest_depot_index = 0;  // important info: destination depot
    double shortest_final_distance = find_distance_between_intersections(final_dropOff_intersection, depots[0]);

    // iterate through all depots
    for(int i = 0; i < depots.size(); ++i) {
        double final_distance = find_distance_between_intersections(final_dropOff_intersection, depots[i]);
        if(final_distance < shortest_final_distance) {
            shortest_final_distance = final_distance;
            dest_depot_index = i;
        }
    }


    //// change from index result to CourierSubPath result ////

    std::vector<CourierSubPath> result;

    IntersectionIdx start = depots[start_depot_index];
    IntersectionIdx end = deliveries[delivery_order_in_index[0].first].pickUp;
    std::vector<StreetSegmentIdx> path = findPathBetweenIntersections(start, end, turn_penalty);
    CourierSubPath sub_path = { start, end, path };
    result.push_back(sub_path);

    for(int i = 0; i < delivery_order_in_index.size() - 1; ++i) {
        start = (delivery_order_in_index[i].second) ? (deliveries[delivery_order_in_index[i].first].pickUp) : (deliveries[delivery_order_in_index[i].first].dropOff);
        end = (delivery_order_in_index[i + 1].second) ? (deliveries[delivery_order_in_index[i + 1].first].pickUp) : (deliveries[delivery_order_in_index[i + 1].first].dropOff);
        path = findPathBetweenIntersections(start, end, turn_penalty);

        sub_path = { start, end, path };
        result.push_back(sub_path);
    }

    start = deliveries[delivery_order_in_index[delivery_order_in_index.size() - 1].first].dropOff;
    end = depots[dest_depot_index];
    path = findPathBetweenIntersections(start, end, turn_penalty);
    sub_path = { start, end, path };
    result.push_back(sub_path);


    // vector<CourierSubPath> result;//
    return result;  //
}
