// Train Display - an RGB matrix departure board for the Raspberry Pi
// JSON parsing
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
#include <iomanip>

using json = nlohmann::json;

// Forward declaration for the debug printing macro
extern bool debug_mode;
#define DEBUG_PRINT(x) if(debug_mode) { std::cerr << x << std::endl; }
#define DEBUG_PRINT_JSON(x) if(debug_mode) { std::cerr << std::setw(4) << x << std::endl; }

class TrainServiceParser {
private:
    json data;                             // Departure data
    std::mutex dataMutex;                  // Process control

    bool showCallingPointETD;              // Flag to show Estimated Time of Departure in calling points
    bool selectPlatform;                   // Flag to indicate whether departures for a specific platform are selected

    std::string selected_platform;         // Store the selected platform
    std::array<size_t, 3> ServiceList;     // Array for the 1st, 2nd and 3rd departures

    std::string processHtmlTags(const std::string& html);     // Helper method to strip HTML tags from text

public:
    TrainServiceParser();  //Constructor
    
    void setShowCallingPointETD(bool show);                   // Yes/No - whether to show departure times for calling points
    void setSelectedPlatform(const std::string& platform);    // Set a specific platform - departures will be found for that platform
    void unsetSelectedPlatform();                             // Unset the selected platform - departures will be found for all platforms
    void updateData(const std::string& jsonString);           // Update with new JSON data
    
    void findServices();                                      // Find the next 3 services - takes into account whether a specific platform has been set

    bool isCancelled(size_t serviceIndex);                    // Yes/No - is the specified departure cancelled
    bool isDelayed(size_t serviceIndex);                      // Yes/No - is the specified departure delayed

    size_t getNumberOfServices();                             // Return the number of Services in the JSON
    size_t getFirstDeparture();                               // Return the index for the first departure
    size_t getSecondDeparture();                              // Return the index for the second departure
    size_t getThirdDeparture();                               // Return the index for the third departure

    std::string getScheduledDepartureTime(size_t serviceIndex);  //Return the scheduled departure time for the selected service
    std::string getEstimatedDepartureTime(size_t serviceIndex);  //Return the estimated departure time for the selected service
    std::string getPlatform(size_t serviceIndex);                //Return the platform for the selected service
    std::string getDestination(size_t serviceIndex);             //Return the destination for the selected service
    std::string getCallingPoints(size_t serviceIndex);           //Return the calling points for the selected service
    std::string getCoaches(size_t serviceIndex, bool addText);   //Return the number of coaches for the selected service - if addText is true then return in the "Formed of x coaches" format
    std::string getOperator(size_t serviceIndex);                //Return the operator of the selected service
    std::string getNrccMessages();                               //Return any Network Rail messages
    
    std::string getDelayReason(size_t serviceIndex);  //Return the delay reason for the selected service
    std::string getCancelReason(size_t serviceIndex); //Return the cancellation reason time for the selected service
    std::string getadhocAlerts(size_t serviceIndex);  //Return any adhoc alerts for the selected service
};

#endif // TRAIN_SERVICE_PARSER_H

