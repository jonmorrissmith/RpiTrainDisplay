// Train Display - an RGB matrix departure board for the Raspberry Pi
// Jon Morris Smith - Feb 2025
// Version 1.0
// Instructions, fixes and issues at https://github.com/jonmorrissmith/RGB_Matrix_Train_Departure_Board
//
// IMPORTANT - replace XXX with your default settings
//
// With thanks to:
// https://github.com/jpsingleton/Huxley2
// https://github.com/hzeller/rpi-rgb-led-matrix
// https://github.com/nlohmann/json
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
        {"from", "XXX"},
        {"to", "XXX"},
        {"APIURL", "https://XXX"},
        {"fontPath", "/home/XXX/rpi-rgb-led-matrix/fonts/9x18.bdf"},
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
        {"ShowCallingPointETD", "Yes"},
        {"ShowMessages", "Yes"},
        
        // RGB Matrix defaults
        {"led-multiplexing", "0"},
        {"led-pixel-mapper", ""},
        {"led-pwm-bits", "11"},
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

    std::string getStringWithDefault(const std::string& key, const std::string& defaultValue) const {
        try {
            return get(key);
        } catch (const std::exception& e) {
            DEBUG_PRINT("Warning: Using default for " << key);
            return defaultValue;
        }
    }

    int getInt(const std::string& key) const {
        return std::stoi(get(key));
    }

    int getIntWithDefault(const std::string& key, int defaultValue) const {
        try {
            return getInt(key);
        } catch (const std::exception& e) {
            DEBUG_PRINT("Warning: Using default for " << key);
            return defaultValue;
        }
    }

    bool getBool(const std::string& key) const {
        return get(key) == "true" || get(key) == "Yes";
    }

    bool getBoolWithDefault(const std::string& key, bool defaultValue) const {
        try {
            return getBool(key);
        } catch (const std::exception& e) {
            DEBUG_PRINT("Warning: Using default for " << key);
            return defaultValue;
        }
    }

    void set(const std::string& key, const std::string& value) {
        settings[key] = value;
    }

    // New method to create a configured matrix
    RGBMatrix* createMatrix() const {
        RGBMatrix::Options matrix_options;
        RuntimeOptions runtime_opt;

        // Configure the matrix options
        configureMatrixOptions(matrix_options);
        
        // Configure runtime options
        configureRuntimeOptions(runtime_opt);
        
        DEBUG_PRINT("Creating matrix with hardware mapping: '" << get("matrixhardware_mapping") << "'");
        
        RGBMatrix* matrix = RGBMatrix::CreateFromOptions(matrix_options, runtime_opt);
        if (matrix == nullptr) {
            throw std::runtime_error("Could not create matrix");
        }
        
        return matrix;
    }

private:
    void configureMatrixOptions(RGBMatrix::Options& options) const {
        // Basic matrix configuration
        options.rows = getIntWithDefault("matrixrows", 64);
        options.cols = getIntWithDefault("matrixcols", 128);
        options.chain_length = getIntWithDefault("matrixchain_length", 3);
        options.parallel = getIntWithDefault("matrixparallel", 1);
        
        // Set hardware mapping
        static std::string hardware_mapping_str = get("matrixhardware_mapping");
        options.hardware_mapping = hardware_mapping_str.c_str();
        
        // Set multiplexing
        options.multiplexing = getIntWithDefault("led-multiplexing", 0);
        
        // Handle pixel mapper if set
        static std::string pixel_mapper_str = getStringWithDefault("led-pixel-mapper", "");
        if (!pixel_mapper_str.empty()) {
            options.pixel_mapper_config = pixel_mapper_str.c_str();
        }
        
        // Display quality settings
        options.pwm_bits = getIntWithDefault("led-pwm-bits", 11);
        options.brightness = getIntWithDefault("led-brightness", 100);
        options.scan_mode = getIntWithDefault("led-scan-mode", 0);
        options.row_address_type = getIntWithDefault("led-row-addr-type", 0);
        
        // Display behavior settings
        options.show_refresh_rate = getBoolWithDefault("led-show-refresh", false);
        options.limit_refresh_rate_hz = getIntWithDefault("led-limit-refresh", 0);
        
        // Color settings
        options.inverse_colors = getBoolWithDefault("led-inverse", false);
        
        // Handle RGB sequence - must be exactly 3 characters
        static std::string rgb_sequence_str = getStringWithDefault("led-rgb-sequence", "RGB");
        if (!rgb_sequence_str.empty() && rgb_sequence_str.length() == 3) {
            options.led_rgb_sequence = rgb_sequence_str.c_str();
        } else {
            DEBUG_PRINT("Warning: led-rgb-sequence must be exactly 3 characters. Using default.");
        }
        
        // Advanced PWM settings
        options.pwm_lsb_nanoseconds = getIntWithDefault("led-pwm-lsb-nanoseconds", 130);
        options.pwm_dither_bits = getIntWithDefault("led-pwm-dither-bits", 0);
        options.disable_hardware_pulsing = getBoolWithDefault("led-no-hardware-pulse", false);
        
        // Handle panel type if set
        static std::string panel_type_str = getStringWithDefault("led-panel-type", "");
        if (!panel_type_str.empty()) {
            options.panel_type = panel_type_str.c_str();
        }
    }
    
    void configureRuntimeOptions(RuntimeOptions& runtime_opt) const {
        runtime_opt.gpio_slowdown = getIntWithDefault("gpio_slowdown", 1);
        
        // Handle daemon mode if supported
        if (getBoolWithDefault("led-daemon", false)) {
            runtime_opt.daemon = 1;
        }
    }
};

#endif // CONFIG_H

