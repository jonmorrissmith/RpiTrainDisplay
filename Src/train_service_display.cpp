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

#include "train_service_display.h"

TrainServiceDisplay::TrainServiceDisplay(RGBMatrix* m, TrainServiceParser& p, TrainAPIClient& ac, const Config& cfg)
: matrix(m),
parser(p),
apiClient(ac),
config(cfg),

// Set flags from configuration
show_platforms(cfg.getBool("ShowPlatforms")),
show_location(cfg.getBool("ShowLocation")),
show_messages(cfg.getBool("ShowMessages")),

// Set timing from configuration
ETD_coach_refresh_seconds(cfg.getInt("ETD_coach_refresh_seconds")),
third_line_refresh_seconds(cfg.getInt("third_line_refresh_seconds")),
Message_Refresh_interval(cfg.getInt("Message_Refresh_interval")),
refresh_interval_seconds(cfg.getInt("refresh_interval_seconds")),

white(255, 255, 255), black(0, 0, 0)
{
    // We're up and running!
    running = true;
    
    // Load and cache the font
    if (!font.LoadFont(config.get("fontPath").c_str())) {
        throw std::runtime_error("Font loading failed for: " + config.get("fontPath"));
    }
    font_cache.setFont(font);
    font_baseline = font_cache.getBaseline();
    font_height = font_cache.getheight();
    
    // Cache matrix parameters
    matrix_width = m->width();
    matrix_height = m->height();
    canvas = m->CreateFrameCanvas();
    
    // Initialise scrolling positions
    calling_points_text.x_position = matrix_width;
    nrcc_message_text.x_position = matrix_width;
    
    // Store the amount of space available to display the calling points;
    // Width of 'Calling at:' never changes
    calling_at_text.setTextAndWidth("Calling at:", font_cache);
    space_for_calling_points = matrix_width - calling_at_text.width;
    scroll_calling_points = true;
    
    // Initialise toggle states
    first_row_state = ETD;
    third_row_state = SECOND_TRAIN;
    fourth_row_state = LOCATION;
    message_scroll_complete = false;
    data_refresh_pending = false;
    data_refresh_completed = false;
    
    // Initialize toggle timestamps
    last_first_row_toggle = std::chrono::steady_clock::now();
    last_third_row_toggle = std::chrono::steady_clock::now();
    last_fourth_row_toggle = std::chrono::steady_clock::now();
    last_refresh = std::chrono::steady_clock::now();
    
    // Initialize text y positions
    first_departure.y_position = config.getInt("first_line_y");
    first_departure_coaches.y_position = config.getInt("first_line_y");
    first_departure_etd.y_position = config.getInt("first_line_y");
    calling_points_text.y_position = config.getInt("second_line_y");
    calling_at_text.y_position = config.getInt("second_line_y");
    second_departure.y_position = config.getInt("third_line_y");
    second_departure_etd.y_position = config.getInt("third_line_y");
    third_departure.y_position= config.getInt("third_line_y");
    third_departure_etd.y_position= config.getInt("third_line_y");
    clock_display_text.y_position = config.getInt("fourth_line_y");
    nrcc_message_text.y_position = config.getInt("fourth_line_y");
    location_name_text.y_position = config.getInt("fourth_line_y");
    
    // Initialize text x positions
    calling_points_text.x_position = matrix_width;
    nrcc_message_text.x_position = matrix_width;
    
    // Set parser options from config
    // Are calling points ETD's being shown?
    parser.setShowCallingPointETD(config.getBool("ShowCallingPointETD"));
    
    // Has a platform been selected?
    if(!config.get("platform").empty()){
        parser.setSelectedPlatform(config.get("platform"));
    }
    
    DEBUG_PRINT("Display initialisation. font: " << config.get("fontPath") << std::endl <<
                "Selected platform (bool/platform): " << selected_platform << "/" << platform_selected << std::endl <<
                "Showing Location: " << show_location << std::endl <<
                "Showing messages: " << show_messages << std::endl <<
                "Showing platforms: " << show_platforms);
    DEBUG_PRINT("Configuration: " << std::endl <<
                "Matrix width: " << matrix_width << std::endl <<
                "first_departure y: " << first_departure.y_position << std::endl <<
                "calling_points_text y: " << calling_points_text.y_position << std::endl <<
                "calling_points_text x: " << calling_points_text.x_position << std::endl <<
                "calling_at_text y: " << calling_at_text.y_position << std::endl <<
                "calling_at_text x: " << calling_at_text.x_position << std::endl <<
                "second_departure y: " << second_departure.y_position << std::endl <<
                "second_departure etd y: " << second_departure_etd.y_position << std::endl <<
                "third_departure y: " << third_departure.y_position << std::endl <<
                "third_departure etd y: " << third_departure_etd.y_position << std::endl <<
                "nrcc_message_text y: " << nrcc_message_text.y_position << std::endl <<
                "nrcc_message_text x: " << nrcc_message_text.x_position << std::endl <<
                "ETD_coach_refresh_seconds: " << ETD_coach_refresh_seconds << std::endl <<
                "third_line_refresh_seconds: " << third_line_refresh_seconds << std::endl <<
                "Message_Refresh_interval: " << Message_Refresh_interval << std::endl <<
                "refresh_interval_seconds (data): " << refresh_interval_seconds << ".");
                
    // Initial data load and version set
    display_data_version = 1;
    api_data_version = 1;
    updateDisplayContent();
    
    // Initial clock value
    updateClockDisplay();
}

void TrainServiceDisplay::updateDisplayContent() {

    try {
        // Get the number of services
        num_services = parser.getNumberOfServices();
        DEBUG_PRINT("Updating the Display Content");
        DEBUG_PRINT("Number of services available: " << num_services);
        
        if (num_services == 0) {
            first_departure = "No services";
            calling_points_text = "";
            second_departure = "";
            third_departure = "";
            return;
        }
        
        DEBUG_PRINT("Showing platforms: " << show_platforms);
        DEBUG_PRINT("Selected platform: " << parser.getSelectedPlatform());
        DEBUG_PRINT("Showing location: " << show_location);
        DEBUG_PRINT("Starting display refresh. API version: " << getCurrentAPIVersion() << ". Display version: " << getCurrentDisplayVersion() << ". Cache version: " << parser.getCurrentVersion());

        // Find Services
        parser.findServices();
        
        // Get location name
        location_name_text = "";
 
        // If we're showing the location then get the width and calculate the x position to centre on the display
        if (show_location) {
            location_name_text.setTextAndWidth(parser.getLocationName(), font_cache);
            location_name_text.x_position = (matrix_width - location_name_text.width)/2;
        }
        
        // Create the Top Line - get the index of the first service to depart.
        first_service_index = parser.getFirstDeparture();
        
        if(first_service_index == 999) {
            first_departure= "No more services";
            calling_points_text = "";
            calling_at_text = "";
        } else {
            // Populate the ServiceInfo struct for the first service.
            first_service_info = parser.getService(first_service_index);
            
            // Populate the first departure on the top line
            first_departure = "";
            first_departure << first_service_info.scheduledTime << " ";
            
            // Instert"'Plat." and the platform number if we're showing platforms.
            if (show_platforms && !first_service_info.platform.empty()) {
                first_departure << "Plat." << first_service_info.platform << " ";
            }
            
            first_departure << first_service_info.destination << " ";
            
            // Populate Coaches and ETD
            first_departure_etd.setTextAndWidth(first_service_info.estimatedTime, font_cache);
            
            // If there's no coach information we'll just display the ETD.
            if (first_service_info.coaches.empty()) {
                first_departure_coaches = first_departure_etd;
            } else {
                first_departure_coaches.setTextAndWidth(first_service_info.coaches + " coaches", font_cache);
            }
    
            // Set x_position to right-justify the ETD and Coaches text
            first_departure_etd.x_position = matrix_width - first_departure_etd.width;
            first_departure_coaches.x_position = matrix_width - first_departure_coaches.width;
            
            // Create calling points
            calling_at_text = "Calling at:";
            calling_points_text = "";
            if(first_service_info.isCancelled) {
                calling_points_text << first_service_info.cancelReason;
            } else {
                // Because we lazy-load the calling-points, we have to use the getCallingPoints method. The calling points are stored in the parser class after this has run.
                calling_points_text << parser.getCallingPoints(first_service_index) << " " << first_service_info.operator_name << parser.getCoaches(first_service_index, true);
            }
            
            if(first_service_info.isDelayed){
                if(!first_service_info.delayReason.empty()){
                    calling_points_text << " - " << first_service_info.delayReason;
                }
            }
            //DEBUG_PRINT("-----------------------");
            //if (debug_mode)  parser.debugPrintServiceStruct(first_service_index);
            DEBUG_PRINT("Display Content update:" << std::endl <<
                        "First Departure:" << first_departure.text << std::endl <<
                        "Calling Points: " << calling_points_text.text << " (width of the scroll: " << calling_points_text.width << ")" );
        }

        // Create the Second Line
        second_service_index = parser.getSecondDeparture();
        if(second_service_index == 999) {
            second_departure = "No more services";
            second_departure_etd = "";
        } else {
            second_service_info = parser.getService(second_service_index);

            second_departure = "2nd ";
            second_departure << second_service_info.scheduledTime + " ";
            if(show_platforms && !second_service_info.platform.empty()) {
                second_departure << "Plat." << second_service_info.platform << " ";
            }
            second_departure << second_service_info.destination << " ";
            
            // Populate Coaches and ETD
            second_departure_etd.setTextAndWidth(second_service_info.estimatedTime, font_cache);
    
            // Set x_position to right-justify the text
            second_departure_etd.x_position = matrix_width - second_departure_etd.width;
            
            //DEBUG_PRINT("-----------------------");
            //if(debug_mode) parser.debugPrintServiceStruct(second_service_index);
            DEBUG_PRINT("2nd Departure: " << second_departure.text << std::endl <<
                        "2nd Departure ETD: " << second_departure_etd.text);
            
        }

        // Create the Third Line
        third_service_index = parser.getThirdDeparture();
        if(third_service_index == 999) {
            third_departure = "No more services";
            third_departure_etd = "";
        } else {
            third_service_info = parser.getService(third_service_index);
            
            third_departure = "3rd ";
            third_departure << third_service_info.scheduledTime << " ";
            
            if(show_platforms && !third_service_info.platform.empty()) {
                third_departure << "Plat." <<  third_service_info.platform << " ";
            }
            
            third_departure << third_service_info.destination << " ";
            
            // Populate Coaches and ETD
            third_departure_etd.setTextAndWidth(third_service_info.estimatedTime, font_cache);
            
            // Set x_position to right-justify the text
            third_departure_etd.x_position = matrix_width - third_departure_etd.width;
            
            //DEBUG_PRINT("-----------------------");
            //if(debug_mode) parser.debugPrintServiceStruct(third_service_index);
            DEBUG_PRINT("3rd Departure: " << third_departure.text<< std::endl <<
                        "3rd Departure ETD: " << third_departure_etd.text);
        }
        
        // Get any NRCC messages
        has_message = false;
        nrcc_message_text = "";
        
        if (show_messages) {
            nrcc_message_text << parser.getNrccMessages();
            has_message = !nrcc_message_text.empty();
            DEBUG_PRINT("NRCC Message: " << (has_message ? nrcc_message_text.text : "None"));
            // End New
        }
        
        // Calculate text widths for scrolling
        calling_points_text.setWidth(font_cache);
        // If the width of the calling points is less than the width of the space for the calling points, then don't scroll.
        scroll_calling_points = (calling_points_text.width < space_for_calling_points ? false : true);
        nrcc_message_text.setWidth(font_cache);
        
    } catch (const std::exception& e) {
        DEBUG_PRINT("Error updating display content: " << e.what());
        // Set fallback content in case of error
        first_departure = "Error fetching data";
        calling_points_text = e.what();
        second_departure = "Error fetching data";
        third_departure = "Error fetching data";
        // End new
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
    
    clock_display_text.setTextAndWidth(timeStream.str(), font_cache);
    clock_display_text.x_position = matrix_width - clock_display_text.width;
}

void TrainServiceDisplay::renderFrame() {
    canvas->Clear();
    int x, y;
    
    // Draw static top line
    rgb_matrix::DrawText(canvas, font, 0, first_departure.y_position, white, first_departure.text.c_str());
    
    // Draw right-justified ETD or Coach configuration for the first service if the first service exists
    if(first_service_index != 999) {
        
        if (first_row_state == ETD) {
            rgb_matrix::DrawText(canvas, font, first_departure_etd.x_position, first_departure_etd.y_position, white, first_departure_etd.text.c_str());
        } else {
            rgb_matrix::DrawText(canvas, font, first_departure_coaches.x_position, first_departure_coaches.y_position, white, first_departure_coaches.text.c_str());
        }
    }
    
    // Draw scrolling calling points with wrap-around
    renderScrollingCallingPoints();
    
    // Draw current third line content based on state
    if (third_row_state == SECOND_TRAIN) {
        rgb_matrix::DrawText(canvas, font, 0, second_departure.y_position, white, second_departure.text.c_str());
        rgb_matrix::DrawText(canvas, font, second_departure_etd.x_position, second_departure_etd.y_position, white, second_departure_etd.text.c_str());
    } else {
        rgb_matrix::DrawText(canvas, font, 0, third_departure.y_position, white, third_departure.text.c_str());
        rgb_matrix::DrawText(canvas, font, third_departure_etd.x_position, third_departure_etd.y_position, white, third_departure_etd.text.c_str());
    }
    
    // Draw fourth line
    
    if (fourth_row_state == LOCATION) { // Show the location
        rgb_matrix::DrawText(canvas, font, location_name_text.x_position, location_name_text.y_position, white, location_name_text.text.c_str());
    } else { // Scroll the message
        renderScrollingMessage();
    }
    
    // Update and draw the clock
    updateClockDisplay();
    
    // Clear the area where the clock will be displayed.  Note the '-2' is applied to the x position to give a small gap between the clock and the scrolling message
    for (x = clock_display_text.x_position - 2; x < matrix_width; x++) {
        for ( y = clock_display_text.y_position - font_baseline; y < clock_display_text.y_position + font_height - font_baseline; y++) {
            if (y >= 0 && y < matrix_height && x >= 0 && x < matrix_width) {
                canvas->SetPixel(x, y, black.r, black.g, black.b);
            }
        }
    }
    // Draw the clock
    rgb_matrix::DrawText(canvas, font, clock_display_text.x_position, clock_display_text.y_position, white, clock_display_text.text.c_str());

    // Update display
    canvas = matrix->SwapOnVSync(canvas);
}

void TrainServiceDisplay::renderScrollingCallingPoints() {
    rgb_matrix::DrawText(canvas, font, calling_points_text.x_position, calling_points_text.y_position, white, calling_points_text.text.c_str());
    if (calling_points_text.x_position < 0) {
        rgb_matrix::DrawText(canvas, font, calling_points_text.x_position + matrix_width + calling_points_text.width, calling_points_text.y_position, white, calling_points_text.text.c_str());
    }
    
    // Clear the area where "Calling at:" will be displayed by drawing a black rectangle
    for (int x = 0; x < calling_at_text.width; x++) {
        for (int y = calling_at_text.y_position - font_baseline; y < calling_at_text.y_position + font_height - font_baseline; y++) {
            if (y >= 0 && y < matrix_height && x >= 0 && x < matrix_width) {
                canvas->SetPixel(x, y, black.r, black.g, black.b);
            }
        }
    }
    // Draw "Calling at:" text
    rgb_matrix::DrawText(canvas, font, 0, calling_at_text.y_position, white, calling_at_text.text.c_str());
}

void TrainServiceDisplay::renderScrollingMessage() {
    rgb_matrix::DrawText(canvas, font, nrcc_message_text.x_position, nrcc_message_text.y_position, white, nrcc_message_text.text.c_str());
    if (nrcc_message_text.x_position < 0) {
        rgb_matrix::DrawText(canvas, font, nrcc_message_text.x_position + matrix_width + nrcc_message_text.width, nrcc_message_text.y_position, white, nrcc_message_text.text.c_str());
    }
}


void TrainServiceDisplay::updateScrollPositions() {
    // Update calling points scroll position with wrap-around if the width of the calling points exceed available space.
    // scroll_calling_points is set in the updateDisplayContent method
    if (scroll_calling_points) {
        calling_points_text-- ;
        if (calling_points_text.x_position < -calling_points_text.width) {
            calling_points_text.x_position = matrix_width;
        }
    } else {calling_points_text.x_position = calling_at_text.width + 2;
        
    }
    //Update the message scroll position and set the complete flag once that's done.
    nrcc_message_text-- ;
    if (nrcc_message_text.x_position < -nrcc_message_text.width) {
        nrcc_message_text.x_position = matrix_width;
        message_scroll_complete = true; // Required to enable the toggle to Location (if set)
    }
}

void TrainServiceDisplay::checkFirstRowStateTransition(){
    auto now = std::chrono::steady_clock::now();
    
    // For ETD/Coach, toggle based on timer
    if (now - last_first_row_toggle >= std::chrono::seconds(ETD_coach_refresh_seconds)) {
        transitionFirstRowState();
        last_first_row_toggle = now;
    }
}


void TrainServiceDisplay::checkThirdRowStateTransition() {
    auto now = std::chrono::steady_clock::now();
    
    // For train information, toggle based on timer
    if (now - last_third_row_toggle >= std::chrono::seconds(third_line_refresh_seconds)) {
        transitionThirdRowState();
        last_third_row_toggle = now;
    }
}

void TrainServiceDisplay::checkFourthRowStateTransition() {
    auto now = std::chrono::steady_clock::now();
    bool should_toggle = false;
    
    if (!show_messages || !has_message) {
        // If messages are disabled or there aren't any, always show the location
        fourth_row_state = LOCATION;
        return;
    }
    
    if (fourth_row_state == MESSAGE) {
        // If we're showing a message, only toggle when scrolling is complete
        if (message_scroll_complete && now - last_fourth_row_toggle >= std::chrono::seconds(Message_Refresh_interval)) {
            should_toggle = true;
        }
    } else {
        // For location, toggle based on timer
        if (now - last_fourth_row_toggle >= std::chrono::seconds(Message_Refresh_interval)) {
            should_toggle = true;
        }
    }
    
    if (should_toggle) {
        transitionFourthRowState();
        last_fourth_row_toggle = now;
    }
}

void TrainServiceDisplay::transitionFirstRowState() {
    // Simply toggle between ETD and Number of Coaches
    first_row_state = (first_row_state == COACHES ? ETD : COACHES);
}

void TrainServiceDisplay::transitionThirdRowState() {
    // Simply toggle between 2nd and 3rd train only
    third_row_state = (third_row_state == SECOND_TRAIN) ? THIRD_TRAIN : SECOND_TRAIN;
}

void TrainServiceDisplay::transitionFourthRowState() {
    if (has_message) {
        // Toggle between location and message
        if (fourth_row_state == LOCATION) {
            fourth_row_state = MESSAGE;
            // Reset message scroll position and completion flag
            nrcc_message_text.x_position = matrix_width;
            message_scroll_complete = false;
        } else { // MESSAGE
            fourth_row_state = LOCATION;
        }
    } else {
        // If no message, always show clock
        fourth_row_state = LOCATION;
    }
}

void TrainServiceDisplay::refreshData() {
    // If a refresh is already pending, don't start another one
    DEBUG_PRINT("-----------------------");
    DEBUG_PRINT("Attempting to start background API refresh.");
    DEBUG_PRINT("Current API version: " << getCurrentAPIVersion() << ". Display version: " << getCurrentDisplayVersion() << ". Cache version: " << parser.getCurrentVersion());
    if (data_refresh_pending.load()) {
        return;
    }
    
    // Set flag to indicate a refresh is pending
    data_refresh_pending.store(true);
    
    // Start a new thread for the API call
    if (api_thread.joinable()) {
        api_thread.join(); // Clean up previous thread if any
    }
    
    api_thread = std::thread([this]() {
        try {
            // Fetch data from API
            std::string api_data = apiClient.fetchDepartures(
                config.get("from"), config.get("to"));
            
            // Store the data safely
            {
                std::lock_guard<std::mutex> lock(api_data_mutex);
                new_api_data = api_data;
            }
            
            // Set flags to indicate completion
            data_refresh_completed.store(true);
            data_refresh_pending.store(false);
            api_data_version.fetch_add(1, std::memory_order_release);
            
            DEBUG_PRINT("Background API refresh completed. API version: " << getCurrentAPIVersion() << ". Display version: " << getCurrentDisplayVersion() << ". Cache version: " << parser.getCurrentVersion());
        } catch (const std::exception& e) {
            std::cerr << "Error refreshing data in background thread: " << e.what() << std::endl;
            data_refresh_pending.store(false);
        }
    });
    
    // Detach the thread so it runs independently
    api_thread.detach();
}

void TrainServiceDisplay::run() {
    while (running) {
        try {
            // Check if it's time to start a new data refresh
            auto now = std::chrono::steady_clock::now();
            if (!data_refresh_pending.load() && 
                now - last_refresh >= std::chrono::seconds(
                refresh_interval_seconds)) {
                refreshData();
                last_refresh = std::chrono::steady_clock::now();
            }
            
            // Check if a data refresh has completed
            if (data_refresh_completed.load()) {
                // Apply the new data to the parser and update display
                DEBUG_PRINT("API refresh complete - updating cached data.");
                std::string api_data;
                {
                    std::lock_guard<std::mutex> lock(api_data_mutex);
                    api_data = new_api_data;
                }
                
                // Update the parser with new data
                parser.updateData(api_data);
                updateDisplayContent();
                
                // Reset the completion flag
                data_refresh_completed.store(false);
                display_data_version.fetch_add(1, std::memory_order_release);
                
                DEBUG_PRINT("Cache refreshed and display updated. API version: " << getCurrentAPIVersion() << ". Display version: " << getCurrentDisplayVersion() << ". Cache version: " << parser.getCurrentVersion());
            }
            
            // Remainder of the run method
            // Check for state transitions
            checkFirstRowStateTransition();
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
    
    // Clean up the API thread if we're shutting down
    if (api_thread.joinable()) {
        api_thread.join();
    }
}

void TrainServiceDisplay::stop() {
    running = false;

    // Clean up the API thread if it's still running
    if (api_thread.joinable()) {
        api_thread.join();
    }
}

TrainServiceDisplay::~TrainServiceDisplay() {
    // First, signal that we're shutting down
    running = false;
    
    // Clean up the API thread if it's still running
    if (api_thread.joinable()) {
        try {
            api_thread.join();
        } catch (const std::exception& e) {
            std::cerr << "Error joining API thread in destructor: " << e.what() << std::endl;
        }
    }
    
    // We don't need to delete the matrix pointer here as it's passed
    // in by the caller and should be managed outside this class.
    // Similarly, the parser and API client references are owned by the caller.
    
    DEBUG_PRINT("TrainServiceDisplay destroyed");
}

