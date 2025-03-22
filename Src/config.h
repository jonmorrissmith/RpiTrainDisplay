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

    // Safe getter with default fallback
    template <typename T>
    T getWithDefault(const std::string& key, const T& defaultValue) const;

    // Private methods for matrix configuration
    void configureMatrixOptions(RGBMatrix::Options& options) const;
    void configureRuntimeOptions(RuntimeOptions& runtime_opt) const;

public:
    Config();

    void loadFromFile(const std::string& filename);
    std::string get(const std::string& key) const;
    std::string getStringWithDefault(const std::string& key, const std::string& defaultValue) const;
    int getInt(const std::string& key) const;
    int getIntWithDefault(const std::string& key, int defaultValue) const;
    bool getBool(const std::string& key) const;
    bool getBoolWithDefault(const std::string& key, bool defaultValue) const;
    void set(const std::string& key, const std::string& value);

    // Method to create a configured matrix
    RGBMatrix* createMatrix() const;
};

#endif // CONFIG_H

