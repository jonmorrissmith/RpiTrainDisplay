// Train Display - Configuration handler
// Jon Morris Smith - Feb 2025
// Version 1.0
// Instructions, fixes and issues at https://github.com/jonmorrissmith/RGB_Matrix_Train_Departure_Board
//
// IMPORTANT - settings in config.txt (if used) are used in preference to defaults set here.
//
//
#ifndef CONFIG_H
#define CONFIG_H

#include <led-matrix.h>
#include <map>
#include <string>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <memory>
#include <utility>

// Forward declaration for the debug printing macro
extern bool debug_mode;
#define DEBUG_PRINT(x) if(debug_mode) { std::cerr << x << std::endl; }

using namespace rgb_matrix;

class Config {
private:
    std::map<std::string, std::string> settings;
    
    const std::map<std::string, std::string> defaults = {
        {"from", ""},
        {"to", ""},
        {"ShowLocation", ""},
        {"APIURL", ""},
        {"APIkey", ""},
        {"Rail_Data_Marketplace", ""},
        {"fontPath", ""},
        {"scroll_slowdown_sleep_ms", "15"},
        {"refresh_interval_seconds", "60"},
        {"Message_Refresh_interval", "20"},
        {"matrixcols", "128"},
        {"matrixrows", "64"},
        {"matrixchain_length", "3"},
        {"matrixparallel", "1"},
        {"matrixhardware_mapping", "adafruit-hat-pwm"},
        {"gpio_slowdown", "4"},
        {"first_line_y", "18"},
        {"second_line_y", "38"},
        {"third_line_y", "58"},
        {"fourth_line_y", "72"},
        {"third_line_refresh_seconds", "10"},
        {"ETD_coach_refresh_seconds", "3"},
        {"ShowCallingPointETD", "Yes"},
        {"ShowMessages", "Yes"},
        {"ShowPlatforms", "Yes"},
        {"platform", ""},
        
        // RGB Matrix defaults
        {"led-multiplexing", "0"},
        {"led-pixel-mapper", ""},
        {"led-pwm-bits", "1"},
        {"led-brightness", "100"},
        {"led-scan-mode", "0"},
        {"led-row-addr-type", "0"},
        {"led-show-refresh", "false"},
        {"led-limit-refresh", "0"},
        {"led-inverse", "false"},
        {"led-rgb-sequence", "RGB"},
        {"led-pwm-lsb-nanoseconds", "130"},
        {"led-pwm-dither-bits", "0"},
        {"led-no-hardware-pulse", "false"},
        {"led-panel-type", ""},
        {"led-daemon", "false"},
        {"led-no-drop-privs", "false"},
        {"led-drop-priv-user", "daemon"},
        {"led-drop-priv-group", "daemon"}
    };

    // Cache for frequently requested configuration values
    mutable std::map<std::string, std::string> value_cache;

    // Private methods for matrix configuration
    void configureMatrixOptions(RGBMatrix::Options& options) const;
    void configureRuntimeOptions(RuntimeOptions& runtime_opt) const;
    
    // Helper to convert a string to lowercase for case-insensitive comparisons
    std::string toLower(std::string str) const {
        std::transform(str.begin(), str.end(), str.begin(),
                      [](unsigned char c){ return std::tolower(c); });
        return str;
    }
    
    // Helper to trim whitespace from beginning and end of string
    std::string trim(const std::string& str) const {
        const auto strBegin = str.find_first_not_of(" \t\r\n");
        if (strBegin == std::string::npos)
            return ""; // no content

        const auto strEnd = str.find_last_not_of(" \t\r\n");
        const auto strRange = strEnd - strBegin + 1;

        return str.substr(strBegin, strRange);
    }

public:
    Config();

    void loadFromFile(const std::string& filename);
    
    // Core configuration access method
    std::string get(const std::string& key) const;
    
    // Type-specific getters with default fallbacks
    std::string getStringWithDefault(const std::string& key, const std::string& defaultValue) const;
    int getInt(const std::string& key) const;
    int getIntWithDefault(const std::string& key, int defaultValue) const;
    bool getBool(const std::string& key) const;
    bool getBoolWithDefault(const std::string& key, bool defaultValue) const;
    
    // Configuration modification
    void set(const std::string& key, const std::string& value);
    
    // Clear cache after modifying configuration
    void clearCache() const;

    // Method to create a configured matrix
    RGBMatrix* createMatrix() const;
    
    // Validates if a key exists
    bool hasKey(const std::string& key) const;
    
    // Debug helper
    void debugPrintConfig() const;
};

#endif // CONFIG_H
