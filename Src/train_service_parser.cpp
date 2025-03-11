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
#include "train_service_parser.h"
#include <iomanip>

TrainServiceParser::TrainServiceParser() : showCallingPointETD(true) {
     showCallingPointETD = true;
     PlatformServices.fill(-1);
} 

void TrainServiceParser::setShowCallingPointETD(bool show) {
    showCallingPointETD = show;
}

void TrainServiceParser::findServicesAtPlatform(const std::string& selectedPlatform) {
    std::lock_guard<std::mutex> lock(dataMutex);
    
    // Reset the PlatformServices array to all empty strings (no services)
    PlatformServices.fill(-1);  
    
    try {
        // Check if train services exist in the data
        if (data.find("trainServices") == data.end() || 
            data["trainServices"].is_null() || 
            data["trainServices"].empty()) {
            DEBUG_PRINT("No train services found in the data");
            return;
        }
        
        size_t serviceCount = 0;
        
        DEBUG_PRINT("Searching for services at platform " << selectedPlatform);
        
        // Iterate through all train services
        for (size_t i = 0; i < data["trainServices"].size() && serviceCount < PlatformServices.size(); ++i) {
            const auto& service = data["trainServices"][i];
            
            // Check if platform exists and matches the selected platform
            if (!service["platform"].is_null() && 
                service["platform"].get<std::string>() == selectedPlatform) {
                
                // Found a service for the selected platform, add its serviceID to the array
                // PlatformServices[serviceCount] = service["serviceID"].get<std::string>();  // Changed from static_cast<int>(i)
                PlatformServices[serviceCount] = i;
                
                // Debug information about the found service
                DEBUG_PRINT("Found service at index " << i << " for platform " << selectedPlatform 
                    << " - Destination: " << service["destination"][0]["locationName"].get<std::string>()
                    << " - Scheduled departure: " << service["std"].get<std::string>()
                    << " - Status: " << service["etd"].get<std::string>());
                
                // Increment the count of services found
                ++serviceCount;
            }
        }
        
        // Log the results
        DEBUG_PRINT("Services found for platform " << selectedPlatform << ": " << serviceCount);
        // for (size_t i = 0; i < PlatformServices.size(); ++i) {
        //        DEBUG_PRINT("  PlatformServices[" << (i) << "] - index: " << PlatformServices[i]);
        //    }
    } catch (const json::exception& e) {
        DEBUG_PRINT("Error finding services at platform " << selectedPlatform << ": " << e.what());
    }
}

int TrainServiceParser::returnServiceAtPlatform(int serviceNumber) {
    std::lock_guard<std::mutex> lock(dataMutex);

    try {
        // Check if serviceNumber is valid (1-based indexing)
        if (serviceNumber < 1 || serviceNumber > PlatformServices.size()) {
           DEBUG_PRINT("Query for service " << serviceNumber << " of " << PlatformServices.size() << " stored. Returning -1 ");
           return -1;
        }
        return PlatformServices[serviceNumber - 1];
    } catch (const json::exception& e) {
        DEBUG_PRINT("Error returning service " << serviceNumber << " at selected platform: " << e.what());
        return -1;
    }
}

std::string TrainServiceParser::processHtmlTags(const std::string& html) {
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

void TrainServiceParser::updateData(const std::string& jsonString) {
    std::lock_guard<std::mutex> lock(dataMutex);
    try {
        data = json::parse(jsonString);
        // Uncomment to see the whole JSON
        // DEBUG_PRINT_JSON(data);
    } catch (const json::parse_error& e) {
        throw std::runtime_error("Failed to parse JSON: " + std::string(e.what()));
    }
}

size_t TrainServiceParser::getNumberOfServices() {
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

std::string TrainServiceParser::getScheduledDepartureTime(size_t serviceIndex) {
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

std::string TrainServiceParser::getEstimatedDepartureTime(size_t serviceIndex) {
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

std::string TrainServiceParser::getPlatform(size_t serviceIndex) {
    std::lock_guard<std::mutex> lock(dataMutex);
    try {
        if (serviceIndex >= data["trainServices"].size()) {
            throw std::out_of_range("Service index out of range");
        }
        if (data["trainServices"][serviceIndex]["platform"].is_null()) {
           return ""; 
        }
        return "Plat " + data["trainServices"][serviceIndex]["platform"].get<std::string>();
    } catch (const json::exception& e) {
        throw std::runtime_error("Error getting platform: " + std::string(e.what()));
    }
}

std::string TrainServiceParser::getDestinationLocation(size_t serviceIndex) {
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

std::string TrainServiceParser::getLocationNameList(size_t serviceIndex) {
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
                // Check if 'et' exists, is not null, and is not empty
                if (callingPoints[i].find("et") != callingPoints[i].end() && 
                    !callingPoints[i]["et"].is_null() &&
                    !callingPoints[i]["et"].get<std::string>().empty()) {
                    
                    // Get the estimated time
                    std::string etValue = callingPoints[i]["et"].get<std::string>();
                    
                    if (etValue == "On time") {
                        // If the train is on time, display the scheduled time instead
                        if (callingPoints[i].find("st") != callingPoints[i].end() && 
                            !callingPoints[i]["st"].is_null() &&
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
        DEBUG_PRINT(data);
        throw std::runtime_error("Error creating location name list: " + std::string(e.what()));
    }
}

std::string TrainServiceParser::getDelayReason(size_t serviceIndex) {
    std::lock_guard<std::mutex> lock(dataMutex);
    try {
        if (serviceIndex >= data["trainServices"].size()) {
            throw std::out_of_range("Service index out of range");
        }
        
        // Check if the service is delayed
        std::string etd = data["trainServices"][serviceIndex]["etd"].get<std::string>();
        if (etd != "On time" && etd != "Cancelled" && etd != "Delayed") {
            // If delayReason exists, is not null, and isn't empty, return it
            if (data["trainServices"][serviceIndex].find("delayReason") != data["trainServices"][serviceIndex].end() && 
                !data["trainServices"][serviceIndex]["delayReason"].is_null() &&
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

std::string TrainServiceParser::getNrccMessages() {
    std::lock_guard<std::mutex> lock(dataMutex);
    try {
        if (data.find("nrccMessages") != data.end() && 
            data["nrccMessages"].is_array() && 
            !data["nrccMessages"].empty()) {
            
            std::stringstream ss;
            
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

