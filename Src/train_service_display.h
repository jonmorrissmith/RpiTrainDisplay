// Train Display - an RGB matrix departure board for the Raspberry Pi
// Jon Morris Smith - Feb 2025
// Version 1.0
// Instructions, fixes and issues at https://github.com/jonmorrissmith/RGB_Matrix_Train_Departure_Board
//
// With thanks to:
// https://github.com/jpsingleton/Huxley2
// https://github.com/hzeller/rpi-rgb-led-matrix
// https://github.com/nlohmann/json
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
    RGBMatrix* matrix;
    FrameCanvas* canvas;
    Font font;
    
    // Colors
    Color white;
    Color black;
    
    // Data handlers
    TrainServiceParser& parser;
    TrainAPIClient& apiClient;
    std::atomic<bool> running;
    const Config& config;
    
    // Display state
    enum ThirdRowState { SECOND_TRAIN, THIRD_TRAIN };
    enum FourthRowState { CLOCK, MESSAGE };
    
    // Display content
    std::string top_line;
    std::string calling_points;
    std::string second_line;
    std::string third_line;
    std::string clock_display;
    std::string nrcc_message;
    bool show_platforms;
    bool platform_selected;
    std::string selected_platform;
    bool has_message;
    bool show_messages;
    bool message_scroll_complete;
    size_t num_services;
    
    // Display positions
    int first_line_y;
    int second_line_y;
    int third_line_y;
    int fourth_line_y;
    int matrix_width;
    int calling_points_width;
    int calling_at_width;
    int message_width;
    
    // Scroll positions
    int scroll_x_calling_points;
    int scroll_x_message;
    
    // State
    ThirdRowState third_row_state;
    FourthRowState fourth_row_state;
    std::chrono::steady_clock::time_point last_third_row_toggle;
    std::chrono::steady_clock::time_point last_fourth_row_toggle;
    std::chrono::steady_clock::time_point last_refresh;

    // Helper methods
    void updateDisplayContent();
    void updateClockDisplay();
    void calculateTextWidths();
    int calculateTextWidth(const std::string& text);
    void renderFrame();
    void renderScrollingText(const std::string& text, int scroll_x, int text_width, int y_position);
    void updateScrollPositions();
    void updateMessageScroll();
    void checkThirdRowStateTransition();
    void checkFourthRowStateTransition();
    void transitionThirdRowState();
    void transitionFourthRowState();
    void refreshData();

public:
    TrainServiceDisplay(RGBMatrix* m, TrainServiceParser& p, TrainAPIClient& ac, const Config& cfg);
    void run();
    void stop();
};

#endif // TRAIN_SERVICE_DISPLAY_H

