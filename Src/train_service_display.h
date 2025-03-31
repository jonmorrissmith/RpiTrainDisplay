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
    enum FourthRowState { CLOCK, MESSAGE };            // Toggle to show the Clock alone or the Clock and message on the 4th line
    
    // Display content
    std::string top_line;              // 1st departure
    std::string top_line_coaches;      // 1st departure - number of coaches
    std::string calling_points;        // 1st departure - calling points
    std::string second_line;           // Calling points
    std::string third_line;            // Toggle between 2nd and 3rd departure
    std::string clock_display;         // The clock
    std::string nrcc_message;          // Network Rail messages
    std::string location_name;         // Location name for the departure board

    bool show_platforms;               // Yes/No - show platforms
    bool show_location;                // Yes/No - show location
    bool platform_selected;            // Yes/No - has a specific platform been selected
    std::string selected_platform;     // The selected platform
    bool has_message;                  // Yes/No - are there messages
    bool show_messages;                // Yes/No - are messages being shown
    bool message_scroll_complete;      // Yes/No - has the message been shown

    size_t num_services;               // The number of services available
    
    std::tuple<std::string, std::string, std::string, std::string, std::string, std::string> first_service;
    
    // Service Data
    TrainServiceParser::TrainServiceInfo first_service_info;       // Full Service data for the first departure
    TrainServiceParser::TrainServiceInfo second_service_info;      // Full Service data for the second departure
    TrainServiceParser::TrainServiceInfo third_service_info;       // Full Service data for the third departure
    
    // Display positions - self explanatory
    int first_line_y;
    int second_line_y;
    int third_line_y;
    int fourth_line_y;
    int matrix_width;
    int calling_points_width;
    int calling_at_width;
    int message_width;
    int coach_etd_width;
    int coach_etd_position;
    int location_x_position;
    
    // Scroll positions - self explanatory
    int scroll_x_calling_points;
    int scroll_x_message;
    
    // State - for toggle on the 1st, 3rd and 4th row and API refresh interval
    size_t first_service_index;                                      // First row - Index of the departure
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
    void refreshData();                    // get JSON departure data from the API
    void updateDisplayContent();           // Create the content to be displayed
    void renderFrame();                    // Render the data into the matrix display

    // Scrolling functions
    void renderScrollingText(const std::string& text, int scroll_x, int text_width, int y_position);
    void updateScrollPositions();
    void updateMessageScroll();

    // Toggles for display data
    void checkFirstRowStateTransition();   // ETD-Coaches
    void checkThirdRowStateTransition();   // 2nd-3rd departure
    void checkFourthRowStateTransition();  // Message-Location/blank
    void transitionFirstRowState();
    void transitionThirdRowState();
    void transitionFourthRowState();

    // Clock display
    void updateClockDisplay();             // Update the clock

    void calculateTextWidths();
    int calculateTextWidth(const std::string& text);

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


