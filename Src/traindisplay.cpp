// Train Display - an RGB matrix departure board for the Raspberry Pi
// Jon Morris Smith - Feb 2025
// Version 1.0
// Instructions, fixes and issues at https://github.com/jonmorrissmith/RGB_Matrix_Train_Departure_Board
//
// IMPORTANT - replace XXX in config.h with your default settings
//
// With thanks to:
// https://github.com/jpsingleton/Huxley2
// https://github.com/hzeller/rpi-rgb-led-matrix
// https://github.com/nlohmann/json
//
#include <signal.h>
#include <iostream>
#include <vector>
#include <string>
#include "config.h"
#include "api_client.h"
#include "train_service_parser.h"
#include "train_service_display.h"

// Global debug flag
bool debug_mode = false;

// Pointer to display for signal handling
TrainServiceDisplay* display_ptr = nullptr;

// Signal handler for graceful shutdown
void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ". Shutting down..." << std::endl;
    if (display_ptr) {
        display_ptr->stop();
    }
}

// Display usage information
void showUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [OPTIONS] [FROM_STATION] [TO_STATION] [REFRESH_INTERVAL]\n"
              << "Options:\n"
              << "  -d, --debug               Enable debug output\n"
              << "  -f, --config FILE         Specify configuration file\n"
              << "  -h, --help                Show this help message\n"
              << "\nExample:\n"
              << "  " << programName << " KGX YRK 60\n"
              << "    Shows trains from London Kings Cross to York, refreshing every 60 seconds\n";
}

// Helper function to process command line arguments
void processCommandLineArgs(int argc, char* argv[], Config& config) {
    std::string config_file;
    std::vector<std::string> station_args;

    // First pass - handle config file and debug mode
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-d" || arg == "--debug") {
            debug_mode = true;
        } else if (arg == "-h" || arg == "--help") {
            showUsage(argv[0]);
            exit(0);
        } else if (arg == "-f" || arg == "--config") {
            if (i + 1 < argc) {
                config_file = argv[++i];
            } else {
                std::cerr << "Error: Config file path not provided after " << arg << std::endl;
                showUsage(argv[0]);
                exit(1);
            }
        } else if (arg.substr(0, 9) == "--config=") {
            config_file = arg.substr(9);
        } else if (arg[0] != '-') {
            // Store non-option arguments for second pass
            station_args.push_back(arg);
        } else {
            std::cerr << "Unknown option: " << arg << std::endl;
            showUsage(argv[0]);
            exit(1);
        }
    }

    // Load configuration file if specified
    if (!config_file.empty()) {
        try {
            DEBUG_PRINT("Loading config from: " << config_file);
            config.loadFromFile(config_file);
        } catch (const std::exception& e) {
            std::cerr << "Error loading config file: " << e.what() << std::endl;
            exit(1);
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
        // Validate that the refresh interval is a number
        try {
            int refresh = std::stoi(station_args[2]);
            if (refresh < 10) {
                std::cerr << "Warning: Refresh interval too short, setting to minimum (10 seconds)" << std::endl;
                config.set("refresh_interval_seconds", "10");
            } else {
                config.set("refresh_interval_seconds", station_args[2]);
            }
            DEBUG_PRINT("Overriding refresh interval with command line value: " << config.get("refresh_interval_seconds"));
        } catch (const std::exception& e) {
            std::cerr << "Error: Invalid refresh interval. Using default." << std::endl;
        }
    }

    // Validate required configuration
    if (config.get("from").empty()) {
        std::cerr << "Error: FROM_STATION is required. Please specify a station code." << std::endl;
        showUsage(argv[0]);
        exit(1);
    }

    // Debug output of key configuration values
    DEBUG_PRINT("Final configuration:");
    DEBUG_PRINT("From: " << config.get("from"));
    DEBUG_PRINT("To: " << config.get("to"));
    DEBUG_PRINT("API URL: " << config.get("APIURL"));
    DEBUG_PRINT("Show Calling Point ETD: " << config.getBool("ShowCallingPointETD"));
    DEBUG_PRINT("Show Messages: " << config.getBool("ShowMessages"));
    DEBUG_PRINT("Show Platforms: " << config.getBool("ShowPlatforms"));
}

int main(int argc, char* argv[]) {
    // Set up signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    try {
        // Initialize configuration
        Config config;
        processCommandLineArgs(argc, argv, config);
   
        // Create matrix from config
        RGBMatrix* matrix = config.createMatrix();
        if (!matrix) {
            std::cerr << "Failed to create RGB Matrix. Check your hardware configuration." << std::endl;
            return 1;
        }
        
        // Create API client
        TrainAPIClient apiClient(config.get("APIURL"));
        
        // Make initial API call and set up parser
        std::string api_data;
        try {
            api_data = apiClient.fetchDepartures(config.get("from"), config.get("to"));
        } catch (const std::exception& e) {
            std::cerr << "Failed to fetch initial train data: " << e.what() << std::endl;
            std::cerr << "Check internet connection and station codes." << std::endl;
            delete matrix;
            return 1;
        }
        
        TrainServiceParser parser;
        parser.updateData(api_data);
        
        // Create and run the display
        TrainServiceDisplay display(matrix, parser, apiClient, config);
        display_ptr = &display; // Set global pointer for signal handler
        
        std::cout << "Train display running. Press Ctrl+C to exit." << std::endl;
        display.run();
        
        // Cleanup
        delete matrix;
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Train display shut down successfully." << std::endl;
    return 0;
}

