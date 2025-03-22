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
    TrainServiceParser& parser;   // JSON for C++ parser
    TrainAPIClient& apiClient;    // API client for data refresh
    std::atomic<bool> running;    // Flag to indicate whether the display is running
    const Config& config;         // Traindisplay configuration object
    
    // Display state
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

    bool show_platforms;               // Yes/No - show platforms
    bool platform_selected;            // Yes/No - has a specific platcorm been selected
    std::string selected_platform;     // The selected platform
    bool has_message;                  // Yes/No - are there messages
    bool show_messages;                // Yes/No - are messages being shown
    bool message_scroll_complete;      // Yes/No - has the message been shown

    size_t num_services;               // The number of services available
    
    // Display positions - self explanatory
    int first_line_y;
    int second_line_y;
    int third_line_y;
    int fourth_line_y;
    int matrix_width;
    int calling_points_width;
    int calling_at_width;
    int message_width;
    
    // Scroll positions - self explanatory
    int scroll_x_calling_points;
    int scroll_x_message;
    
    // State - for toggle on the 3rd and 4th row
    ThirdRowState third_row_state;
    FourthRowState fourth_row_state;
    std::chrono::steady_clock::time_point last_third_row_toggle;
    std::chrono::steady_clock::time_point last_fourth_row_toggle;
    std::chrono::steady_clock::time_point last_refresh;

    // Helper methods
    void refreshData();
    void updateDisplayContent();
    void renderFrame();

    void renderScrollingText(const std::string& text, int scroll_x, int text_width, int y_position);
    void updateScrollPositions();
    void updateMessageScroll();

    void checkThirdRowStateTransition();
    void checkFourthRowStateTransition();
    void transitionThirdRowState();
    void transitionFourthRowState();

    void updateClockDisplay();

    void calculateTextWidths();
    int calculateTextWidth(const std::string& text);

public:
    TrainServiceDisplay(RGBMatrix* m, TrainServiceParser& p, TrainAPIClient& ac, const Config& cfg);
    void run();
    void stop();
};

#endif // TRAIN_SERVICE_DISPLAY_H
