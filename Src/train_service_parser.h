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
#ifndef TRAIN_SERVICE_PARSER_H
#define TRAIN_SERVICE_PARSER_H

#include <nlohmann/json.hpp>
#include <string>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <array>
#include <iostream>

using json = nlohmann::json;

// Forward declaration for the debug printing macro
extern bool debug_mode;
#define DEBUG_PRINT(x) if(debug_mode) { std::cerr << x << std::endl; }
#define DEBUG_PRINT_JSON(x) if(debug_mode) { std::cerr << std::setw(4) << x << std::endl; }

class TrainServiceParser {
private:
    json data;
    std::mutex dataMutex;
    bool showCallingPointETD;  // Added flag for ETD display
    std::array<int, 3> PlatformServices;

    // Helper method to strip HTML tags from text
    std::string processHtmlTags(const std::string& html);

public:
    TrainServiceParser();
    
    void setShowCallingPointETD(bool show);
    void updateData(const std::string& jsonString);
    
    void findServicesAtPlatform(const std::string& selectedPlatform);
    int returnServiceAtPlatform(int serviceNumber);
    
    size_t getNumberOfServices();
    std::string getScheduledDepartureTime(size_t serviceIndex);
    std::string getEstimatedDepartureTime(size_t serviceIndex);
    std::string getPlatform(size_t serviceIndex);
    std::string getDestinationLocation(size_t serviceIndex);
    std::string getLocationNameList(size_t serviceIndex);
    std::string getDelayReason(size_t serviceIndex);
    std::string getNrccMessages();
};

#endif // TRAIN_SERVICE_PARSER_H
