/*
 * Copyright 2021 University of Toronto
 *
 * Permission is hereby granted, to use this software and associated
 * documentation files (the "Software") in course work at the University
 * of Toronto, or for personal use. Other uses are prohibited, in
 * particular the distribution of the Software either publicly or to third
 * parties.
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <iostream>
#include <string>

#include "m1.h"
#include "m2.h"
#include "m3.h"
#include "helper.h"

//Program exit codes
constexpr int SUCCESS_EXIT_CODE = 0;  //Everyting went OK
constexpr int ERROR_EXIT_CODE = 1;  //An error occured
constexpr int BAD_ARGUMENTS_EXIT_CODE = 2;  //Invalid command-line usage

//The default map to load if none is specified
//std::string default_map_path = "/cad2/ece297s/public/maps/saint-helena.streets.bin";

//std::string default_map_path = "/cad2/ece297s/public/maps/toronto_canada.streets.bin";

std::string default_map_path = "/cad2/ece297s/public/maps/london_england.streets.bin";



// The start routine of your program (main) when you are running your standalone
// mapper program. This main routine is *never called* when you are running
// ece297exercise (the unit tests) -- those tests have their own main routine
// and directly call your functions in /libstreetmap/src/ to test them.
// Don't write any code in this file that you want run by ece297exerise -- it
// will not be called!
int main(int argc, char** argv) {
    std::string map_path;
    if(argc == 1) {
        //Use a default map
        map_path = default_map_path;
    } else if(argc == 2) {
        //Get the map from the command line
        map_path = argv[1];
    } else {
        //Invalid arguments
        std::cerr << "Usage: " << argv[0] << " [map_file_path]\n";
        std::cerr << "  If no map_file_path is provided a default map is loaded.\n";
        return BAD_ARGUMENTS_EXIT_CODE;
    }
    //    }
    //    std::vector<std::string> map_path;
    //    std::string map_path_identifier;
    //    int num_map;
    //
    //    std::cout << "Please input number of maps you want to display: ";
    //    std::cin >> num_map;
    //    map_path.resize(num_map);
    //    std::getline(std::cin, map_path_identifier);  // empty getline
    //
    //    // give user template to input map
    //    std::cout << "Please choose from the following map:\n";
    //    std::cout << "*************************************\n";
    //    std::cout << "Beijing, China\n";
    //    std::cout << "Cairo, Egypt\n";
    //    std::cout << "Cape-Town, South-Africa\n";
    //    std::cout << "Golden-Horseshoe, Canada\n";
    //    std::cout << "Hamilton, Canada\n";
    //    std::cout << "Hong-Kong, China\n";
    //    std::cout << "Iceland\n";
    //    std::cout << "Interlaken, Switzerland\n";
    //    std::cout << "London, England\n";
    //    std::cout << "Moscow, Russia\n";
    //    std::cout << "New-Delhi, India\n";
    //    std::cout << "New-York, USA\n";
    //    std::cout << "Rio-De-Janeiro, Brazil\n";
    //    std::cout << "Saint-Helena\n";
    //    std::cout << "Singapore\n";
    //    std::cout << "Sydney, Australia\n";
    //    std::cout << "Tehran, Iran\n";
    //    std::cout << "Tokyo, Japan\n";
    //    std::cout << "Toronto, Canada\n";
    //    cout << "*************************************\n";
    //
    //    std::cout << "Please input all map names, separate by Enter: \n";
    //    for (int i = 0; i < num_map; ++i) {
    //        std::getline(std::cin, map_path_identifier);
    //        while (!identify_map(map_path_identifier)) {
    //            std::cout << "Invalid map name, please try again:\n";
    //            std::getline(std::cin, map_path_identifier);
    //        }
    //        map_path[i] = map_path_identifier;
    //    }


    bool load_success = loadMap(default_map_path);
    if(!load_success) {
        std::cerr << "\nFailed to load map '" << default_map_path << "'\n";
        return ERROR_EXIT_CODE;
    }
    std::cout << "\nSuccessfully loaded map '" << default_map_path << "'\n";

    //You can now do something with the map data
    // drawMap();

    vector<StreetSegmentIdx> result = findPathBetweenIntersections(228788, 4467, 15.00000000000000000);
    // vector<StreetSegmentIdx> result2 = findPathBetweenIntersections(88253, 24143, 15.00000000000000000);
    // vector<StreetSegmentIdx> result = findPathBetweenIntersections(95180, 26717, 0.00000000000000000);
    
    
    for(int i = 0; i < result.size(); i++){
        cout << result[i] << " ";
    }
    
//    cout << endl << "**************************" << endl;
//    for(int i = 0; i < result2.size(); i++){
//        cout << result2[i] << " ";
//    }
//    cout << endl << "**************************" << endl;
//    for(int i = 0; i < result3.size(); i++){
//        cout << result3[i] << " ";
//    }

    //Clean-up the map data and related data structures
    std::cout << "\nClosing map\n";
    closeMap();


    return SUCCESS_EXIT_CODE;
}


//    auto vec = find_shortest_path(95,300);

//    auto vec1 = findStreetIdsFromPartialStreetName("Dea");
//        for(auto i:vec1){
//       cout << i << endl;
//   }

//  Helena Test case:
//   auto vec2 = find_intersection_of_two_streetName("Ch","cas");

// Toronto Test case:
// auto vec2 = find_intersection_of_two_streetName("Cas", "Ch");
// Toronto: "Baldwin Street","Henry Street");
