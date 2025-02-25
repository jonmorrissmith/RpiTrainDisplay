// Train Service Display - an RGB matrix departure board for the Raspberry Pi
// Jon Morris Smith - Feb 2025
// Version 1.0
// Instructions, fixes and issues at https://github.com/jonmorrissmith/RGB_Matrix_Train_Departure_Board
//
// IMPORTANT - replace XXX in the Config class with your default settings
//
// With thanks to:
// https://github.com/jpsingleton/Huxley2
// https://github.com/hzeller/rpi-rgb-led-matrix
// https://github.com/nlohmann/json
//
#include <nlohmann/json.hpp>
#include <led-matrix.h>
#include <graphics.h>
#include <thread>
#include <chrono>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>
#include <curl/curl.h>
#include <atomic>
#include <mutex>
#include <iostream>
#include <sstream>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <limits.h>

using json = nlohmann::json;
using namespace rgb_matrix;

// Global debug flag
bool debug_mode = false;

// Debug print macro
#define DEBUG_PRINT(x) if(debug_mode) { std::cerr << x << std::endl; }

// Callback function to handle API response
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Configuration class
class Config {
private:
    std::map<std::string, std::string> settings;
    
    const std::map<std::string, std::string> defaults = {
        {"from", "SAC"},
        {"to", "STP"},
        {"APIURL", "https://api.anotherpartialsuccess.com"},
        {"fontPath", "/home/display/rpi-rgb-led-matrix/fonts/9x18.bdf"},
        {"scroll_slowdown_sleep_ms", "50"},
        {"refresh_interval_seconds", "60"},
        {"matrixcols", "128"},
        {"matrixrows", "64"},
        {"matrixchain_length", "3"},
        {"matrixparallel", "1"},
        {"matrixhardware_mapping", "adafruit-hat-pwm"},
        {"gpio_slowdown", "4"},
        {"first_line_y", "18"},
        {"second_line_y", "38"},
        {"third_line_y", "58"},
        {"third_line_refresh_seconds", "10"},
        {"ShowCallingPointETD", "Yes"}  // Added default for ETD display
    };

public:
    Config() {
        settings = defaults;
    }

    void loadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open config file: " + filename);
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;
            size_t pos = line.find('=');
            if (pos == std::string::npos) continue;

            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);

            if (!key.empty() && !value.empty()) {
                settings[key] = value;
            }
        }
    }

    std::string get(const std::string& key) const {
        auto it = settings.find(key);
        if (it != settings.end() && !it->second.empty()) {
            return it->second;
        }
        auto default_it = defaults.find(key);
        if (default_it != defaults.end()) {
            return default_it->second;
        }
        throw std::runtime_error("Configuration key not found: " + key);
    }

    int getInt(const std::string& key) const {
        return std::stoi(get(key));
    }

    void set(const std::string& key, const std::string& value) {
        settings[key] = value;
    }
};

// Train Service Parser class
class TrainServiceParser {
private:
    json data;
    std::mutex dataMutex;
    bool showCallingPointETD;  // Added flag for ETD display

public:
    TrainServiceParser() : showCallingPointETD(true) {}  // Initialize with default value
    
    // Add setter method for the config option
    void setShowCallingPointETD(bool show) {
        showCallingPointETD = show;
    }

    void updateData(const std::string& jsonString) {
        std::lock_guard<std::mutex> lock(dataMutex);
        try {
            data = json::parse(jsonString);
        } catch (const json::parse_error& e) {
            throw std::runtime_error("Failed to parse JSON: " + std::string(e.what()));
        }
    }

    size_t getNumberOfServices() {
        std::lock_guard<std::mutex> lock(dataMutex);
        try {
            return data["trainServices"].size();
        } catch (const json::exception& e) {
            return 0;
        }
    }

    std::string getScheduledDepartureTime(size_t serviceIndex) {
        std::lock_guard<std::mutex> lock(dataMutex);
        try {
            if (serviceIndex >= data["trainServices"].size()) {
                throw std::out_of_range("Service index out of range");
            }
            return data["trainServices"][serviceIndex]["std"].get<std::string>();
        } catch (const json::exception& e) {
            throw std::runtime_error("Error getting scheduled departure time: " + std::string(e.what()));
        }
    }

    std::string getEstimatedDepartureTime(size_t serviceIndex) {
        std::lock_guard<std::mutex> lock(dataMutex);
        try {
            if (serviceIndex >= data["trainServices"].size()) {
                throw std::out_of_range("Service index out of range");
            }
            return data["trainServices"][serviceIndex]["etd"].get<std::string>();
        } catch (const json::exception& e) {
            throw std::runtime_error("Error getting estimated departure time: " + std::string(e.what()));
        }
    }

    std::string getDestinationLocation(size_t serviceIndex) {
        std::lock_guard<std::mutex> lock(dataMutex);
        try {
            if (serviceIndex >= data["trainServices"].size()) {
                throw std::out_of_range("Service index out of range");
            }
            return data["trainServices"][serviceIndex]["destination"][0]["locationName"].get<std::string>();
        } catch (const json::exception& e) {
            throw std::runtime_error("Error getting destination location: " + std::string(e.what()));
        }
    }

    // Modified to include ETDs based on configuration
    std::string getLocationNameList(size_t serviceIndex) {
        std::lock_guard<std::mutex> lock(dataMutex);
        try {
            if (serviceIndex >= data["trainServices"].size()) {
                throw std::out_of_range("Service index out of range");
            }
            const auto& service = data["trainServices"][serviceIndex];
            const auto& callingPoints = service["subsequentCallingPoints"][0]["callingPoint"];
            
            std::stringstream ss;
            for (size_t i = 0; i < callingPoints.size(); ++i) {
                if (i > 0) ss << ", ";
                ss << callingPoints[i]["locationName"].get<std::string>();
                
                // Add estimated time in brackets if available and if enabled in config
                if (showCallingPointETD && 
                    callingPoints[i].find("et") != callingPoints[i].end() && 
                    !callingPoints[i]["et"].get<std::string>().empty()) {
                    ss << " (" << callingPoints[i]["et"].get<std::string>() << ")";
                }
            }
            return ss.str();
        } catch (const json::exception& e) {
            throw std::runtime_error("Error creating location name list: " + std::string(e.what()));
        }
    }

    // Added method to get delay reason
    std::string getDelayReason(size_t serviceIndex) {
        std::lock_guard<std::mutex> lock(dataMutex);
        try {
            if (serviceIndex >= data["trainServices"].size()) {
                throw std::out_of_range("Service index out of range");
            }
            
            // Check if the service is delayed
            std::string etd = data["trainServices"][serviceIndex]["etd"].get<std::string>();
            if (etd != "On time" && etd != "Cancelled" && etd != "Delayed") {
                // If delayReason exists and isn't empty, return it
                if (data["trainServices"][serviceIndex].find("delayReason") != data["trainServices"][serviceIndex].end() && 
                    !data["trainServices"][serviceIndex]["delayReason"].get<std::string>().empty()) {
                    return data["trainServices"][serviceIndex]["delayReason"].get<std::string>();
                }
            }
            return ""; // No delay or no reason provided
        } catch (const json::exception& e) {
            return ""; // Return empty string in case of error
        }
    }
};

// API calling function
std::string callAPI(const std::string& from, const std::string& destination, const std::string& baseUrl) {
    CURL* curl = curl_easy_init();
    std::string readBuffer;
    
    if(curl) {
        std::string url;
        if(destination.empty()) {
            url = baseUrl + "/departures/" + from + "/3?expand=true";
        } else {
            url = baseUrl + "/departures/" + from + "/to/" + destination + "/3?expand=true";
        }

        DEBUG_PRINT("Making API call to: " << url);

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        CURLcode res = curl_easy_perform(curl);
        
        if(res != CURLE_OK) {
            curl_easy_cleanup(curl);
            throw std::runtime_error("Failed to make API call: " + std::string(curl_easy_strerror(res)));
        }

        curl_easy_cleanup(curl);
        
        DEBUG_PRINT("API Response: " << readBuffer);
    } else {
        throw std::runtime_error("Failed to initialize CURL");
    }

    return readBuffer;
}

class TrainServiceDisplay {
private:
    RGBMatrix* matrix;
    FrameCanvas* canvas;
    Font font;
    Color white;
    Color black;
    TrainServiceParser& parser;
    std::atomic<bool> running;
    Config& config;

public:
    TrainServiceDisplay(RGBMatrix* m, TrainServiceParser& p, Config& cfg) 
        : matrix(m), canvas(m->CreateFrameCanvas()),
          white(255, 255, 255), black(0, 0, 0), 
          parser(p), running(true), config(cfg) {
    
        // Set parser options from config
        parser.setShowCallingPointETD(config.get("ShowCallingPointETD") == "Yes");
    
        if (!font.LoadFont(config.get("fontPath").c_str())) {
            throw std::runtime_error("Font loading failed for: " + config.get("fontPath"));
        }
        DEBUG_PRINT("Display initialized with font: " << config.get("fontPath"));
    }

    void run() {
        while (running) {
            try {
                json data;
                size_t num_services = 0;
                try {
                    std::string api_data = callAPI(config.get("from"), config.get("to"), 
                                                 config.get("APIURL"));
                    parser.updateData(api_data);
                    num_services = parser.getNumberOfServices();
                    DEBUG_PRINT("Number of services available: " << num_services);
                } catch (const std::exception& e) {
                    std::cerr << "API Error: " << e.what() << std::endl;
                    std::this_thread::sleep_for(std::chrono::seconds(30));
                    continue;
                }

                if (num_services == 0) {
                    DEBUG_PRINT("No services available");
                    std::this_thread::sleep_for(std::chrono::seconds(30));
                    continue;
                }

                // Get all the text content
                std::string top_line = parser.getScheduledDepartureTime(0) + " " + 
                                     parser.getDestinationLocation(0) + " " + 
                                     parser.getEstimatedDepartureTime(0);
                
                std::string calling_points = parser.getLocationNameList(0);

                // Get delay reason if the first train is delayed
                std::string delay_reason = parser.getDelayReason(0);
                if (!delay_reason.empty()) {
                    calling_points += " - DELAY REASON: " + delay_reason;
                }

                std::string second_line = num_services > 1 ? 
                    "2nd " + parser.getScheduledDepartureTime(1) + " " + 
                    parser.getDestinationLocation(1) + " " + 
                    parser.getEstimatedDepartureTime(1) : "No more services";
                
                std::string third_line = num_services > 2 ? 
                    "3rd " + parser.getScheduledDepartureTime(2) + " " + 
                    parser.getDestinationLocation(2) + " " + 
                    parser.getEstimatedDepartureTime(2) : "No more services";

                // Get vertical positions from config
                int first_line_y = config.getInt("first_line_y");
                int second_line_y = config.getInt("second_line_y");
                int third_line_y = config.getInt("third_line_y");

                // Animation loop
                int matrix_width = matrix->width();
                int calling_points_width = 0;
                for (const char& c : calling_points) {
                    calling_points_width += font.CharacterWidth(c);
                }
                
                int scroll_x = matrix_width;
                bool show_second = true;
                auto last_toggle = std::chrono::steady_clock::now();
                auto last_refresh = std::chrono::steady_clock::now();

                while (running) {
                    canvas->Clear();

                    // Draw static top line
                    rgb_matrix::DrawText(canvas, font, 0, first_line_y, white, top_line.c_str());

                    // Draw scrolling calling points with wrap-around
                    rgb_matrix::DrawText(canvas, font, scroll_x, second_line_y, white, calling_points.c_str());
                    if (scroll_x < 0) {
                        rgb_matrix::DrawText(canvas, font, scroll_x + matrix_width + calling_points_width, 
                                           second_line_y, white, calling_points.c_str());
                    }

                    // Handle third line alternating text
                    auto now = std::chrono::steady_clock::now();
                    if (now - last_toggle >= std::chrono::seconds(
                        config.getInt("third_line_refresh_seconds"))) {
                        show_second = !show_second;
                        last_toggle = now;
                    }

                    // Draw current third line text
                    rgb_matrix::DrawText(canvas, font, 0, third_line_y, white,
                                       (show_second ? second_line : third_line).c_str());

                    // Update display
                    canvas = matrix->SwapOnVSync(canvas);
                    
                    // Update scroll position with wrap-around
                    scroll_x--;
                    if (scroll_x < -calling_points_width) {
                        scroll_x = matrix_width;
                    }
                    
                    std::this_thread::sleep_for(std::chrono::milliseconds(
                        config.getInt("scroll_slowdown_sleep_ms")));

                    // Check if it's time to refresh the data
                    if (now - last_refresh >= std::chrono::seconds(
                        config.getInt("refresh_interval_seconds"))) {
                        break;
                    }
                }

                // Refresh data
                std::string api_data = callAPI(config.get("from"), config.get("to"), 
                                             config.get("APIURL"));
                parser.updateData(api_data);
                last_refresh = std::chrono::steady_clock::now();

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

int main(int argc, char* argv[]) {
    Config config;
    std::string config_file;
    std::vector<std::string> station_args;

    // First pass - handle config file and debug mode
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-d" || arg == "--debug") {
            debug_mode = true;
        } else if (arg == "-f" || arg == "--config") {
            if (i + 1 < argc) {
                config_file = argv[++i];
            } else {
                std::cerr << "Error: Config file path not provided after " << arg << std::endl;
                return 1;
            }
        } else if (arg.substr(0, 9) == "--config=") {
            config_file = arg.substr(9);
        } else if (arg[0] != '-') {
            // Store non-option arguments for second pass
            station_args.push_back(arg);
        }
    }

    // Load configuration file if specified
    if (!config_file.empty()) {
        try {
            DEBUG_PRINT("Loading config from: " << config_file);
            config.loadFromFile(config_file);
        } catch (const std::exception& e) {
            std::cerr << "Error loading config file: " << e.what() << std::endl;
            return 1;
        }
    }

    // Process station arguments
    if (station_args.size() > 0) {
        config.set("from", station_args[0]);
        DEBUG_PRINT("Overriding 'from' with command line value: " << station_args[0]);
    }
    if (station_args.size() > 1) {
        config.set("to", station_args[1]);
        DEBUG_PRINT("Overriding 'to' with command line value: " << station_args[1]);
    }
    if (station_args.size() > 2) {
        config.set("refresh_interval_seconds", station_args[2]);
        DEBUG_PRINT("Overriding refresh interval with command line value: " << station_args[2]);
    }

    // Debug output
    DEBUG_PRINT("Final configuration:");
    DEBUG_PRINT("From: " << config.get("from"));
    DEBUG_PRINT("To: " << config.get("to"));
    DEBUG_PRINT("API URL: " << config.get("APIURL"));
    DEBUG_PRINT("Show Calling Point ETD: " << config.get("ShowCallingPointETD"));

try {
    RGBMatrix::Options matrix_options;
    matrix_options.rows = config.getInt("matrixrows");
    matrix_options.cols = config.getInt("matrixcols");
    matrix_options.chain_length = config.getInt("matrixchain_length");
    matrix_options.parallel = config.getInt("matrixparallel");

    // Set hardware mapping
    static std::string hardware_mapping_str = config.get("matrixhardware_mapping");
    matrix_options.hardware_mapping = hardware_mapping_str.c_str();
    
    // Add new RGB Matrix parameters from config - with error handling
    try {
        matrix_options.multiplexing = config.getInt("led-multiplexing");
    } catch (const std::exception& e) {
        DEBUG_PRINT("Warning: using default multiplexing");
    }
    
    // Handle pixel mapper if set
    try {
        static std::string pixel_mapper_str = config.get("led-pixel-mapper");
        if (!pixel_mapper_str.empty()) {
            matrix_options.pixel_mapper_config = pixel_mapper_str.c_str();
        }
    } catch (const std::exception& e) {
        DEBUG_PRINT("Warning: using default pixel mapper");
    }
    
    try {
        matrix_options.pwm_bits = config.getInt("led-pwm-bits");
    } catch (const std::exception& e) {
        DEBUG_PRINT("Warning: using default pwm bits");
    }
    
    try {
        matrix_options.brightness = config.getInt("led-brightness");
    } catch (const std::exception& e) {
        DEBUG_PRINT("Warning: using default brightness");
    }
    
    try {
        matrix_options.scan_mode = config.getInt("led-scan-mode");
    } catch (const std::exception& e) {
        DEBUG_PRINT("Warning: using default scan mode");
    }
    
    try {
        matrix_options.row_address_type = config.getInt("led-row-addr-type");
    } catch (const std::exception& e) {
        DEBUG_PRINT("Warning: using default row address type");
    }
    
    // Handle boolean parameters with string comparison
    try {
        matrix_options.show_refresh_rate = (config.get("led-show-refresh") == "true");
    } catch (const std::exception& e) {
        DEBUG_PRINT("Warning: using default refresh rate display setting");
    }
    
    // Handle refresh rate limit
    try {
        matrix_options.limit_refresh_rate_hz = config.getInt("led-limit-refresh");
    } catch (const std::exception& e) {
        DEBUG_PRINT("Warning: using default refresh rate limit");
    }
    
    // Set inverse colors
    try {
        matrix_options.inverse_colors = (config.get("led-inverse") == "true");
    } catch (const std::exception& e) {
        DEBUG_PRINT("Warning: using default color inversion setting");
    }
    
    // Handle RGB sequence
    try {
        static std::string rgb_sequence_str = config.get("led-rgb-sequence");
        if (!rgb_sequence_str.empty() && rgb_sequence_str.length() == 3) {
            matrix_options.led_rgb_sequence = rgb_sequence_str.c_str();
        } else {
            DEBUG_PRINT("Warning: led-rgb-sequence must be exactly 3 characters. Using default.");
        }
    } catch (const std::exception& e) {
        DEBUG_PRINT("Warning: using default RGB sequence");
    }
    
    // Set PWM LSB nanoseconds
    try {
        matrix_options.pwm_lsb_nanoseconds = config.getInt("led-pwm-lsb-nanoseconds");
    } catch (const std::exception& e) {
        DEBUG_PRINT("Warning: using default PWM LSB nanoseconds");
    }
    
    // Set PWM dither bits
    try {
        matrix_options.pwm_dither_bits = config.getInt("led-pwm-dither-bits");
    } catch (const std::exception& e) {
        DEBUG_PRINT("Warning: using default PWM dither bits");
    }
    
    // Handle hardware pulse setting
    try {
        matrix_options.disable_hardware_pulsing = (config.get("led-no-hardware-pulse") == "true");
    } catch (const std::exception& e) {
        DEBUG_PRINT("Warning: using default hardware pulsing setting");
    }
    
    // Handle panel type if set
    try {
        static std::string panel_type_str = config.get("led-panel-type");
        if (!panel_type_str.empty()) {
            matrix_options.panel_type = panel_type_str.c_str();
        }
    } catch (const std::exception& e) {
        DEBUG_PRINT("Warning: using default panel type");
    }
    
    // Runtime options - using only the supported options
    RuntimeOptions runtime_opt;
    runtime_opt.gpio_slowdown = config.getInt("gpio_slowdown");
    
    // Handle daemon mode if supported in your version
    try {
        if (config.get("led-daemon") == "true") {
            runtime_opt.daemon = 1;
        }
    } catch (const std::exception& e) {
        DEBUG_PRINT("Warning: using default daemon mode setting");
    }
    
    DEBUG_PRINT("Creating matrix with hardware mapping: '" << hardware_mapping_str << "'");
    
    RGBMatrix* matrix = RGBMatrix::CreateFromOptions(matrix_options, runtime_opt);
    if (matrix == nullptr) {
        throw std::runtime_error("Could not create matrix");
    }

        // Make initial API call
        std::string api_data = callAPI(config.get("from"), config.get("to"), 
                                     config.get("APIURL"));
        
        // Create parser and display
        TrainServiceParser parser;
        parser.updateData(api_data);
        
        // Set the ShowCallingPointETD configuration value from config
        parser.setShowCallingPointETD(config.get("ShowCallingPointETD") == "Yes");

        TrainServiceDisplay display(matrix, parser, config);
        display.run();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
