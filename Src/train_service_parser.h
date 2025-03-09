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
#include <iostream>

using json = nlohmann::json;

// Forward declaration for the debug printing macro
extern bool debug_mode;
#define DEBUG_PRINT(x) if(debug_mode) { std::cerr << x << std::endl; }

class TrainServiceParser {
private:
    json data;
    std::mutex dataMutex;
    bool showCallingPointETD;  // Added flag for ETD display

    // Helper method to strip HTML tags from text
    std::string processHtmlTags(const std::string& html) {
        std::string result;
        bool inTag = false;
        for (size_t i = 0; i < html.length(); ++i) {
            if (html[i] == '<') {
                inTag = true;
            } else if (html[i] == '>') {
                inTag = false;
            } else if (!inTag) {
                // Handle HTML entity for quotes
                if (i + 5 < html.length() && html.substr(i, 5) == "&quot;") {
                    result += "\"";
                    i += 4; // Skip the entity (& is already processed)
                } else if (i + 4 < html.length() && html.substr(i, 4) == "&lt;") {
                    result += "<";
                    i += 3; // Skip the entity
                } else if (i + 4 < html.length() && html.substr(i, 4) == "&gt;") {
                    result += ">";
                    i += 3; // Skip the entity
                } else if (i + 5 < html.length() && html.substr(i, 5) == "&amp;") {
                    result += "&";
                    i += 4; // Skip the entity
                } else {
                    result += html[i];
                }
            }
        }
        return result;
    }

public:
    TrainServiceParser() : showCallingPointETD(true) {}
    
    // Set configuration options
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
            if (data.find("trainServices") != data.end() && 
                !data["trainServices"].is_null()) {
                return data["trainServices"].size();
            }
            return 0;
        } catch (const json::exception& e) {
            DEBUG_PRINT("Error getting number of services: " << e.what());
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

    // Get location name list with estimated times
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
                
                // Add time in brackets if ETD display is enabled
                if (showCallingPointETD) {
                    // Check if 'et' exists and is not empty
                    if (callingPoints[i].find("et") != callingPoints[i].end() && 
                        !callingPoints[i]["et"].get<std::string>().empty()) {
                        
                        // Get the estimated time
                        std::string etValue = callingPoints[i]["et"].get<std::string>();
                        
                        if (etValue == "On time") {
                            // If the train is on time, display the scheduled time instead
                            if (callingPoints[i].find("st") != callingPoints[i].end() && 
                                !callingPoints[i]["st"].get<std::string>().empty()) {
                                ss << " (" << callingPoints[i]["st"].get<std::string>() << ")";
                            }
                        } else {
                            // For any other value (delayed, etc.), display the estimated time
                            ss << " (" << etValue << ")";
                        }
                    }
                }
            }
            return ss.str();
        } catch (const json::exception& e) {
            throw std::runtime_error("Error creating location name list: " + std::string(e.what()));
        }
    }

    // Get the delay reason for a service
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
            DEBUG_PRINT("Error getting delay reason: " << e.what());
            return ""; // Return empty string in case of error
        }
    }

    // Get NRCC messages
    std::string getNrccMessages() {
        std::lock_guard<std::mutex> lock(dataMutex);
        try {
            if (data.find("nrccMessages") != data.end() && 
                data["nrccMessages"].is_array() && 
                !data["nrccMessages"].empty()) {
                
                std::stringstream ss;
                ss << "MESSAGE: ";
                
                for (size_t i = 0; i < data["nrccMessages"].size(); ++i) {
                    if (i > 0) ss << " | ";
                    
                    // Extract the value from the message
                    std::string message = data["nrccMessages"][i]["value"].get<std::string>();
                    
                    // Process HTML tags
                    message = processHtmlTags(message);
                    
                    ss << message;
                }
                return ss.str();
            }
            return "";
        } catch (const json::exception& e) {
            DEBUG_PRINT("Error getting NRCC messages: " << e.what());
            return "";
        }
    }
};

#endif // TRAIN_SERVICE_PARSER_H

