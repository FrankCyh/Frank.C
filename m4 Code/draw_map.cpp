#include "m1.h"
#include "StreetsDatabaseAPI.h"
#include "globals.h"
#include "m2.h"
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "ezgl/point.hpp"
#include "helper.h"
#include "m3.h"
#include <cmath>
#include <fstream>
#include <sstream>

void initial_setup(ezgl::application* application, bool new_window);
void act_on_mouse_click(ezgl::application* app, GdkEventButton* event, double x, double y);
void act_on_key_press(ezgl::application* application, GdkEventKey* event, char* key_name);
void Find_button(GtkWidget* /*widget*/, ezgl::application* app);
void Clear_Highlight_button(GtkWidget* /*widget*/, ezgl::application* app);
void Shortest_Path_button(GtkWidget* /*widget*/, ezgl::application* app);
void Path_by_Clicking_button(GtkWidget* /*widget*/, ezgl::application* app);
void Path_by_Commands_button(GtkWidget* /*widget*/, ezgl::application* app);

void on_dialog_response(GtkDialog* dialog, gint /*response_id*/, gpointer /*user_data*/);
void Travel_Directions_button(GtkWidget* /*widget*/, ezgl::application* app);
void HELP_button(GtkWidget* /*widget*/, ezgl::application* app);


void updateTravelDirectionInfo(std::vector<StreetSegmentIdx> path);


void draw_main_canvas(ezgl::renderer* g);
void draw_intersection(ezgl::renderer* g);
void draw_street(ezgl::renderer* g);
void draw_POI(ezgl::renderer* g);
void draw_feature(ezgl::renderer* g);
bool check_high_zoom(ezgl::renderer* g, double& smallest_dimension, double scale);
void draw_name_minor_street(ezgl::renderer* g);
void draw_name_major_street(ezgl::renderer* g);
void draw_name_POI(ezgl::renderer* g);
void draw_name_major_feature(ezgl::renderer* g);
void draw_open_feature(ezgl::renderer* g, FeatureIdx idx, int width);
void draw_closed_feature(ezgl::renderer* g, FeatureIdx idx);

/*void draw_name_feature(ezgl::renderer *g);*/  // NOT used


// used to save start and destination intersection
IntersectionIdx intersection_start = NON_EXISTENT;
IntersectionIdx intersection_dest = NON_EXISTENT;
// used to represent whether finding path or highlighting intersection upon mouse clicking
bool find_path_upon_click = false;
// used to save travel directions information
std::string travel_direction_info = "No path has been found yet.";


void drawMap() {
    // convert coordinate from latlon to xy coordinate
    double max_lat = getIntersectionPosition(0).latitude();
    double min_lat = max_lat;
    double max_lon = getIntersectionPosition(0).longitude();
    double min_lon = max_lon;

    // Initialize the global variable: vector<intersection_data> intersections
    intersections.resize(getNumIntersections());
    for(int id = 0; id < getNumIntersections(); ++id) {
        intersections[id].position = getIntersectionPosition(id);
        intersections[id].name = getIntersectionName(id);

        // iterate through all intersections to find the boundary
        max_lat = std::max(max_lat, intersections[id].position.latitude());
        min_lat = std::min(min_lat, intersections[id].position.latitude());
        max_lon = std::max(max_lon, intersections[id].position.longitude());
        min_lon = std::min(min_lon, intersections[id].position.longitude());
    }

    // Initialize the global variable avg_lat
    avg_lat = (min_lat + max_lat) / 2;

    // Load vector of structure for drawing
    load_segments();
    load_POIs();
    load_features();

    // Drawing setting
    ezgl::application::settings settings;
    settings.main_ui_resource = "libstreetmap/resources/main.ui";
    settings.window_identifier = "MainWindow";
    settings.canvas_identifier = "MainCanvas";

    ezgl::application application(settings);

    // set up xy coordinate
    ezgl::rectangle initial_world({ x_from_lon(min_lon), y_from_lat(min_lat) }, { x_from_lon(max_lon), y_from_lat(max_lat) });

    application.add_canvas("MainCanvas", draw_main_canvas, initial_world);

    application.run(initial_setup, act_on_mouse_click, nullptr, act_on_key_press);
}

void draw_main_canvas(ezgl::renderer* g) {
    // visualize features
    draw_feature(g);

    // visualize streets
    draw_street(g);

    // visualize intersections
    draw_intersection(g);

    // visualize POIs and names if highly zoomed
    double smallest_dimension;
    if(check_high_zoom(g, smallest_dimension, 3000)) {
        draw_name_major_feature(g);

        draw_name_major_street(g);

        if(check_high_zoom(g, smallest_dimension, 500)) {
            draw_name_minor_street(g);

            if(check_high_zoom(g, smallest_dimension, 300)) {
                //draw_POI(g);
                //draw_name_POI(g);

                /*draw_name_feature(g);*/
            }
        }
    }
    //if(travel_direction_info != "") std::cout << travel_direction_info;  FOR DEBUG USE
}

// intersection image
void draw_intersection(ezgl::renderer* g) {
    g->set_color(ezgl::PLUM);

    for(size_t i = 0; i < intersections.size(); ++i) {
        //float x = intersections[i].x_coord;
        //float y = intersections[i].y_coord;
        float x = x_from_lon(intersections[i].position.longitude());
        float y = y_from_lat(intersections[i].position.latitude());  // need to put calculation into drawMap()

        float INTERSECTION_SIZE = 4;  // draw intersections 4m X 4m
        float width = INTERSECTION_SIZE;
        float height = INTERSECTION_SIZE;

        if(intersections[i].highlight) {
            g->set_color(ezgl::RED);
            width = INTERSECTION_SIZE * 3;
            height = INTERSECTION_SIZE * 3;
        } else {
            g->set_color(ezgl::PLUM);
            width = INTERSECTION_SIZE;
            height = INTERSECTION_SIZE;
        }

        /*if(i == 88253 || i == 24143) {  // for debug use
        	g->set_color(ezgl::RED);
            width = INTERSECTION_SIZE * 100;
            height = INTERSECTION_SIZE * 100;
        }*/

        g->fill_rectangle({ x - width / 2, y - height / 2 }, { x + width / 2, y + height / 2 });

        /*if(intersections[i].is_dest) {
            ezgl::surface* png_surface;
            png_surface = ezgl::renderer::load_png("libstreetmap/resources/dest.png");
            g->draw_surface(png_surface, {x, y});
            ezgl::renderer::free_surface(png_surface);
        }*/
    }
}

// street image
void draw_street(ezgl::renderer* g) {
    g->set_line_cap(ezgl::line_cap::butt);  // Butt ends
    g->set_line_dash(ezgl::line_dash::none);  // Solid line

    // not highlihted segments

    for(StreetSegmentIdx i = 0; i < segments.size(); ++i) {
        int curve_point_num = segments[i].num_curve_points;
        double x_from = x_from_lon(getIntersectionPosition(segments[i].from_point).longitude());
        double y_from = y_from_lat(getIntersectionPosition(segments[i].from_point).latitude());
        double x_to = x_from_lon(getIntersectionPosition(segments[i].to_point).longitude());
        double y_to = y_from_lat(getIntersectionPosition(segments[i].to_point).latitude());

        if(segments[i].speed_limit >= 25) {  // 72km/h, major road
            g->set_color(255, 220, 51, 255);
            g->set_line_width(3);
        } else {
            g->set_color(224, 224, 224, 255);
            g->set_line_width(2);
        }

        if(curve_point_num == 0) {
            // segment has no curve point
            g->draw_line({ x_from, y_from }, { x_to, y_to });
        } else {
            // segment has at least 1 curve point

            // draw from 'from' to 'first curve point'
            g->draw_line({ x_from,
                           y_from },
                         { x_from_lon(getStreetSegmentCurvePoint(i, 0).longitude()),
                           y_from_lat(getStreetSegmentCurvePoint(i, 0).latitude()) });
            // draw among curve points
            for(int j = 0; j < curve_point_num - 1; ++j) {
                g->draw_line({ x_from_lon(getStreetSegmentCurvePoint(i, j).longitude()),
                               y_from_lat(getStreetSegmentCurvePoint(i, j).latitude()) },
                             { x_from_lon(getStreetSegmentCurvePoint(i, j + 1).longitude()),
                               y_from_lat(getStreetSegmentCurvePoint(i, j + 1).latitude()) });
            }
            // draw from 'last curve point' to 'to'
            g->draw_line({ x_from_lon(getStreetSegmentCurvePoint(i, curve_point_num - 1).longitude()),
                           y_from_lat(getStreetSegmentCurvePoint(i, curve_point_num - 1).latitude()) },
                         { x_to,
                           y_to });
        }
    }

    // highlighted segments

    for(StreetSegmentIdx i = 0; i < segments.size(); ++i) {
        if(segments[i].highlight == true) {
            g->set_color(255, 165, 149, 255);
            g->set_line_width(8);

            int curve_point_num = segments[i].num_curve_points;
            double x_from = x_from_lon(getIntersectionPosition(segments[i].from_point).longitude());
            double y_from = y_from_lat(getIntersectionPosition(segments[i].from_point).latitude());
            double x_to = x_from_lon(getIntersectionPosition(segments[i].to_point).longitude());
            double y_to = y_from_lat(getIntersectionPosition(segments[i].to_point).latitude());

            if(curve_point_num == 0) {
                // segment has no curve point
                g->draw_line({ x_from, y_from }, { x_to, y_to });
            } else {
                // segment has at least 1 curve point

                // draw from 'from' to 'first curve point'
                g->draw_line({ x_from,
                               y_from },
                             { x_from_lon(getStreetSegmentCurvePoint(i, 0).longitude()),
                               y_from_lat(getStreetSegmentCurvePoint(i, 0).latitude()) });
                // draw among curve points
                for(int j = 0; j < curve_point_num - 1; ++j) {
                    g->draw_line({ x_from_lon(getStreetSegmentCurvePoint(i, j).longitude()),
                                   y_from_lat(getStreetSegmentCurvePoint(i, j).latitude()) },
                                 { x_from_lon(getStreetSegmentCurvePoint(i, j + 1).longitude()),
                                   y_from_lat(getStreetSegmentCurvePoint(i, j + 1).latitude()) });
                }
                // draw from 'last curve point' to 'to'
                g->draw_line({ x_from_lon(getStreetSegmentCurvePoint(i, curve_point_num - 1).longitude()),
                               y_from_lat(getStreetSegmentCurvePoint(i, curve_point_num - 1).latitude()) },
                             { x_to,
                               y_to });
            }
        }
    }
}

// street name
void draw_name_minor_street(ezgl::renderer* g) {
    g->set_font_size(11);
    g->set_color(80, 80, 80, 255);

    for(StreetIdx i = 0; i < street_streetSegments.size(); ++i) {
        for(int j = 0; j < street_streetSegments[i].size(); j += 4) {
            StreetSegmentIdx id = street_streetSegments[i][j];
            std::string street_name = segments[id].name;

            if(street_name != "<unknown>" && segments[id].speed_limit < 25) {
                int curve_point_num = segments[id].num_curve_points;

                double x_from, y_from, x_to, y_to;
                if(curve_point_num <= 1) {
                    // segment has no curve point
                    x_from = x_from_lon(getIntersectionPosition(segments[id].from_point).longitude());
                    y_from = y_from_lat(getIntersectionPosition(segments[id].from_point).latitude());
                    x_to = x_from_lon(getIntersectionPosition(segments[id].to_point).longitude());
                    y_to = y_from_lat(getIntersectionPosition(segments[id].to_point).latitude());

                } else {
                    // segment has at least 2 curve points
                    x_from = x_from_lon(getStreetSegmentCurvePoint(id, curve_point_num / 2 - 1).longitude());
                    y_from = y_from_lat(getStreetSegmentCurvePoint(id, curve_point_num / 2 - 1).latitude());
                    x_to = x_from_lon(getStreetSegmentCurvePoint(id, curve_point_num / 2).longitude());
                    y_to = y_from_lat(getStreetSegmentCurvePoint(id, curve_point_num / 2).latitude());
                }

                int angle = segments[id].tilt_angle;
                g->set_text_rotation(angle);

                if(segments[id].one_way) {
                    if(x_from_lon(getIntersectionPosition(segments[id].from_point).longitude()) > x_from_lon(getIntersectionPosition(segments[id].to_point).longitude())) {
                        g->draw_text({ (x_from + x_to) / 2, (y_from + y_to) / 2 }, "<< " + street_name + " <<");
                    } else {
                        g->draw_text({ (x_from + x_to) / 2, (y_from + y_to) / 2 }, ">> " + street_name + " >>");
                    }
                } else {
                    g->draw_text({ (x_from + x_to) / 2, (y_from + y_to) / 2 }, street_name);
                }
            }
        }
    }
    g->set_text_rotation(0);
}

void draw_name_major_street(ezgl::renderer* g) {
    g->set_font_size(14);
    g->set_color(204, 51, 0, 255);

    for(StreetIdx i = 0; i < street_streetSegments.size(); ++i) {
        for(int j = 0; j < street_streetSegments[i].size(); j += 80) {
            StreetSegmentIdx id = street_streetSegments[i][j];
            std::string street_name = segments[id].name;

            if(street_name != "<unknown>" && segments[id].speed_limit >= 25) {
                int curve_point_num = segments[id].num_curve_points;

                double x_from, y_from, x_to, y_to;
                if(curve_point_num <= 1) {
                    // segment has no curve point
                    x_from = x_from_lon(getIntersectionPosition(segments[id].from_point).longitude());
                    y_from = y_from_lat(getIntersectionPosition(segments[id].from_point).latitude());
                    x_to = x_from_lon(getIntersectionPosition(segments[id].to_point).longitude());
                    y_to = y_from_lat(getIntersectionPosition(segments[id].to_point).latitude());

                } else {
                    // segment has at least 2 curve points
                    x_from = x_from_lon(getStreetSegmentCurvePoint(id, curve_point_num / 2 - 1).longitude());
                    y_from = y_from_lat(getStreetSegmentCurvePoint(id, curve_point_num / 2 - 1).latitude());
                    x_to = x_from_lon(getStreetSegmentCurvePoint(id, curve_point_num / 2).longitude());
                    y_to = y_from_lat(getStreetSegmentCurvePoint(id, curve_point_num / 2).latitude());
                }

                int angle = segments[id].tilt_angle;
                g->set_text_rotation(angle);

                if(segments[id].one_way) {
                    if(x_from_lon(getIntersectionPosition(segments[id].from_point).longitude()) > x_from_lon(getIntersectionPosition(segments[id].to_point).longitude())) {
                        // use ">" to indicate one way
                        g->draw_text({ (x_from + x_to) / 2, (y_from + y_to) / 2 }, "<< " + street_name + " <<");
                    } else {
                        g->draw_text({ (x_from + x_to) / 2, (y_from + y_to) / 2 }, ">> " + street_name + " >>");
                    }
                } else {
                    g->draw_text({ (x_from + x_to) / 2, (y_from + y_to) / 2 }, street_name);
                }
            }
        }
    }
    g->set_text_rotation(0);
}

// POI image
void draw_POI(ezgl::renderer* g) {
    for(POIIdx i = 0; i < POIs.size(); ++i) {
        ezgl::surface* png_surface;

        if(POIs[i].type == "bar") {
            png_surface = ezgl::renderer::load_png("libstreetmap/resources/bar.png");
        } else if(POIs[i].type == "cafe") {
            png_surface = ezgl::renderer::load_png("libstreetmap/resources/cafe.png");
        } else if(POIs[i].type == "fast_food") {
            png_surface = ezgl::renderer::load_png("libstreetmap/resources/fast_food.png");
        } else if(POIs[i].type == "restaurant" || POIs[i].type == "food_court") {
            png_surface = ezgl::renderer::load_png("libstreetmap/resources/restaurant.png");
        } else if(POIs[i].type == "pub") {
            png_surface = ezgl::renderer::load_png("libstreetmap/resources/pub.png");
        } else if(POIs[i].type == "university" || POIs[i].type == "college") {
            png_surface = ezgl::renderer::load_png("libstreetmap/resources/university.png");
        } else if(POIs[i].type == "school" || POIs[i].type == "language_school" || POIs[i].type == "music_school" || POIs[i].type == "kindergarten") {
            png_surface = ezgl::renderer::load_png("libstreetmap/resources/school.png");
        } else if(POIs[i].type == "library") {
            png_surface = ezgl::renderer::load_png("libstreetmap/resources/library.png");
        } else if(POIs[i].type == "bus_station") {
            png_surface = ezgl::renderer::load_png("libstreetmap/resources/bus.png");
        } else if(POIs[i].type == "ferry_terminal") {
            png_surface = ezgl::renderer::load_png("libstreetmap/resources/ferry.png");
        } else if(POIs[i].type == "fuel") {
            png_surface = ezgl::renderer::load_png("libstreetmap/resources/fuel.png");
        } else if(POIs[i].type == "parking" || POIs[i].type == "bicycle_parking" || POIs[i].type == "motorcycle_parking") {
            png_surface = ezgl::renderer::load_png("libstreetmap/resources/parking.png");
        } else if(POIs[i].type == "atm") {
            png_surface = ezgl::renderer::load_png("libstreetmap/resources/atm.png");
        } else if(POIs[i].type == "bank") {
            png_surface = ezgl::renderer::load_png("libstreetmap/resources/bank.png");
        } else if(POIs[i].type == "clinic" || POIs[i].type == "doctors") {
            png_surface = ezgl::renderer::load_png("libstreetmap/resources/clinic.png");
        } else if(POIs[i].type == "dentist") {
            png_surface = ezgl::renderer::load_png("libstreetmap/resources/dentist.png");
        } else if(POIs[i].type == "hospital") {
            png_surface = ezgl::renderer::load_png("libstreetmap/resources/hospital.png");
        } else if(POIs[i].type == "pharmacy") {
            png_surface = ezgl::renderer::load_png("libstreetmap/resources/pharmacy.png");
        } else if(POIs[i].type == "post_box" || POIs[i].type == "post_office") {
            png_surface = ezgl::renderer::load_png("libstreetmap/resources/post.png");
        } else if(POIs[i].type == "toilets") {
            png_surface = ezgl::renderer::load_png("libstreetmap/resources/toilets.png");
        } else {
            png_surface = ezgl::renderer::load_png("libstreetmap/resources/small_image.png");
        }

        g->draw_surface(png_surface, { x_from_lon(POIs[i].position.longitude()), y_from_lat(POIs[i].position.latitude()) });
        ezgl::renderer::free_surface(png_surface);
    }
}

// POI name
void draw_name_POI(ezgl::renderer* g) {
    g->set_font_size(9);
    g->set_color(ezgl::BLACK);

    for(POIIdx i = 0; i < POIs.size(); ++i) {
        g->draw_text({ x_from_lon(POIs[i].position.longitude()) + 2,
                       y_from_lat(POIs[i].position.latitude()) - 6 },
                     POIs[i].name);
    }
}

// feature image
void draw_open_feature(ezgl::renderer* g, FeatureIdx idx, int width) {
    g->set_line_cap(ezgl::line_cap::butt);  // Butt ends
    g->set_line_dash(ezgl::line_dash::none);  // Solid line
    g->set_line_width(width);

    for(int j = 0; j < features[idx].num_points - 1; ++j) {
        g->draw_line({ x_from_lon(getFeaturePoint(idx, j).longitude()), y_from_lat(getFeaturePoint(idx, j).latitude()) },
                     { x_from_lon(getFeaturePoint(idx, j + 1).longitude()), y_from_lat(getFeaturePoint(idx, j + 1).latitude()) });
    }
}

void draw_closed_feature(ezgl::renderer* g, FeatureIdx idx) {
    vector<ezgl::point2d> points;

    for(int j = 0; j < features[idx].num_points - 1; ++j) {
        points.push_back({ x_from_lon(getFeaturePoint(idx, j).longitude()),
                           y_from_lat(getFeaturePoint(idx, j).latitude()) });
    }

    if(points.size() > 1) {  // if not degenerate feature
        g->fill_poly(points);
    }
}

void draw_feature(ezgl::renderer* g) {
    g->set_font_size(16);

    for(FeatureIdx i = 0; i < features.size(); ++i) {
        if(features[i].type == "unknown") {
            g->set_color(102, 0, 153, 255);
            if(getFeaturePoint(i, 0) == getFeaturePoint(i, features[i].num_points - 1)) {  // close
                draw_closed_feature(g, i);
            } else {  // open
                draw_open_feature(g, i, 2);
            }
        } else if(features[i].type == "island") {
            g->set_color(255, 255, 170, 255);
            draw_closed_feature(g, i);

            if(features[i].highlight) {
                g->set_color(ezgl::BLACK);
                g->draw_text({ x_from_lon(getFeaturePoint(i, 0).longitude()), y_from_lat(getFeaturePoint(i, 0).latitude()) }, features[i].name);
            }
        } else if(features[i].type == "lake") {
            g->set_color(ezgl::LIGHT_SKY_BLUE);
            draw_closed_feature(g, i);
        }
    }

    for(FeatureIdx i = 0; i < features.size(); ++i) {
        if(features[i].type == "park") {
            if(features[i].highlight) {
                g->set_color(234, 204, 255, 255);
            } else {
                g->set_color(153, 255, 153, 255);
            }
            draw_closed_feature(g, i);

            if(features[i].highlight) {
                g->set_color(ezgl::BLACK);
                g->draw_text({ x_from_lon(getFeaturePoint(i, 0).longitude()), y_from_lat(getFeaturePoint(i, 0).latitude()) }, features[i].name);
            }
        } else if(features[i].type == "beach") {
            if(features[i].highlight) {
                g->set_color(234, 204, 255, 255);
            } else {
                g->set_color(ezgl::YELLOW);
            }
            draw_closed_feature(g, i);

            if(features[i].highlight) {
                g->set_color(ezgl::BLACK);
                g->draw_text({ x_from_lon(getFeaturePoint(i, 0).longitude()), y_from_lat(getFeaturePoint(i, 0).latitude()) }, features[i].name);
            }
        } else if(features[i].type == "river") {
            g->set_color(74, 181, 247, 255);
            draw_open_feature(g, i, 4);
        } else if(features[i].type == "building") {
            if(features[i].highlight) {
                g->set_color(234, 204, 255, 255);
            } else {
                g->set_color(192, 192, 192, 255);
            }
            draw_closed_feature(g, i);

            if(features[i].highlight) {
                g->set_color(ezgl::BLACK);
                g->draw_text({ x_from_lon(getFeaturePoint(i, 0).longitude()), y_from_lat(getFeaturePoint(i, 0).latitude()) }, features[i].name);
            }
        } else if(features[i].type == "greenspace") {
            if(features[i].highlight) {
                g->set_color(234, 204, 255, 255);
            } else {
                g->set_color(0, 255, 0, 255);
            }
            draw_closed_feature(g, i);

            if(features[i].highlight) {
                g->set_color(ezgl::BLACK);
                g->draw_text({ x_from_lon(getFeaturePoint(i, 0).longitude()), y_from_lat(getFeaturePoint(i, 0).latitude()) }, features[i].name);
            }
        } else if(features[i].type == "golf course") {
            if(features[i].highlight) {
                g->set_color(234, 204, 255, 255);
            } else {
                g->set_color(180, 255, 0, 255);
            }
            draw_closed_feature(g, i);

            if(features[i].highlight) {
                g->set_color(ezgl::BLACK);
                g->draw_text({ x_from_lon(getFeaturePoint(i, 0).longitude()), y_from_lat(getFeaturePoint(i, 0).latitude()) }, features[i].name);
            }
        } else if(features[i].type == "stream") {
            g->set_color(74, 181, 247, 255);
            draw_open_feature(g, i, 2);
        }

        if(features[i].highlight) {
            g->set_color(ezgl::BLACK);
            g->draw_text({ x_from_lon(getFeaturePoint(i, 0).longitude()), y_from_lat(getFeaturePoint(i, 0).latitude()) }, features[i].name);
        }
    }
}

// feature name
void draw_name_major_feature(ezgl::renderer* g) {
    g->set_font_size(12);

    for(FeatureIdx i = 0; i < features.size(); ++i) {
        if((features[i].name != "<noname>") && (features[i].type == "river" || features[i].area > 10000)) {
            double total_x = 0, total_y = 0, avg_x, avg_y;
            double num_of_points = features[i].num_points;

            // find the approximate center to put the text
            for(int j = 0; j < num_of_points; ++j) {
                total_x += x_from_lon(getFeaturePoint(i, j).longitude());
                total_y += y_from_lat(getFeaturePoint(i, j).latitude());
            }
            avg_x = total_x / num_of_points;
            avg_y = total_y / num_of_points;

            if(features[i].type == "river") {
                g->set_color(ezgl::BLUE);
            } else {
                g->set_color(ezgl::BLACK);
            }
            g->draw_text({ avg_x, avg_y }, features[i].name);
        }
    }
}

/*void draw_name_feature(ezgl::renderer *g) {    // NOT used
	g->set_font_size(12);
	
	for (FeatureIdx i = 0; i < features.size(); ++i) {
		if (features[i].name != "<noname>") {
			double total_x = 0, total_y = 0, avg_x, avg_y;
			double num_of_points = features[i].num_points;
			
			for (int j = 0; j < num_of_points; ++j) {
				total_x += x_from_lon(getFeaturePoint(i, j).longitude());
				total_y += y_from_lat(getFeaturePoint(i, j).latitude());
			}
			avg_x = total_x / num_of_points;
			avg_y = total_y / num_of_points;
			
			if (features[i].type == "river" || features[i].type == "stream") {
				g->set_color(ezgl::BLUE);
			} else {
				g->set_color(ezgl::BLACK);
			}
			g->draw_text({avg_x, avg_y}, features[i].name);
		}
	}
}*/


// detect if highly zoomed to decide whether to draw names
bool check_high_zoom(ezgl::renderer* g, double& smallest_dimension, double scale) {
    ezgl::rectangle visible_world = g->get_visible_world();
    smallest_dimension = std::min(visible_world.height(), visible_world.width());

    if(smallest_dimension < scale) {
        return true;
    } else {
        return false;
    }
}


// update travel_direction_info by a specific path
void updateTravelDirectionInfo(std::vector<StreetSegmentIdx> path) {
    travel_direction_info = "";  // a global string variable, not declared yet
    int turn_direction;  // to indicate turn direction
    int len = 0;  // to store the travel distance along a street (integer)

    for(int i = 0; i < path.size() - 1; ++i) {
        if(getStreetSegmentInfo(path[i]).streetID == getStreetSegmentInfo(path[i + 1]).streetID) {
            len += segment_length[path[i]];

        } else {
            len += segment_length[path[i]];

            travel_direction_info += "Travel along <";
            travel_direction_info += getStreetName(getStreetSegmentInfo(path[i]).streetID);
            travel_direction_info += "> for ";
            travel_direction_info += std::to_string(len);
            travel_direction_info += " m.      ||      ";

            len = 0;


            // current: the previous segment before turn; next: the next segment we want to go after turn
            IntersectionIdx current_from = getStreetSegmentInfo(path[i]).from;
            IntersectionIdx current_to = getStreetSegmentInfo(path[i]).to;
            IntersectionIdx next_from = getStreetSegmentInfo(path[i + 1]).from;
            IntersectionIdx next_to = getStreetSegmentInfo(path[i + 1]).to;

            LatLon current_from_pos, current_to_pos, next_from_pos, next_to_pos;

            if(current_from == next_from) {  // case 1
                if(getStreetSegmentInfo(path[i]).numCurvePoints == 0) {
                    current_from_pos = getIntersectionPosition(current_to);  //
                } else {
                    current_from_pos = getStreetSegmentCurvePoint(path[i], 0);  //first curve point
                }

                current_to_pos = getIntersectionPosition(current_from);
                next_from_pos = getIntersectionPosition(next_from);

                if(getStreetSegmentInfo(path[i + 1]).numCurvePoints == 0) {
                    next_to_pos = getIntersectionPosition(next_to);  //
                } else {
                    next_to_pos = getStreetSegmentCurvePoint(path[i + 1], 0);  //first curve point
                }

            } else if(current_from == next_to) {  // case 2
                if(getStreetSegmentInfo(path[i]).numCurvePoints == 0) {
                    current_from_pos = getIntersectionPosition(current_to);  //
                } else {
                    current_from_pos = getStreetSegmentCurvePoint(path[i], 0);  //first curve point
                }

                current_to_pos = getIntersectionPosition(current_from);
                next_from_pos = getIntersectionPosition(next_to);

                if(getStreetSegmentInfo(path[i + 1]).numCurvePoints == 0) {
                    next_to_pos = getIntersectionPosition(next_from);  //
                } else {
                    next_to_pos = getStreetSegmentCurvePoint(path[i + 1], getStreetSegmentInfo(path[i + 1]).numCurvePoints - 1);  //last curve point
                }

            } else if(current_to == next_from) {  // case 3
                if(getStreetSegmentInfo(path[i]).numCurvePoints == 0) {
                    current_from_pos = getIntersectionPosition(current_from);  //
                } else {
                    current_from_pos = getStreetSegmentCurvePoint(path[i], getStreetSegmentInfo(path[i]).numCurvePoints - 1);  //last curve point
                }

                current_to_pos = getIntersectionPosition(current_to);
                next_from_pos = getIntersectionPosition(next_from);

                if(getStreetSegmentInfo(path[i + 1]).numCurvePoints == 0) {
                    next_to_pos = getIntersectionPosition(next_to);  //
                } else {
                    next_to_pos = getStreetSegmentCurvePoint(path[i + 1], 0);  //first curve point
                }

            } else {  // case 4
                if(getStreetSegmentInfo(path[i]).numCurvePoints == 0) {
                    current_from_pos = getIntersectionPosition(current_from);  //
                } else {
                    current_from_pos = getStreetSegmentCurvePoint(path[i], getStreetSegmentInfo(path[i]).numCurvePoints - 1);  //last curve point
                }

                current_to_pos = getIntersectionPosition(current_to);
                next_from_pos = getIntersectionPosition(next_to);

                if(getStreetSegmentInfo(path[i + 1]).numCurvePoints == 0) {
                    next_to_pos = getIntersectionPosition(next_from);  //
                } else {
                    next_to_pos = getStreetSegmentCurvePoint(path[i + 1], getStreetSegmentInfo(path[i + 1]).numCurvePoints - 1);  //last curve point
                }
            }

            std::pair<LatLon, LatLon> current_points = std::make_pair(current_from_pos, current_to_pos);
            std::pair<double, double> current_vector = findDirectionVectorBetweenTwoPoints(current_points);
            std::pair<LatLon, LatLon> next_points = std::make_pair(next_from_pos, next_to_pos);
            std::pair<double, double> next_vector = findDirectionVectorBetweenTwoPoints(next_points);

            turn_direction = findTurnDirectionFromTwoVectors(current_vector, next_vector);
            if(turn_direction == 1) {
                travel_direction_info += "Go straight onto <";
            } else if(turn_direction == 2) {
                travel_direction_info += "Turn slightly left onto <";
            } else if(turn_direction == 3) {
                travel_direction_info += "Turn slightly right onto <";
            } else if(turn_direction == 4) {
                travel_direction_info += "Turn left onto <";
            } else if(turn_direction == 5) {
                travel_direction_info += "Turn right onto <";
            } else {
                travel_direction_info += "Turn around onto <";
            }
            travel_direction_info += getStreetName(getStreetSegmentInfo(path[i + 1]).streetID);
            travel_direction_info += ">.\n";
        }

        if(i == path.size() - 2) {  // reach the final segment
            travel_direction_info += "Travel along <";
            travel_direction_info += getStreetName(getStreetSegmentInfo(path[i + 1]).streetID);
            travel_direction_info += "> and you will reach your destination.";
        }
    }

    if(travel_direction_info == "") {
        travel_direction_info = "No path has been found.";
    }
}


// setup
void initial_setup(ezgl::application* app, bool /*new_window*/) {
    // Update the status bar message
    app->update_message("Map initialized.");

    // Create buttons
    app->create_button("Find", 6, Find_button);

    app->create_button("Clear Highlight", 7, Clear_Highlight_button);

    //app->create_button("Shortest Path", 8, Shortest_Path_button);

    app->create_button("     Find Path\nby Commands", 8, Path_by_Commands_button);

    app->create_button(" Find Path\nby Clicking", 9, Path_by_Clicking_button);

    app->create_button("    Travel\nDirections", 10, Travel_Directions_button);

    app->create_button("HELP", 11, HELP_button);
}

// display closest intersection position and information about this intersection
void act_on_mouse_click(ezgl::application* app, GdkEventButton* /*event*/, double x, double y) {
    if(find_path_upon_click == false) {  // highlight intersections and features upon mouse click

        app->update_message("Intersection info printed in command window");

        std::cout << "Mouse clicked at(" << x / 1000 << "," << y / 1000 << ") [km]\n";

        LatLon pos = LatLon(lat_from_y(y), lon_from_x(x));

        // highlight clicked intersection
        IntersectionIdx id = findClosestIntersection(pos);
        pair<LatLon, LatLon> point_pair(pos, getIntersectionPosition(id));

        if(findDistanceBetweenTwoPoints(point_pair) < 8) {
            std::cout << "Closest Intersection: " << intersections[id].name << "\n";
            std::cout << "Intersection ID of this intersection: " << id << "\n";

            int num = getNumIntersectionStreetSegment(id);
            std::cout << "This intersection connects " << num << " street segments:\n";
            for(int i = 0; i < num; ++i) {
                std::cout << getStreetName(getStreetSegmentInfo(getIntersectionStreetSegment(id, i)).streetID) << "\n";
                //std::cout << getIntersectionStreetSegment(id, i) << "\n";//
            }

            intersections[id].highlight = true;
        }

        // highlight clicked closed feature
        for(int i = 0; i < features.size(); ++i) {
            double lon_min = getFeaturePoint(i, 0).longitude();
            double lat_min = getFeaturePoint(i, 0).latitude();
            double lon_max = getFeaturePoint(i, 0).longitude();
            double lat_max = getFeaturePoint(i, 0).latitude();

            for(int j = 1; j < features[i].num_points; ++j) {
                if(getFeaturePoint(i, j).longitude() < lon_min)
                    lon_min = getFeaturePoint(i, j).longitude();
                if(getFeaturePoint(i, j).latitude() < lat_min)
                    lat_min = getFeaturePoint(i, j).latitude();
                if(getFeaturePoint(i, j).longitude() > lon_max)
                    lon_max = getFeaturePoint(i, j).longitude();
                if(getFeaturePoint(i, j).latitude() > lat_max)
                    lat_max = getFeaturePoint(i, j).latitude();
            }

            if(lon_from_x(x) >= lon_min && lat_from_y(y) >= lat_min &&
               lon_from_x(x) <= lon_max && lat_from_y(y) <= lat_max) {
                features[i].highlight = true;
            }
        }

    } else {  // find path and highlight street segments upon two mouse clicks

        //std::cout << "Please click on intersections of start and destination in order\n";//

        LatLon pos = LatLon(lat_from_y(y), lon_from_x(x));
        IntersectionIdx id = findClosestIntersection(pos);
        pair<LatLon, LatLon> point_pair(pos, getIntersectionPosition(id));

        vector<StreetSegmentIdx> result1;

        if(findDistanceBetweenTwoPoints(point_pair) < 8) {
            if(intersection_start == NON_EXISTENT) {
                intersection_start = id;
                intersections[id].highlight = true;
                cout << "\nStart is at <" << getIntersectionName(id) << ">\n";  //
            } else {
                intersection_dest = id;
                intersections[id].highlight = true;
                cout << "Destination is at <" << getIntersectionName(id) << ">\n";  //

                // Assume turn penalty is 15 sec
                result1 = findPathBetweenIntersections(intersection_start, intersection_dest, 15);
                //intersections[id].is_dest = true;
                update_highlighted_segments(result1);

                if(result1.size() > 0) {
                    app->update_message("The path has been highlighted");
                    // update travel_direction_info
                    updateTravelDirectionInfo(result1);
                } else {
                    app->update_message("No path has been found");
                }
                result1.clear();
            }
        }
    }

    app->refresh_drawing();
}

//act_on_mouse_move

void act_on_key_press(ezgl::application* app, GdkEventKey* /*event*/, char* key_name) {
    app->update_message("Key Pressed");

    std::cout << "\n"
              << key_name << " key is pressed\n";
}

// buttons
void Find_button(GtkWidget* /*widget*/, ezgl::application* app) {
    // Get a renderer that can be used to draw on top of the main canvas
    //ezgl::renderer *g = app->get_renderer();

    // Update the status bar message
    app->update_message("Find button pressed");
    std::cout << "\nInput names of two streets here, separate by Enter (partial name supported):\n";

    std::string first_name, second_name;
    std::getline(std::cin, first_name);
    std::getline(std::cin, second_name);

    vector<IntersectionIdx> common_intersections = find_intersection_of_two_streetName(first_name, second_name);
    std::cout << "The streets have " << common_intersections.size() << " common intersections\n";
    for(int i = 0; i < common_intersections.size(); ++i) {
        std::cout << intersections[common_intersections[i]].name << " (ID: " << common_intersections[i] << ")"
                  << "\n";
        intersections[common_intersections[i]].highlight = true;
    }

    // Redraw the main canvas
    app->refresh_drawing();
}

void Clear_Highlight_button(GtkWidget* /*widget*/, ezgl::application* app) {
    app->update_message("Clear Highlight button pressed");

    for(int i = 0; i < intersections.size(); ++i) {
        intersections[i].highlight = false;
        //intersections[i].is_dest = false;
    }

    for(int i = 0; i < segments.size(); ++i) {
        segments[i].highlight = false;
    }

    for(int i = 0; i < features.size(); ++i) {
        features[i].highlight = false;
    }

    find_path_upon_click = false;
    intersection_start = NON_EXISTENT;
    intersection_dest = NON_EXISTENT;

    // Redraw the main canvas
    app->refresh_drawing();
    app->update_message("All highlight cleared");
}

void Shortest_Path_button(GtkWidget* /*widget*/, ezgl::application* app) {
    app->update_message("Input 2 intersection IDs in command window as start and destination");
    std::cout << "\nPlease input 2 intersection IDs here as start and destination, separate by Enter.\n";
    std::cout << "(Intersection ID can be displayed by clicking on an intersection):\n";

    int start, dest;
    std::cin >> start >> dest;

    std::set<IntersectionIdx> selected_intersections = find_shortest_path(start, dest);

    /*// Get a renderer that can be used to draw on top of the main canvas
	ezgl::renderer *g = app->get_renderer();
	g->set_line_width(4);
	g->set_color(ezgl::PURPLE);
	double x_from, y_from, x_to, y_to;
	
	for (int i = 0; i < selected_intersections.size() - 1; ++i) {
		x_from = x_from_lon(getIntersectionPosition(selected_intersections[i]).longitude());
		y_from = y_from_lat(getIntersectionPosition(selected_intersections[i]).latitude());
		x_to = x_from_lon(getIntersectionPosition(selected_intersections[i + 1]).longitude());
		y_to = y_from_lat(getIntersectionPosition(selected_intersections[i + 1]).latitude());
		
		g->draw_line({x_from, y_from}, {x_to, y_to});
	}
	
	app->flush_drawing();*/

    std::set<IntersectionIdx>::iterator it = selected_intersections.begin();
    while(it != selected_intersections.end()) {
        int i = *it;
        intersections[i].highlight = true;
        ++it;
    }

    // Redraw the main canvas
    app->refresh_drawing();
}

void Path_by_Clicking_button(GtkWidget* /*widget*/, ezgl::application* app) {
    app->update_message("Please click on intersections of start and destination in order");

    // clear all changes to global variables
    intersection_start = NON_EXISTENT;
    intersection_dest = NON_EXISTENT;
    find_path_upon_click = true;
}

void Path_by_Commands_button(GtkWidget* /*widget*/, ezgl::application* app) {
    app->update_message("Please input your start and destination in the command window");

    // intersection of start

    vector<IntersectionIdx> common_intersections_start;
    do {
        std::cout << "\n>>Input two street names of the start intersection here, separate by Enter (partial name supported):\n";
        std::string first_name, second_name;

        getline(cin, first_name);
        getline(cin, second_name);
        //cout << "first = " << first_name << '^' << "second = " << second_name << endl;  FOR DEBUG USE

        common_intersections_start = find_intersection_of_two_streetName(first_name, second_name);

        /*std::cout << "There are " << common_intersections_start.size() << " common intersections of the two streets\n";
		for(int i = 0; i < common_intersections_start.size(); ++i) {
        	std::cout << intersections[common_intersections_start[i]].name << " (ID: " << common_intersections_start[i] << ")" << "\n";
        	intersections[common_intersections_start[i]].highlight = true;
    	}*/

        if(common_intersections_start.size() > 0) {
            intersection_start = common_intersections_start[0];  //????????????user??????
        } else {
            cout << "\nNo common intersections between two streets! Please input again.\n";
        }
    } while(common_intersections_start.size() == 0);

    // intersection of destination

    vector<IntersectionIdx> common_intersections_dest;
    do {
        std::cout << "\n>>Input two street names of the destination intersection here, separate by Enter (partial name supported):\n";
        std::string first_name, second_name;

        getline(cin, first_name);
        getline(cin, second_name);
        //cout << "first = " << first_name << '^' << "second = " << second_name << endl;  FOR DEBUG USE

        common_intersections_dest = find_intersection_of_two_streetName(first_name, second_name);

        /*std::cout << "There are " << common_intersections_dest.size() << " common intersections of the two streets\n";
		for(int i = 0; i < common_intersections_dest.size(); ++i) {
        	std::cout << intersections[common_intersections_dest[i]].name << " (ID: " << common_intersections_dest[i] << ")" << "\n";
        	intersections[common_intersections_dest[i]].highlight = true;
    	}*/

        if(common_intersections_dest.size() > 0) {
            intersection_dest = common_intersections_dest[0];  //????????????user??????
        } else {
            cout << "\nNo common intersections between two streets! Please input again.\n";
        }
    } while(common_intersections_dest.size() == 0);


    // Assume turn penalty is 15 sec
    vector<StreetSegmentIdx> result2 = findPathBetweenIntersections(intersection_start, intersection_dest, 15);

    intersections[intersection_start].highlight = true;
    intersections[intersection_dest].highlight = true;
    update_highlighted_segments(result2);

    if(result2.size() > 0) {
        // Redraw the main canvas
        app->refresh_drawing();
        app->update_message("The path has been highlighted");
        cout << "The path between <" << getIntersectionName(intersection_start) << "> and <" << getIntersectionName(intersection_dest) << "> has been found.\n";
        cout << "The path has been highlighted in the map.\n";
        // update travel_direction_info
        updateTravelDirectionInfo(result2);
    } else {
        cout << "No path exists between <" << getIntersectionName(intersection_start) << "> and <" << getIntersectionName(intersection_dest) << ">.\n";
    }


    result2.clear();
    // clear all changes to global variables
    intersection_start = NON_EXISTENT;
    intersection_dest = NON_EXISTENT;
    find_path_upon_click = true;
}


/**
 * A callback function for responding to the dialog (pressing buttons or the X button).
 */
void on_dialog_response(GtkDialog* dialog, gint /*response_id*/, gpointer /*user_data*/) {
    /*This will cause the dialog to be destroyed*/
    gtk_widget_destroy(GTK_WIDGET(dialog));
}

void Travel_Directions_button(GtkWidget* /*widget*/, ezgl::application* app) {
    // Update the status bar message
    app->update_message("Displaying travel directions");

    // Redraw the main canvas
    app->refresh_drawing();

    GObject* window;  // the parent window over which to add the dialog
    GtkWidget* content_area;  // the content area of the dialog
    GtkWidget* label;  // the label we will create to display a message in the content area
    GtkWidget* dialog;  // the dialog box we will create

    // get a pointer to the main application window
    window = app->get_object(app->get_main_window_id().c_str());

    // Create the dialog window. Modal windows prevent interaction with other windows in the same application
    dialog = gtk_dialog_new_with_buttons(
    "Travel Directions",
    (GtkWindow*)window,
    GTK_DIALOG_MODAL,
    /*("OK"),
	  GTK_RESPONSE_ACCEPT,*/
    ("CANCEL"),
    GTK_RESPONSE_REJECT,
    NULL);

    // Create a label and attach it to the content area of the dialog
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    label = gtk_label_new(travel_direction_info.c_str());
    gtk_container_add(GTK_CONTAINER(content_area), label);

    // The main purpose of this is to show dialog??s child widget, label
    gtk_widget_show_all(dialog);

    // Connecting the "response" signal from the user to the associated callback function
    g_signal_connect(
    GTK_DIALOG(dialog),
    "response",
    G_CALLBACK(on_dialog_response),
    NULL);
}

void HELP_button(GtkWidget* /*widget*/, ezgl::application* app) {
    app->update_message("Displaying help instructions");
    app->refresh_drawing();

    GObject* window;
    GtkWidget* content_area;
    GtkWidget* label;
    GtkWidget* dialog;

    window = app->get_object(app->get_main_window_id().c_str());

    dialog = gtk_dialog_new_with_buttons(
    "HELP",
    (GtkWindow*)window,
    GTK_DIALOG_MODAL,
    /*("OK"),
	  GTK_RESPONSE_ACCEPT,*/
    ("CANCEL"),
    GTK_RESPONSE_REJECT,
    NULL);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    std::ifstream inFile_help("libstreetmap/resources/HELP_button.txt");
    std::stringstream buffer;
    buffer << inFile_help.rdbuf();
    std::string help(buffer.str());

    label = gtk_label_new(help.c_str());
    inFile_help.close();

    gtk_container_add(GTK_CONTAINER(content_area), label);

    gtk_widget_show_all(dialog);

    g_signal_connect(
    GTK_DIALOG(dialog),
    "response",
    G_CALLBACK(on_dialog_response),
    NULL);
}
