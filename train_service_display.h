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
    enum ThirdRowState { SECOND_TRAIN, THIRD_TRAIN, MESSAGE };
    
    // Display content
    std::string top_line;
    std::string calling_points;
    std::string second_line;
    std::string third_line;
    std::string nrcc_message;
    bool has_message;
    bool show_messages;
    bool message_scroll_complete;
    size_t num_services;
    
    // Display positions
    int first_line_y;
    int second_line_y;
    int third_line_y;
    int matrix_width;
    int calling_points_width;
    int message_width;
    
    // Scroll positions
    int scroll_x_calling_points;
    int scroll_x_message;
    
    // State
    ThirdRowState third_row_state;
    std::chrono::steady_clock::time_point last_toggle;
    std::chrono::steady_clock::time_point last_refresh;

    // Helper methods
    void updateDisplayContent() {
        try {
            // Get the number of services
            num_services = parser.getNumberOfServices();
            DEBUG_PRINT("Number of services available: " << num_services);
            
            if (num_services == 0) {
                top_line = "No services available";
                calling_points = "";
                second_line = "";
                third_line = "";
                return;
            }
            
            // Get all the text content
            top_line = parser.getScheduledDepartureTime(0) + " " + 
                      parser.getDestinationLocation(0) + " " + 
                      parser.getEstimatedDepartureTime(0);
            
            calling_points = parser.getLocationNameList(0);

            // Get delay reason if the first train is delayed
            std::string delay_reason = parser.getDelayReason(0);
            if (!delay_reason.empty()) {
                calling_points += " - DELAY REASON: " + delay_reason;
            }

            second_line = num_services > 1 ? 
                "2nd " + parser.getScheduledDepartureTime(1) + " " + 
                parser.getDestinationLocation(1) + " " + 
                parser.getEstimatedDepartureTime(1) : "No more services";
            
            third_line = num_services > 2 ? 
                "3rd " + parser.getScheduledDepartureTime(2) + " " + 
                parser.getDestinationLocation(2) + " " + 
                parser.getEstimatedDepartureTime(2) : "No more services";

            // Check if messages should be shown according to config
            show_messages = config.getBool("ShowMessages");
            
            // Get any NRCC messages
            nrcc_message = "";
            has_message = false;
            
            if (show_messages) {
                nrcc_message = parser.getNrccMessages();
                has_message = !nrcc_message.empty();
                DEBUG_PRINT("NRCC Message: " << (has_message ? nrcc_message : "None"));
            }
            
            // Calculate text widths for scrolling
            calculateTextWidths();
        } catch (const std::exception& e) {
            DEBUG_PRINT("Error updating display content: " << e.what());
            // Set fallback content in case of error
            top_line = "Error fetching data";
            calling_points = e.what();
            second_line = "Please try again later";
            third_line = "";
        }
    }

    void calculateTextWidths() {
        calling_points_width = 0;
        for (const char& c : calling_points) {
            calling_points_width += font.CharacterWidth(c);
        }
        
        message_width = 0;
        if (has_message) {
            for (const char& c : nrcc_message) {
                message_width += font.CharacterWidth(c);
            }
        }
    }
    
    void renderFrame() {
        canvas->Clear();

        // Draw static top line
        rgb_matrix::DrawText(canvas, font, 0, first_line_y, white, top_line.c_str());

        // Draw scrolling calling points with wrap-around
        renderScrollingText(calling_points, scroll_x_calling_points, calling_points_width, second_line_y);

        // Draw current third line content based on state
        if (third_row_state == MESSAGE && has_message) {
            renderScrollingText(nrcc_message, scroll_x_message, message_width, third_line_y);
            updateMessageScroll();
        } else {
            // Draw static train info (2nd or 3rd)
            std::string current_line = (third_row_state == SECOND_TRAIN) ? second_line : third_line;
            rgb_matrix::DrawText(canvas, font, 0, third_line_y, white, current_line.c_str());
        }

        // Update display
        canvas = matrix->SwapOnVSync(canvas);
    }

    void renderScrollingText(const std::string& text, int scroll_x, int text_width, int y_position) {
        rgb_matrix::DrawText(canvas, font, scroll_x, y_position, white, text.c_str());
        if (scroll_x < 0) {
            rgb_matrix::DrawText(canvas, font, scroll_x + matrix_width + text_width, 
                               y_position, white, text.c_str());
        }
    }

    void updateScrollPositions() {
        // Update calling points scroll position with wrap-around
        scroll_x_calling_points--;
        if (scroll_x_calling_points < -calling_points_width) {
            scroll_x_calling_points = matrix_width;
        }
    }

    void updateMessageScroll() {
        // Update message scroll position
        scroll_x_message--;
        
        // Check if a complete message scroll cycle has occurred
        if (scroll_x_message <= -message_width) {
            // Reset scroll position for a smooth continuous scroll
            scroll_x_message = matrix_width;
            // Mark that we've completed at least one full scroll
            message_scroll_complete = true;
        }
    }

    void checkThirdRowStateTransition() {
        auto now = std::chrono::steady_clock::now();
        bool should_toggle = false;
        
        if (third_row_state == MESSAGE) {
            // If we're showing a message, only toggle when scrolling is complete
            if (message_scroll_complete && 
                now - last_toggle >= std::chrono::seconds(config.getInt("third_line_refresh_seconds"))) {
                should_toggle = true;
            }
        } else {
            // For train information, toggle based on timer
            if (now - last_toggle >= std::chrono::seconds(config.getInt("third_line_refresh_seconds"))) {
                should_toggle = true;
            }
        }
        
        if (should_toggle) {
            transitionThirdRowState();
            last_toggle = now;
        }
    }

    void transitionThirdRowState() {
        if (has_message) {
            // If message exists, cycle through 3 states
            if (third_row_state == SECOND_TRAIN) {
                third_row_state = THIRD_TRAIN;
            } else if (third_row_state == THIRD_TRAIN) {
                third_row_state = MESSAGE;
                // Reset message scroll position and completion flag
                scroll_x_message = matrix_width;
                message_scroll_complete = false;
            } else { // MESSAGE
                third_row_state = SECOND_TRAIN;
            }
        } else {
            // If no message, toggle between 2nd and 3rd train only
            third_row_state = (third_row_state == SECOND_TRAIN) ? THIRD_TRAIN : SECOND_TRAIN;
        }
    }
    
    void refreshData() {
        try {
            std::string api_data = apiClient.fetchDepartures(
                config.get("from"), config.get("to"));
            parser.updateData(api_data);
            updateDisplayContent();
        } catch (const std::exception& e) {
            std::cerr << "Error refreshing data: " << e.what() << std::endl;
        }
    }

public:
    TrainServiceDisplay(RGBMatrix* m, TrainServiceParser& p, TrainAPIClient& ac, const Config& cfg) 
        : matrix(m), canvas(m->CreateFrameCanvas()),
          white(255, 255, 255), black(0, 0, 0), 
          parser(p), apiClient(ac), running(true), config(cfg),
          first_line_y(cfg.getInt("first_line_y")),
          second_line_y(cfg.getInt("second_line_y")),
          third_line_y(cfg.getInt("third_line_y")),
          matrix_width(m->width()),
          calling_points_width(0), message_width(0),
          scroll_x_calling_points(matrix_width), scroll_x_message(matrix_width),
          third_row_state(SECOND_TRAIN), message_scroll_complete(false) {
    
        if (!font.LoadFont(config.get("fontPath").c_str())) {
            throw std::runtime_error("Font loading failed for: " + config.get("fontPath"));
        }
        
        // Initialize timestamps
        last_toggle = std::chrono::steady_clock::now();
        last_refresh = std::chrono::steady_clock::now();
        
        DEBUG_PRINT("Display initialized with font: " << config.get("fontPath"));
        
        // Set parser options from config
        parser.setShowCallingPointETD(config.getBool("ShowCallingPointETD"));
        
        // Initial data load
        updateDisplayContent();
    }

    void run() {
        while (running) {
            try {
                // Refresh data if needed
                auto now = std::chrono::steady_clock::now();
                if (now - last_refresh >= std::chrono::seconds(
                    config.getInt("refresh_interval_seconds"))) {
                    refreshData();
                    last_refresh = std::chrono::steady_clock::now();
                }
                
                // Check for state transitions
                checkThirdRowStateTransition();
                
                // Render the current frame
                renderFrame();
                
                // Update scroll positions
                updateScrollPositions();
                
                // Sleep for the configured time
                std::this_thread::sleep_for(std::chrono::milliseconds(
                    config.getInt("scroll_slowdown_sleep_ms")));
                
            } catch (const std::exception& e) {
                std::cerr << "Display error: " << e.what() << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(
                    config.getInt("refresh_interval_seconds")));
            }
        }
    }

    void stop() {
        running = false;
    }
};

#endif // TRAIN_SERVICE_DISPLAY_H

