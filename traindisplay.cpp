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
#include <iostream>
#include <vector>
#include <string>
#include "config.h"
#include "api_client.h"
#include "train_service_parser.h"
#include "train_service_display.h"

// Global debug flag
bool debug_mode = false;

// Helper function to process command line arguments
void processCommandLineArgs(int argc, char* argv[], Config& config) {
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
                exit(1);
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
        config.set("refresh_interval_seconds", station_args[2]);
        DEBUG_PRINT("Overriding refresh interval with command line value: " << station_args[2]);
    }

    // Debug output of key configuration values
    DEBUG_PRINT("Final configuration:");
    DEBUG_PRINT("From: " << config.get("from"));
    DEBUG_PRINT("To: " << config.get("to"));
    DEBUG_PRINT("API URL: " << config.get("APIURL"));
    DEBUG_PRINT("Show Calling Point ETD: " << config.getBool("ShowCallingPointETD"));
    DEBUG_PRINT("Show Messages: " << config.getBool("ShowMessages"));
}

int main(int argc, char* argv[]) {
    try {
        // Initialize configuration
        Config config;
        processCommandLineArgs(argc, argv, config);
   
        // Create matrix from config
        RGBMatrix* matrix = config.createMatrix();
        
        // Create API client
        TrainAPIClient apiClient(config.get("APIURL"));
        
        // Make initial API call and set up parser
        std::string api_data = apiClient.fetchDepartures(config.get("from"), config.get("to"));
        TrainServiceParser parser;
        parser.updateData(api_data);
        
        // Create and run the display
        TrainServiceDisplay display(matrix, parser, apiClient, config);
        display.run();
        
        // Cleanup
        delete matrix;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}


