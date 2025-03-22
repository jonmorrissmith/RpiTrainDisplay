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
#include "train_service_parser.h"

TrainServiceParser::TrainServiceParser() : showCallingPointETD(true) {
     showCallingPointETD = true;
     selectPlatform = false;
     ServiceList.fill(999);
}

void TrainServiceParser::updateData(const std::string& jsonString) {
    std::lock_guard<std::mutex> lock(dataMutex);
    try {
        data = json::parse(jsonString);
        ServiceList.fill(999);
    } catch (const json::parse_error& e) {
        throw std::runtime_error("Failed to parse JSON: " + std::string(e.what()));
    }
}

void TrainServiceParser::setShowCallingPointETD(bool show) {
    showCallingPointETD = show;
}

void TrainServiceParser::setSelectedPlatform(const std::string& platform) {
    std::lock_guard<std::mutex> lock(dataMutex);

    // set the flag to find departures for a specific platform and store the platform
    selectPlatform = true;
    selected_platform = platform;
}

void TrainServiceParser::unsetSelectedPlatform() {
    std::lock_guard<std::mutex> lock(dataMutex);
    
    // set the flag to find first three departures
    selectPlatform = false;
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

void TrainServiceParser::findServices() {
    std::lock_guard<std::mutex> lock(dataMutex);
    size_t i;
    size_t numServices;
    
    try {
        // Check if train services exist in the data
        if (data.find("trainServices") == data.end() ||
            data["trainServices"].is_null() ||
            data["trainServices"].empty()) {
            DEBUG_PRINT("No train services found in the data");
            return;
        }
        
        // Set the number of services
        numServices = data["trainServices"].size();
        
        // Clear saved services
        ServiceList.fill(999);
        
        if (selectPlatform) {
            // Find departures for the selected platform
            size_t serviceCount = 0;

            DEBUG_PRINT("Searching for services at platform " << selected_platform);
            
            // Iterate through all train services
            for (i = 0; i < numServices && serviceCount < ServiceList.size(); ++i) {
                const auto& service = data["trainServices"][i];
                
                // First check if the platform field exists
                if (service.contains("platform")) {
                    
                    // Now check if platform value matches the selected platform
                    if (!service["platform"].is_null() &&
                        service["platform"].get<std::string>() == selected_platform) {

                        // Found a service for the selected platform, add its index to the array
                        // DEBUG_PRINT("Found service for platform " << selected_platform << " at position " << i);
                        ServiceList[serviceCount] = i;

                        // Increment the count of services found
                        ++serviceCount;
                    }
                } else {
                    DEBUG_PRINT("iteration " << i << " does not contain a platform field");
                }
            }

        } else {
            // Find the first 3 departures
            for (i = 0; i < 3; i++) {
                if (i < numServices) {
                    ServiceList[i] = i;
                }
            }
        }

        // Debug information about the found service
        if (debug_mode) {
            if (selectPlatform) {
                DEBUG_PRINT("Finding the first 3 departures for platform " << selected_platform);
            } else {
                DEBUG_PRINT("Finding the first 3 departures");
            }
            for (i=0; i < 3; i++) {
                if (ServiceList[i] == 999) {
                    DEBUG_PRINT("Index " << i << " - Service " << ServiceList[i] <<". No service found");
                } else {
                    DEBUG_PRINT("Index " << i << " - Service " << ServiceList[i]);
                    if(!data["trainServices"][ServiceList[i]]["platform"].is_null()) {
                        DEBUG_PRINT("    Platform " << data["trainServices"][ServiceList[i]]["platform"].get<std::string>());
                    }
                    DEBUG_PRINT("    Destination: " << data["trainServices"][ServiceList[i]]["destination"][0]["locationName"].get<std::string>()
                                << " - Scheduled departure: " << data["trainServices"][ServiceList[i]]["std"].get<std::string>()
                                << " - Estimated departure: " << data["trainServices"][ServiceList[i]]["etd"].get<std::string>());
                }
            }
        }
    } catch (const json::exception& e) {
        DEBUG_PRINT("Error finding services: " << e.what());
    }
}

size_t TrainServiceParser::getFirstDeparture() {
    std::lock_guard<std::mutex> lock(dataMutex);
    return ServiceList[0];
}

size_t TrainServiceParser::getSecondDeparture(){
    std::lock_guard<std::mutex> lock(dataMutex);
    return ServiceList[1];
}

size_t TrainServiceParser::getThirdDeparture(){
    std::lock_guard<std::mutex> lock(dataMutex);
    return ServiceList[2];
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
            } else if (i + 2 < html.length() && html.substr(i, 2) == "\n") {
                result += "";
                i += 1; // Skip the entity
            } else {
                result += html[i];
            }
        }
    }
    return result;
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

std::string TrainServiceParser::getDestination(size_t serviceIndex) {
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

std::string TrainServiceParser::getCallingPoints(size_t serviceIndex) {
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

bool TrainServiceParser::isCancelled(size_t serviceIndex) {
    std::lock_guard<std::mutex> lock(dataMutex);
    try {
        if (serviceIndex >= data["trainServices"].size()) {
            throw std::out_of_range("Service index out of range");
        }
        return data["trainServices"][serviceIndex]["isCancelled"].get<bool>();
    } catch (const json::exception& e) {
        DEBUG_PRINT("Error getting cancelled status: " << e.what());
        return false; // Return false in case of error
    }
}

std::string TrainServiceParser::getCancelReason(size_t serviceIndex) {
    std::lock_guard<std::mutex> lock(dataMutex);
    try {
        if (serviceIndex >= data["trainServices"].size()) {
            throw std::out_of_range("Service index out of range");
        }
        // If cancelReason exists, is not null, and isn't empty, return it
        if (data["trainServices"][serviceIndex].find("cancelReason") != data["trainServices"][serviceIndex].end() &&
            !data["trainServices"][serviceIndex]["cancelReason"].is_null() &&
            !data["trainServices"][serviceIndex]["cancelReason"].get<std::string>().empty()) {
            return data["trainServices"][serviceIndex]["cancelReason"].get<std::string>();
        }
        return ""; // No cancellation or no reason provided
    } catch (const json::exception& e) {
        DEBUG_PRINT("Error getting cancellation reason: " << e.what());
        return ""; // Return empty string in case of error
    }
}

bool TrainServiceParser::isDelayed(size_t serviceIndex) {
    std::lock_guard<std::mutex> lock(dataMutex);
    try {
        if (serviceIndex >= data["trainServices"].size()) {
            throw std::out_of_range("Service index out of range");
        }
        // Check if the service is delayed
        std::string etd = data["trainServices"][serviceIndex]["etd"].get<std::string>();
        if (etd != "On time" && etd != "Cancelled" ) {
            return true;
        } else {
            return false;
        }
    } catch (const json::exception& e) {
        DEBUG_PRINT("Error getting cancelled status: " << e.what());
        return false; // Return false in case of error
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
        if (etd != "On time" && etd != "Cancelled" ) {
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

std::string TrainServiceParser::getadhocAlerts(size_t serviceIndex){
    std::lock_guard<std::mutex> lock(dataMutex);
    try {
        if (serviceIndex >= data["trainServices"].size()) {
            throw std::out_of_range("Service index out of range");
        }
        // If adhocAlerts exists, is not null, and isn't empty, return it
        if (data["trainServices"][serviceIndex].find("adhocAlerts") != data["trainServices"][serviceIndex].end() &&
            !data["trainServices"][serviceIndex]["adhocAlerts"].is_null() &&
            !data["trainServices"][serviceIndex]["adhocAlerts"].get<std::string>().empty()) {
            return data["trainServices"][serviceIndex]["adhocAlerts"].get<std::string>();
        }
        return ""; // No adhoc alert
    } catch (const json::exception& e) {
        DEBUG_PRINT("Error getting Adhoc Alerts: " << e.what());
        return ""; // Return empty string in case of error
    }
}



std::string TrainServiceParser::getCoaches(size_t serviceIndex, bool addText) {
    std::lock_guard<std::mutex> lock(dataMutex);
    size_t coaches;
    std::string ss;
    try {
        if (serviceIndex >= data["trainServices"].size()) {
            throw std::out_of_range("Service index out of range");
        }
        // If coaches exists (NRE data), is not null, and isn't empty
        if (data["trainServices"][serviceIndex].find("coaches") != data["trainServices"][serviceIndex].end() &&
            !data["trainServices"][serviceIndex]["coaches"].is_null() &&
            !data["trainServices"][serviceIndex]["coaches"].get<std::string>().empty()) {
            ss = data["trainServices"][serviceIndex]["coaches"].get<std::string>();
        }
        // I length exists (Raildata Marketplace), is not 0, and isn't empty, return it
        if (data["trainServices"][serviceIndex].find("length") != data["trainServices"][serviceIndex].end() &&
            !data["trainServices"][serviceIndex]["length"].is_null()) {
            coaches = data["trainServices"][serviceIndex]["length"].get<size_t>();
            if (coaches !=0) {
                ss = std::to_string(coaches);
            }
        }
        if (ss.empty()) {
            return ""; // No coach data
        } else {
            if (addText) {
                ss = " formed of " + ss + " coaches";
            }
            return ss;
        }
    } catch (const json::exception& e) {
        DEBUG_PRINT("Error getting coach information " << e.what());
        return ""; // Return empty string in case of error
    }
}

std::string TrainServiceParser::getOperator(size_t serviceIndex) {
    std::lock_guard<std::mutex> lock(dataMutex);
    try {
        if (serviceIndex >= data["trainServices"].size()) {
            throw std::out_of_range("Service index out of range");
        }
        // If operator exists, is not null, and isn't empty, return it
        if (data["trainServices"][serviceIndex].find("operator") != data["trainServices"][serviceIndex].end() &&
            !data["trainServices"][serviceIndex]["operator"].is_null() &&
            !data["trainServices"][serviceIndex]["operator"].get<std::string>().empty()) {
            return "   A " + data["trainServices"][serviceIndex]["operator"].get<std::string>() + " service";
        }
        return ""; // No operator
    } catch (const json::exception& e) {
        DEBUG_PRINT("Error getting operator information " << e.what());
        return ""; // Return empty string in case of error
    }
}



std::string TrainServiceParser::getNrccMessages() {
    std::lock_guard<std::mutex> lock(dataMutex);
    std::string output;
    char c;
    try {
        if (data.find("nrccMessages") != data.end() &&
            data["nrccMessages"].is_array() &&
            !data["nrccMessages"].empty()) {

            std::stringstream ss;
            
            for (size_t i = 0; i < data["nrccMessages"].size(); ++i) {
                if (i > 0) ss << " | ";
                
                std::string message;
                const auto& messageObj = data["nrccMessages"][i];
                
                // Try both "Value" and "value" field names
                if (messageObj.contains("Value") && !messageObj["Value"].is_null()) {
                    message = messageObj["Value"].get<std::string>();
                    // DEBUG_PRINT("Found NRCC message with 'Value' field (capital V)" );
                }
                else if (messageObj.contains("value") && !messageObj["value"].is_null()) {
                    message = messageObj["value"].get<std::string>();
                    // DEBUG_PRINT("Found NRCC message with 'value' field (lowercase V");
                }
                else {
                    DEBUG_PRINT("Message at index " << i << " has neither 'Value' nor 'value' field, or it's null");
                    continue; // Skip this message
                }
                
                // Process HTML tags
                message = processHtmlTags(message);
                ss << message;
            }
            output = ss.str();
            c = output[0];
            if (int(c)=10) {
                output.erase(0,1);
            }
            return output;
        }
        return "";
    } catch (const json::exception& e) {
        DEBUG_PRINT("Error getting NRCC messages: " << e.what());
        return "";
    }
}

