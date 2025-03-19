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

#include "train_service_display.h"

TrainServiceDisplay::TrainServiceDisplay(RGBMatrix* m, TrainServiceParser& p, TrainAPIClient& ac, const Config& cfg) 
    : matrix(m), canvas(m->CreateFrameCanvas()),
      white(255, 255, 255), black(0, 0, 0), 
      parser(p), apiClient(ac), running(true), config(cfg),
      first_line_y(cfg.getInt("first_line_y")),
      second_line_y(cfg.getInt("second_line_y")),
      third_line_y(cfg.getInt("third_line_y")),
      fourth_line_y(cfg.getInt("fourth_line_y")),
      matrix_width(m->width()),
      calling_points_width(0), message_width(0),
      scroll_x_calling_points(matrix_width), scroll_x_message(matrix_width),
      third_row_state(SECOND_TRAIN), fourth_row_state(CLOCK), message_scroll_complete(false) {

    if (!font.LoadFont(config.get("fontPath").c_str())) {
        throw std::runtime_error("Font loading failed for: " + config.get("fontPath"));
    }
    
    // Initialize timestamps
    last_third_row_toggle = std::chrono::steady_clock::now();
    last_fourth_row_toggle = std::chrono::steady_clock::now();
    last_refresh = std::chrono::steady_clock::now();
    
    DEBUG_PRINT("Display initialized with font: " << config.get("fontPath"));
    
    // Set parser options from config
    parser.setShowCallingPointETD(config.getBool("ShowCallingPointETD"));
    
    // Initial data load
    updateDisplayContent();
    
    // Initial clock value
    updateClockDisplay();
}

void TrainServiceDisplay::updateDisplayContent() {
    int departure;

    try {
        // Get the number of services
        num_services = parser.getNumberOfServices();
        DEBUG_PRINT("Number of services available: " << num_services);
        
        if (num_services == 0) {
            top_line = "No services";
            calling_points = "";
            second_line = "";
            third_line = "";
            return;
        }
        
        // Are we showing platforms? 
        show_platforms = config.getBool("ShowPlatforms");

        // Has a specific platform been selected?
        selected_platform = config.get("platform");
        platform_selected = !selected_platform.empty();
        DEBUG_PRINT("Selected platform: " << selected_platform);
        if (platform_selected) {
                   parser.findServicesAtPlatform(selected_platform);
        }

        if (platform_selected) {
            // Platform selected
            // display the next departure along with the 2nd and 3rd
            // for the selected platform

            // First departure from selected platform
            departure = parser.returnServiceAtPlatform(1); 
            if (departure >= 0) { 
               DEBUG_PRINT("Top line. Index: " << departure << " "
                        << parser.getScheduledDepartureTime(departure) << " " 
                        << parser.getDestinationLocation(departure) << " " 
                        << parser.getEstimatedDepartureTime(departure) << " "
                        << parser.getOperator(departure)  << " "
                        << parser.getCoaches(departure, true)  << " "
                        << ". Cancelled flag: " << parser.isCancelled(departure));

                top_line = parser.getScheduledDepartureTime(departure) + " ";
                if (show_platforms) { 
                          top_line = top_line + parser.getPlatform(departure) + " ";
                }
                top_line = top_line + parser.getDestinationLocation(departure) + " "; 
                top_line = top_line + parser.getEstimatedDepartureTime(departure);
        
                if (parser.isCancelled(departure)) {
                    calling_points = parser.getCancelReason(departure);
                } else {
                    calling_points = parser.getLocationNameList(departure) + parser.getOperator(departure) + parser.getCoaches(departure, true);
                }

                // Get delay reason if the first train is delayed
                std::string delay_reason = parser.getDelayReason(departure);
                if (!delay_reason.empty()) {
                    calling_points += " - " + delay_reason;
                }
             }

             // Second departure from selected platform
            departure = parser.returnServiceAtPlatform(2); 
            if (departure >= 0 ) {

               DEBUG_PRINT("Second line. Index: " << departure << " "
                        << parser.getScheduledDepartureTime(departure) << " "
                        << parser.getDestinationLocation(departure) << " "
                        << parser.getEstimatedDepartureTime(departure) << " "
                        << ". Cancelled flag: " << parser.isCancelled(departure));

                second_line ="2nd " + parser.getScheduledDepartureTime(departure) + " ";
                if (show_platforms) {
                      second_line = second_line + parser.getPlatform(departure) + " ";
                }
                second_line = second_line + parser.getDestinationLocation(departure) + " ";
                second_line = second_line + parser.getEstimatedDepartureTime(departure);
            } 
            else {
                second_line = "No more services";
            }
       

            // Third departure from selected platformA
            departure = parser.returnServiceAtPlatform(3);
            if (departure >= 0 ) {
               DEBUG_PRINT("Third line. Index: " << departure << " "
                        << parser.getScheduledDepartureTime(departure) << " "
                        << parser.getDestinationLocation(departure) << " "
                        << parser.getEstimatedDepartureTime(departure) << " "
                        << ". Cancelled flag: " << parser.isCancelled(departure)); 

                third_line ="3rd " + parser.getScheduledDepartureTime(departure) + " ";
                if (show_platforms) {
                      third_line = third_line + parser.getPlatform(departure) + " ";
                }
                third_line = third_line + parser.getDestinationLocation(departure) + " ";
                third_line = third_line + parser.getEstimatedDepartureTime(departure);
            }
            else {
                third_line = "No more services";
            } 
        } else {
            // Platform not selected
            // Display the next departure along with the 2nd and 3rd for all platforms
            DEBUG_PRINT("Top line. Index: 0 " << " "
                     << parser.getScheduledDepartureTime(0) << " "
                     << parser.getDestinationLocation(0) << " "
                     << parser.getEstimatedDepartureTime(0) << " "
                     << parser.getOperator(0) << " "
                     << parser.getCoaches(0, true) << " "
                     << ". Cancelled flag: " << parser.isCancelled(0));

            top_line = parser.getScheduledDepartureTime(0) + " ";
            if (show_platforms) {
                      top_line = top_line + parser.getPlatform(0) + " ";
            }
            top_line = top_line + parser.getDestinationLocation(0) + " ";
            top_line = top_line + parser.getEstimatedDepartureTime(0);

            if (parser.isCancelled(0)) {
                calling_points = parser.getCancelReason(0);
                DEBUG_PRINT("Cancellation reason: " << parser.getCancelReason(0));
            } else {
                calling_points = parser.getLocationNameList(0) + parser.getOperator(0) + parser.getCoaches(0, true);
            }

            // Get delay reason if the first train is delayed
            std::string delay_reason = parser.getDelayReason(0);
            if (!delay_reason.empty()) {
                calling_points += " - " + delay_reason;
            }

             if (num_services > 1 ) {
                DEBUG_PRINT("2nd line. Index: 1 " << " "
                         << parser.getScheduledDepartureTime(1) << " "
                         << parser.getDestinationLocation(1) << " "
                         << parser.getEstimatedDepartureTime(1) << " "
                         << parser.getOperator(1) << " "
                         << parser.getCoaches(1, true) << " "
                         << ". Cancelled flag: " << parser.isCancelled(1));

                second_line ="2nd " + parser.getScheduledDepartureTime(1) + " ";
                if (show_platforms) {
                      second_line = second_line + parser.getPlatform(1) + " ";
                }
                second_line = second_line + parser.getDestinationLocation(1) + " ";
                second_line = second_line + parser.getEstimatedDepartureTime(1);
            }
            else {
                second_line = "No more services";
            }


            if (num_services > 2 ) {
                DEBUG_PRINT("3rd line. Index: 2 " << " "
                         << parser.getScheduledDepartureTime(2) << " "
                         << parser.getDestinationLocation(2) << " "
                         << parser.getEstimatedDepartureTime(2) << " "
                         << parser.getOperator(2) << " "
                         << parser.getCoaches(2, true) << " "
                         << ". Cancelled flag: " << parser.isCancelled(2));

                third_line ="3rd " + parser.getScheduledDepartureTime(2) + " ";
                if (show_platforms) {
                      third_line = third_line + parser.getPlatform(2) + " ";
                }
                third_line = third_line + parser.getDestinationLocation(2) + " ";
                third_line = third_line + parser.getEstimatedDepartureTime(2);
            }
            else {
                third_line = "No more services";
            }
        }
        
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
        third_line = "Please try again later";
    }
}

void TrainServiceDisplay::updateClockDisplay() {
    // Get current time
    auto now = std::time(nullptr);
    auto tm = std::localtime(&now);
    
    // Format time as HH:MM:SS in 24-hour format
    std::ostringstream timeStream;
    timeStream << std::setfill('0') << std::setw(2) << tm->tm_hour << ":"
               << std::setfill('0') << std::setw(2) << tm->tm_min << ":"
               << std::setfill('0') << std::setw(2) << tm->tm_sec;
    
    clock_display = timeStream.str();
}

void TrainServiceDisplay::calculateTextWidths() {
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

    calling_at_width = 0;
    for (const char& c : "Calling at:") {
        calling_at_width += font.CharacterWidth(c);
    }
}

int TrainServiceDisplay::calculateTextWidth(const std::string& text) {
    int width = 0;
    for (const char& c : text) {
        width += font.CharacterWidth(c);
    }
    return width;
}

void TrainServiceDisplay::renderFrame() {
    canvas->Clear();

    // Draw static top line
    rgb_matrix::DrawText(canvas, font, 0, first_line_y, white, top_line.c_str());

    // Draw scrolling calling points with wrap-around
    renderScrollingText(calling_points, scroll_x_calling_points, calling_points_width, second_line_y);

    // Draw current third line content based on state
    std::string current_third_line = (third_row_state == SECOND_TRAIN) ? second_line : third_line;
    rgb_matrix::DrawText(canvas, font, 0, third_line_y, white, current_third_line.c_str());

    // Draw fourth row content (either clock or message)
    if (fourth_row_state == CLOCK || !has_message) {
        // Update clock display
        updateClockDisplay();
        
        // Right-Justify the clock text
        int clock_width = calculateTextWidth(clock_display);
        int x_position = (matrix_width - clock_width);
        
        // Draw the right-justified clock
        rgb_matrix::DrawText(canvas, font, x_position, fourth_line_y, white, clock_display.c_str());
    } else if (fourth_row_state == MESSAGE && has_message) {
        // Draw scrolling message (the clock will be drawn inside renderScrollingText)
        renderScrollingText(nrcc_message, scroll_x_message, message_width, fourth_line_y);
        updateMessageScroll();
    }

    // Update display
    canvas = matrix->SwapOnVSync(canvas);
}

void TrainServiceDisplay::renderScrollingText(const std::string& text, int scroll_x, int text_width, int y_position) {
    // Draw the scrolling text as normal
    rgb_matrix::DrawText(canvas, font, scroll_x, y_position, white, text.c_str());
    if (scroll_x < 0) {
        rgb_matrix::DrawText(canvas, font, scroll_x + matrix_width + text_width, 
                           y_position, white, text.c_str());
    }
    
    // If this is the calling points line, add "Calling at:" at the beginning
    if (y_position == second_line_y) {
        // Clear the area where "Calling at:" will be displayed by drawing a black rectangle
        for (int x = 0; x < calling_at_width; x++) {
            for (int y = y_position - font.baseline(); y < y_position + font.height() - font.baseline(); y++) {
                if (y >= 0 && y < matrix->height() && x >= 0 && x < matrix->width()) {
                    canvas->SetPixel(x, y, black.r, black.g, black.b);
                }
            }
        }
        
        // Draw "Calling at:" text
        rgb_matrix::DrawText(canvas, font, 0, y_position, white, "Calling at:");
    }
    
    // If this is the message line, add clock at the right side
    if (y_position == fourth_line_y && fourth_row_state == MESSAGE) {
        // Update clock display
        updateClockDisplay();
        
        // Calculate clock width and position
        int clock_width = calculateTextWidth(clock_display) ;
        int x_position = matrix_width - clock_width;
        
        // Clear the area where the clock will be displayed
        for (int x = x_position; x < matrix_width; x++) {
            for (int y = y_position - font.baseline(); y < y_position + font.height() - font.baseline(); y++) {
                if (y >= 0 && y < matrix->height() && x >= 0 && x < matrix->width()) {
                    canvas->SetPixel(x, y, black.r, black.g, black.b);
                }
            }
        }
        
        // Draw the right-justified clock
        rgb_matrix::DrawText(canvas, font, x_position, y_position, white, clock_display.c_str());
    }
}


void TrainServiceDisplay::updateScrollPositions() {
    // Update calling points scroll position with wrap-around
    scroll_x_calling_points--;
    if (scroll_x_calling_points < -calling_points_width) {
        scroll_x_calling_points = matrix_width;
    }
}

void TrainServiceDisplay::updateMessageScroll() {
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

void TrainServiceDisplay::checkThirdRowStateTransition() {
    auto now = std::chrono::steady_clock::now();
    
    // For train information, toggle based on timer
    if (now - last_third_row_toggle >= std::chrono::seconds(config.getInt("third_line_refresh_seconds"))) {
        transitionThirdRowState();
        last_third_row_toggle = now;
    }
}

void TrainServiceDisplay::checkFourthRowStateTransition() {
    auto now = std::chrono::steady_clock::now();
    bool should_toggle = false;
    
    if (!show_messages || !has_message) {
        // If messages are disabled or there aren't any, always show the clock
        fourth_row_state = CLOCK;
        return;
    }
    
    if (fourth_row_state == MESSAGE) {
        // If we're showing a message, only toggle when scrolling is complete
        if (message_scroll_complete && 
            now - last_fourth_row_toggle >= std::chrono::seconds(config.getInt("Message_Refresh_interval"))) {
            should_toggle = true;
        }
    } else {
        // For clock, toggle based on timer
        if (now - last_fourth_row_toggle >= std::chrono::seconds(config.getInt("Message_Refresh_interval"))) {
            should_toggle = true;
        }
    }
    
    if (should_toggle) {
        transitionFourthRowState();
        last_fourth_row_toggle = now;
    }
}

void TrainServiceDisplay::transitionThirdRowState() {
    // Simply toggle between 2nd and 3rd train only
    third_row_state = (third_row_state == SECOND_TRAIN) ? THIRD_TRAIN : SECOND_TRAIN;
}

void TrainServiceDisplay::transitionFourthRowState() {
    if (has_message) {
        // Toggle between clock and message
        if (fourth_row_state == CLOCK) {
            fourth_row_state = MESSAGE;
            // Reset message scroll position and completion flag
            scroll_x_message = matrix_width;
            message_scroll_complete = false;
        } else { // MESSAGE
            fourth_row_state = CLOCK;
        }
    } else {
        // If no message, always show clock
        fourth_row_state = CLOCK;
    }
}

void TrainServiceDisplay::refreshData() {
    try {
        std::string api_data = apiClient.fetchDepartures(
            config.get("from"), config.get("to"));
        parser.updateData(api_data);
        updateDisplayContent();
    } catch (const std::exception& e) {
        std::cerr << "Error refreshing data: " << e.what() << std::endl;
    }
}

void TrainServiceDisplay::run() {
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
            checkFourthRowStateTransition();
            
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

void TrainServiceDisplay::stop() {
    running = false;
}

