#include "m1.h"
#include "StreetsDatabaseAPI.h"
#include "globals.h"
#include "m2.h"
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "ezgl/point.hpp"
#include "helper.h"
#include <cmath>

void initial_setup(ezgl::application* application, bool new_window);
void act_on_mouse_click(ezgl::application* app, GdkEventButton* event, double x, double y);
void act_on_key_press(ezgl::application *application, GdkEventKey *event, char *key_name);
void Find_button(GtkWidget */*widget*/, ezgl::application *app);
void Clear_Highlight_button(GtkWidget */*widget*/, ezgl::application *app);
void Shortest_Path_button(GtkWidget */*widget*/, ezgl::application *app);


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
                draw_POI(g);
                draw_name_POI(g);

                /*draw_name_feature(g);*/
            }
        }
    }
}

// intersection image
void draw_intersection(ezgl::renderer* g) {
    g->set_color(ezgl::PLUM);

    for(size_t i = 0; i < intersections.size(); ++i) {
        //float x = intersections[i].x_coord;
        //float y = intersections[i].y_coord;
        float x = x_from_lon(intersections[i].position.longitude());
        float y = y_from_lat(intersections[i].position.latitude());  // need to put calculation into drawMap()

        float INTERSECTION_SIZE = 4;  // draw intersections 8m X 8m
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

        g->fill_rectangle({ x - width / 2, y - height / 2 }, { x + width / 2, y + height / 2 });
    }
}

// street image
void draw_street(ezgl::renderer* g) {
    g->set_line_cap(ezgl::line_cap::butt);  // Butt ends
    g->set_line_dash(ezgl::line_dash::none);  // Solid line

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
        } else if(features[i].type == "park") {
        	if (features[i].highlight) {
        		g->set_color(234, 204, 255, 255);
			} else {
				g->set_color(153, 255, 153, 255);
			}			
            draw_closed_feature(g, i);
            
            if (features[i].highlight) {
				g->set_color(ezgl::BLACK);
        		g->draw_text({x_from_lon(getFeaturePoint(i, 0).longitude()), y_from_lat(getFeaturePoint(i, 0).latitude())}, features[i].name);
			}
        } else if(features[i].type == "beach") {
        	if (features[i].highlight) {
        		g->set_color(234, 204, 255, 255);
			} else {
				g->set_color(ezgl::YELLOW);
			}			
            draw_closed_feature(g, i);
            
            if (features[i].highlight) {
				g->set_color(ezgl::BLACK);
        		g->draw_text({x_from_lon(getFeaturePoint(i, 0).longitude()), y_from_lat(getFeaturePoint(i, 0).latitude())}, features[i].name);
			}
        } else if(features[i].type == "lake") {
			g->set_color(ezgl::LIGHT_SKY_BLUE);
            draw_closed_feature(g, i);
        } else if(features[i].type == "river") {
            g->set_color(74, 181, 247, 255);
            draw_open_feature(g, i, 4);
        } else if(features[i].type == "island") {
        	if (features[i].highlight) {
        		g->set_color(234, 204, 255, 255);
			} else {
				g->set_color(ezgl::KHAKI);
			}			
            draw_closed_feature(g, i);
            
            if (features[i].highlight) {
				g->set_color(ezgl::BLACK);
        		g->draw_text({x_from_lon(getFeaturePoint(i, 0).longitude()), y_from_lat(getFeaturePoint(i, 0).latitude())}, features[i].name);
			}
        } else if(features[i].type == "building") {
        	if (features[i].highlight) {
        		g->set_color(234, 204, 255, 255);
			} else {
				g->set_color(192, 192, 192, 255);
			}			
            draw_closed_feature(g, i);
            
            if (features[i].highlight) {
				g->set_color(ezgl::BLACK);
        		g->draw_text({x_from_lon(getFeaturePoint(i, 0).longitude()), y_from_lat(getFeaturePoint(i, 0).latitude())}, features[i].name);
			}
        } else if(features[i].type == "greenspace") {
        	if (features[i].highlight) {
        		g->set_color(234, 204, 255, 255);
			} else {
				g->set_color(0, 255, 0, 255);
			}			
            draw_closed_feature(g, i);
            
            if (features[i].highlight) {
				g->set_color(ezgl::BLACK);
        		g->draw_text({x_from_lon(getFeaturePoint(i, 0).longitude()), y_from_lat(getFeaturePoint(i, 0).latitude())}, features[i].name);
			}
        } else if(features[i].type == "golf course") {
        	if (features[i].highlight) {
        		g->set_color(234, 204, 255, 255);
			} else {
				g->set_color(180, 255, 0, 255);
			}			
            draw_closed_feature(g, i);
            
            if (features[i].highlight) {
				g->set_color(ezgl::BLACK);
        		g->draw_text({x_from_lon(getFeaturePoint(i, 0).longitude()), y_from_lat(getFeaturePoint(i, 0).latitude())}, features[i].name);
			}
        } else if(features[i].type == "stream") {
            g->set_color(74, 181, 247, 255);
            draw_open_feature(g, i, 2);
        }
        
        if (features[i].highlight) {
            g->set_color(ezgl::BLACK);
            g->draw_text({x_from_lon(getFeaturePoint(i, 0).longitude()), y_from_lat(getFeaturePoint(i, 0).latitude())}, features[i].name);
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


// setup
void initial_setup(ezgl::application* app, bool /*new_window*/) {
    // Update the status bar message
    app->update_message("Map initialized.");
    
    // Create buttons
	app->create_button("Find", 6, Find_button);
	
	app->create_button("Clear Highlight", 7, Clear_Highlight_button);
	
	app->create_button("Shortest Path", 8, Shortest_Path_button);
}

// display closest intersection position and information about this intersection
void act_on_mouse_click(ezgl::application* app, GdkEventButton* /*event*/, double x, double y) {
    app->update_message("Intersection info printed in command window");

    std::cout << "Mouse clicked at(" << x / 1000 << "," << y / 1000 << ") [km]\n";

    LatLon pos = LatLon(lat_from_y(y), lon_from_x(x));
    
    // highlight clicked intersection
    IntersectionIdx id = findClosestIntersection(pos);
    pair<LatLon, LatLon> point_pair(pos, getIntersectionPosition(id));
    
    if (findDistanceBetweenTwoPoints(point_pair) < 8) {
    	std::cout << "Closest Intersection: " << intersections[id].name << "\n";
    	std::cout << "Intersection ID of this intersection: " << id << "\n";
    	
    	int num = getNumIntersectionStreetSegment(id);
   		std::cout << "This intersection connects " << num << " street segments:\n";
    	for(int i = 0; i < num; ++i) {
        	std::cout << getStreetName(getStreetSegmentInfo(getIntersectionStreetSegment(id, i)).streetID) << "\n";
    	}
    	
    	intersections[id].highlight = true;
    }
    
    // highlight clicked closed feature
    for (int i = 0; i < features.size(); ++i) {
    	double lon_min = getFeaturePoint(i, 0).longitude();
    	double lat_min = getFeaturePoint(i, 0).latitude();
    	double lon_max = getFeaturePoint(i, 0).longitude();
    	double lat_max = getFeaturePoint(i, 0).latitude();
    	
    	for (int j = 1; j < features[i].num_points; ++j) {
    		if (getFeaturePoint(i, j).longitude() < lon_min) lon_min = getFeaturePoint(i, j).longitude();
    		if (getFeaturePoint(i, j).latitude() < lat_min) lat_min = getFeaturePoint(i, j).latitude();
			if (getFeaturePoint(i, j).longitude() > lon_max) lon_max = getFeaturePoint(i, j).longitude();
    		if (getFeaturePoint(i, j).latitude() > lat_max) lat_max = getFeaturePoint(i, j).latitude();
		}
		
		if (lon_from_x(x) >= lon_min && lat_from_y(y) >= lat_min && 
		    lon_from_x(x) <= lon_max && lat_from_y(y) <= lat_max) {
		    	features[i].highlight = true;
		}
    }

    app->refresh_drawing();
}

//act_on_mouse_move 

void act_on_key_press(ezgl::application *app, GdkEventKey */*event*/, char *key_name) {
	app->update_message("Key Pressed");
	
	std::cout << "\n" << key_name <<" key is pressed\n";
}

// buttons
void Find_button(GtkWidget */*widget*/, ezgl::application *app) {
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
        std::cout << intersections[common_intersections[i]].name << " (ID: " << common_intersections[i] << ")" << "\n";
        intersections[common_intersections[i]].highlight = true;
    }
	
	// Redraw the main canvas
	app->refresh_drawing();
}

void Clear_Highlight_button(GtkWidget */*widget*/, ezgl::application *app) {
	app->update_message("All highlight cleared");
	
	for(int i = 0; i < intersections.size(); ++i) {
        intersections[i].highlight = false;
    }
    
    for(int i = 0; i < segments.size(); ++i) {
    	segments[i].highlight = false;
    }
    
    for(int i = 0; i < features.size(); ++i) {
    	features[i].highlight = false;
    }
	
	// Redraw the main canvas
	app->refresh_drawing();
}

void Shortest_Path_button(GtkWidget */*widget*/, ezgl::application *app) {
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
	while (it != selected_intersections.end()) {
		int i = *it;
		intersections[i].highlight = true;
		++it;
	}
	
	// Redraw the main canvas
	app->refresh_drawing();
}