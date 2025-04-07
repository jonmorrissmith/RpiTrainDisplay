// Train Display - Configuration handler
// Jon Morris Smith - Feb 2025
// Version 1.0
// Instructions, fixes and issues at https://github.com/jonmorrissmith/RGB_Matrix_Train_Departure_Board
//
// IMPORTANT - set defaults in config.h
//

#include "config.h"

Config::Config() {
    // Start with defaults
    settings = defaults;
    DEBUG_PRINT("Configuration initialized with default values");
}

void Config::loadFromFile(const std::string& filename) {
    DEBUG_PRINT("Loading configuration from " << filename);

    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open config file: " + filename);
    }

    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;
        
        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = trim(line.substr(0, pos));
        std::string value = trim(line.substr(pos + 1));

        if (!key.empty()) {
            // Set the value, even if it's empty - we'll handle fallbacks in get()
            settings[key] = value;
            DEBUG_PRINT("Loaded config: " << key << " = " << (value.empty() ? "<empty>" : value));
        }
    }
    
    // Clear cache after loading new configuration
    clearCache();
    
    DEBUG_PRINT("Configuration loaded successfully from " << filename);
}

std::string Config::get(const std::string& key) const {
    // First check the cache
    auto cache_it = value_cache.find(key);
    if (cache_it != value_cache.end()) {
        return cache_it->second;
    }
    
    std::string result;
    
    // Check if it exists in settings from config.txt
    auto it = settings.find(key);
    if (it != settings.end() && !it->second.empty()) {
        result = it->second;
    } else {
        // Fall back to defaults
        auto default_it = defaults.find(key);
        if (default_it != defaults.end()) {
            if (!default_it->second.empty()) {
                result = default_it->second;
            } else {
                // Both settings and defaults have empty values
                if (key == "to" || key == "platform" || key == "led-pixel-mapper" || key == "led-panel-type") {
                    // These keys are allowed to be empty
                    result = "";
                } else {
                    DEBUG_PRINT("Warning: Configuration key '" << key << "' has empty value in both config file and defaults");
                    result = "";
                }
            }
        } else {
            throw std::runtime_error("Configuration key not found: " + key);
        }
    }
    
    // Cache the result
    value_cache[key] = result;
    return result;
}

std::string Config::getStringWithDefault(const std::string& key, const std::string& defaultValue) const {
    try {
        std::string value = get(key);
        if (value.empty()) {
            DEBUG_PRINT("Warning: Using provided default for empty key " << key);
            return defaultValue;
        }
        return value;
    } catch (const std::exception& e) {
        DEBUG_PRINT("Warning: " << e.what() << " - Using provided default");
        return defaultValue;
    }
}

int Config::getInt(const std::string& key) const {
    std::string value = get(key);
    if (value.empty()) {
        throw std::runtime_error("Cannot convert empty string to integer for key: " + key);
    }
    
    try {
        return std::stoi(value);
    } catch (const std::exception& e) {
        throw std::runtime_error("Invalid integer value for key '" + key + "': " + value);
    }
}

int Config::getIntWithDefault(const std::string& key, int defaultValue) const {
    try {
        return getInt(key);
    } catch (const std::exception& e) {
        DEBUG_PRINT("Warning: " << e.what() << " - Using default value " << defaultValue);
        return defaultValue;
    }
}

bool Config::getBool(const std::string& key) const {
    std::string value = toLower(get(key));
    // Check for empty string
    if (value.empty()) {
        throw std::runtime_error("Empty boolean value for key: " + key);
    }
    
    // Check for various true values
    if (value == "true" || value == "yes" || value == "1" || value == "on") {
        return true;
    }
    
    // Check for various false values
    if (value == "false" || value == "no" || value == "0" || value == "off") {
        return false;
    }
    
    // If it's neither clearly true nor false, throw an exception
    throw std::runtime_error("Invalid boolean value for key '" + key + "': " + value);
}

bool Config::getBoolWithDefault(const std::string& key, bool defaultValue) const {
    try {
        return getBool(key);
    } catch (const std::exception& e) {
        DEBUG_PRINT("Warning: " << e.what() << " - Using default value " << (defaultValue ? "true" : "false"));
        return defaultValue;
    }
}

void Config::set(const std::string& key, const std::string& value) {
    settings[key] = value;
    // Clear cache entry if it exists
    value_cache.erase(key);
    DEBUG_PRINT("Set config: " << key << " = " << value);
}

void Config::clearCache() const {
    value_cache.clear();
    DEBUG_PRINT("Configuration cache cleared");
}

bool Config::hasKey(const std::string& key) const {
    return settings.find(key) != settings.end() || defaults.find(key) != defaults.end();
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
    
    // Set hardware mapping - must be stored in a static variable to ensure c_str() remains valid
    static std::string hardware_mapping_str = get("matrixhardware_mapping");
    options.hardware_mapping = hardware_mapping_str.c_str();
    
    // Set multiplexing
    options.multiplexing = getIntWithDefault("led-multiplexing", 0);
    
    // Handle pixel mapper if set - also needs to be static
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
    
    // Handle RGB sequence - must be exactly 3 characters and static
    static std::string rgb_sequence_str = getStringWithDefault("led-rgb-sequence", "RGB");
    if (rgb_sequence_str.length() == 3) {
        options.led_rgb_sequence = rgb_sequence_str.c_str();
    } else {
        DEBUG_PRINT("Warning: led-rgb-sequence must be exactly 3 characters. Using default 'RGB'.");
        static std::string default_rgb = "RGB";
        options.led_rgb_sequence = default_rgb.c_str();
    }
    
    // Advanced PWM settings
    options.pwm_lsb_nanoseconds = getIntWithDefault("led-pwm-lsb-nanoseconds", 130);
    options.pwm_dither_bits = getIntWithDefault("led-pwm-dither-bits", 0);
    options.disable_hardware_pulsing = getBoolWithDefault("led-no-hardware-pulse", false);
    
    // Handle panel type if set - also needs to be static
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

void Config::debugPrintConfig() const {
    DEBUG_PRINT("Current configuration:");
    for (const auto& pair : settings) {
        DEBUG_PRINT("  " << pair.first << " = " << (pair.second.empty() ? "<empty>" : pair.second));
    }
}
