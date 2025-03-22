// Train Display - Configuration handler
// Jon Morris Smith - Feb 2025
// Version 1.0
// Instructions, fixes and issues at https://github.com/jonmorrissmith/RGB_Matrix_Train_Departure_Board
//
// IMPORTANT - set defaults in config.h
//

#include "config.h"

Config::Config() {
    settings = defaults;
}

void Config::loadFromFile(const std::string& filename) {
    DEBUG_PRINT("Loading configuration from " << filename);

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

std::string Config::get(const std::string& key) const {
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

std::string Config::getStringWithDefault(const std::string& key, const std::string& defaultValue) const {
    try {
        return get(key);
    } catch (const std::exception& e) {
        DEBUG_PRINT("Warning: Using default for " << key);
        return defaultValue;
    }
}

int Config::getInt(const std::string& key) const {
    return std::stoi(get(key));
}

int Config::getIntWithDefault(const std::string& key, int defaultValue) const {
    try {
        return getInt(key);
    } catch (const std::exception& e) {
        DEBUG_PRINT("Warning: Using default for " << key);
        return defaultValue;
    }
}

bool Config::getBool(const std::string& key) const {
    return get(key) == "true" || get(key) == "Yes";
}

bool Config::getBoolWithDefault(const std::string& key, bool defaultValue) const {
    try {
        return getBool(key);
    } catch (const std::exception& e) {
        DEBUG_PRINT("Warning: Using default for " << key);
        return defaultValue;
    }
}

void Config::set(const std::string& key, const std::string& value) {
    settings[key] = value;
}

RGBMatrix* Config::createMatrix() const {
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

void Config::configureMatrixOptions(RGBMatrix::Options& options) const {
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

void Config::configureRuntimeOptions(RuntimeOptions& runtime_opt) const {
    runtime_opt.gpio_slowdown = getIntWithDefault("gpio_slowdown", 1);
    
    // Handle daemon mode if supported
    if (getBoolWithDefault("led-daemon", false)) {
        runtime_opt.daemon = 1;
    }
}

