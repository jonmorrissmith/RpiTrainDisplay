// Train Display - an RGB matrix departure board for the Raspberry Pi
// Display handler
// Jon Morris Smith - Feb 2025
// Version 1.0
// Instructions, fixes and issues at https://github.com/jonmorrissmith/RGB_Matrix_Train_Departure_Board
//
// With thanks to:
// https://github.com/hzeller/rpi-rgb-led-matrix
// https://github.com/nlohmann/json
//
//
#ifndef TRAIN_SERVICE_DISPLAY_H
#define TRAIN_SERVICE_DISPLAY_H

#include <led-matrix.h>
#include <graphics.h>
#include <thread>
#include <chrono>
#include <string>
#include <atomic>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <tuple>
#include "config.h"
#include "api_client.h"
#include "train_service_parser.h"
#include "display_text.h"

using namespace rgb_matrix;

// Forward declaration for the debug printing macro
extern bool debug_mode;
#define DEBUG_PRINT(x) if(debug_mode) { std::cerr << x << std::endl; }

class TrainServiceDisplay {
private:
    // Hardware components
    RGBMatrix* matrix;     // The RGB matrix object
    FrameCanvas* canvas;   // Matrix canvas for creating content to display
    Font font;             // Font
    FontCache font_cache;  // Cache of font sizes
    int font_baseline;     // Baseline size of the font
    int font_height;       // Height of the font
    int matrix_width;      // Width of the matrix
    int matrix_height;     // Height of the matrix
    
    // Colors
    Color white;
    Color black;
    
    // Data handlers
    TrainServiceParser& parser;                   // JSON for C++ parser
    TrainAPIClient& apiClient;                    // API client for data refresh
    std::atomic<bool> running;                    // Flag to indicate whether the display is running
    const Config& config;                         // Traindisplay configuration object
    
    // Display state
    enum FirstRowState { ETD, COACHES };               // Toggle to show the Estimated Time of Departure or Coaches on the 1st line
    enum ThirdRowState { SECOND_TRAIN, THIRD_TRAIN };  // Toggle to show 2nd or 3rd train on the 3rd line
    enum FourthRowState { LOCATION, MESSAGE };         // Toggle to show the Clock alone or the Clock and message on the 4th line
    
    // Text to be displayed - using the DisplayText class as this holds text, x/y position, width and data version
    DisplayText first_departure;
    DisplayText first_departure_coaches;
    DisplayText first_departure_etd;
    DisplayText calling_points_text;
    DisplayText calling_at_text;
    DisplayText second_departure;
    DisplayText second_departure_etd;
    DisplayText third_departure;
    DisplayText third_departure_etd;
    DisplayText clock_display_text;
    DisplayText nrcc_message_text;
    DisplayText location_name_text;
    
    // flags for refreshing display element
    bool refresh_whole_display;                                     // Yes/No - do we need to refresh the whole display
    bool refresh_whole_display_first_pass_complete;                 // Has the first whole-display refresh completed
    bool refresh_whole_display_second_pass_complete;                // Has the second whole-display refresh completed
    bool refresh_first_departure;                                   // Has the first departure refreshed
    bool refresh_first_departure_etd_coaches;                       // Has the first departure ETD/Coach data refreshed
    bool refresh_first_departure_etd_coaches_first_pass_complete;   // Has first departure ETD/Coach data 1st refresh completed
    bool refresh_2nd_3rd_departure;                                 // Has the 2nd/3rd departure refreshed
    bool refresh_location;                                          // Has the Location refreshed
    
    // Scrolling flags and variables
    bool message_scroll_complete;                     // Yes/No - has the message been shown
    int space_for_calling_points;                     // How much space there is to display the calling points
    bool scroll_calling_points;                       // Yes/No - this is 'true' if the calling points exceed the width of the screen
    int baseline_2nd_3rd_departure_scroll;            // The baseline for the 2nd/3rd departure scroll
    int offset_2nd_3rd_departure_scroll;              // Offset for vertical-scroll of the 2nd/3rd departure
    bool scroll_2nd_3rd_departures;                   // Trigger a vertical-scroll when the 2nd changes to the 3rd departure
    bool scroll_2nd_3rd_departures_first_pass;        // Vertical-scroll needs to be rendered twice to accommodate canvas-swap
    
    // Display options
    bool show_platforms;               // Yes/No - show platforms
    bool show_location;                // Yes/No - show location
    bool platform_selected;            // Yes/No - has a specific platform been selected
    std::string selected_platform;     // The selected platform
    bool has_message;                  // Yes/No - are there messages
    bool show_messages;                // Yes/No - are messages being shown
    
    // Service Data
    size_t num_services;                                             // The number of services available
    TrainServiceParser::TrainServiceInfo first_service_info;         // Full Service data for the first departure
    TrainServiceParser::TrainServiceInfo second_service_info;        // Full Service data for the second departure
    TrainServiceParser::TrainServiceInfo third_service_info;         // Full Service data for the third departure
    
    // State - for toggle on the 1st, 3rd and 4th row and API refresh interval
    size_t first_service_index;                                      // First departure - Index of the departure
    size_t second_service_index;                                     // Second departure - Index of the departure
    size_t third_service_index;                                      // Third departure - Index of the departure
    FirstRowState first_row_state;                                   // First row - ETD-Coaches
    ThirdRowState third_row_state;                                   // Third row - 2nd-3rd departure
    FourthRowState fourth_row_state;                                 // Fourth row - Message-Location/blank
    int ETD_coach_refresh_seconds;                                   // First row - ETD-Coaches
    int third_line_refresh_seconds;                                  // Third row - 2nd-3rd departure
    int Message_Refresh_interval;                                    // Fourth row - Message-Location/blank
    int refresh_interval_seconds;                                    // Data refresh interval
    std::chrono::steady_clock::time_point last_first_row_toggle;     // First row - ETD-Coaches
    std::chrono::steady_clock::time_point last_third_row_toggle;     // Third row - 2nd-3rd departure
    std::chrono::steady_clock::time_point last_fourth_row_toggle;    // Fourth row - Message-Location/blank
    std::chrono::steady_clock::time_point last_refresh;              // Data refresh

    // Helper methods
    void refreshData();                                                   // get JSON departure data from the API
    void updateDisplayContent();                                          // Create the content to be displayed
    void renderFrame();                                                   // Render the data into the matrix display
    void clearArea(int x_origin, int y_origin, int x_size, int y_size);   // Clear an area on the matrix

    // Scrolling functions
    void renderScrollingCallingPoints();
    void renderScrollingMessage();
    void updateScrollPositions();

    // Toggles for display data
    void checkFirstRowStateTransition();   // ETD-Coaches
    void checkThirdRowStateTransition();   // 2nd-3rd departure
    void checkFourthRowStateTransition();  // Message-Location/blank
    void transitionFirstRowState();
    void transitionThirdRowState();
    void transitionFourthRowState();

    // Clock display
    void updateClockDisplay();             // Update the clock

    // For background refresh of API data - this to avoid the display pausing while data refreshes
    std::thread api_thread;                        // Thread for API calls
    std::atomic<bool> data_refresh_pending;        // Flag to indicate data refresh is in progress
    std::atomic<bool> data_refresh_completed;      // Flag to indicate new data is available
    std::mutex api_data_mutex;                     // Mutex for thread-safe access to API data
    std::string new_api_data;                      // Buffer for new API data
    std::atomic<uint64_t> display_data_version;    // Version control of display data
    std::atomic<uint64_t> api_data_version;        // Version control of api data
    
    uint64_t getCurrentDisplayVersion() const {    // Return current version of display data
        return display_data_version.load();
    }
    
    uint64_t getCurrentAPIVersion() const {        // Return current version of display data
        return api_data_version.load();
    }

public:
    TrainServiceDisplay(RGBMatrix* m, TrainServiceParser& p, TrainAPIClient& ac, const Config& cfg);
    ~TrainServiceDisplay(); 
    void run();
    void stop();
};

#endif // TRAIN_SERVICE_DISPLAY_H


