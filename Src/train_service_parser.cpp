// Train Display - an RGB matrix departure board for the Raspberry Pi
// JSON parsing
// Jon Morris Smith - Feb 2025
// Version 1.0
// Instructions, fixes and issues at https://github.com/jonmorrissmith/RGB_Matrix_Train_Departure_Board
//
// With thanks to:
// https://github.com/nlohmann/json
//
#include "train_service_parser.h"

TrainServiceParser::TrainServiceParser() : showCallingPointETD(true) {
    showCallingPointETD = true;
    selectPlatform = false;
    ServiceList.fill(999);
    data_version = 1;
}

TrainServiceParser::TrainServiceInfo TrainServiceParser::getService(size_t serviceIndex) {
    std::lock_guard<std::mutex> lock(dataMutex);
    
    try {
        if (serviceIndex >= number_of_services) {
            throw std::out_of_range("Service index out of range");
        }
        
        return Services[serviceIndex];
    } catch (const json::exception& e) {
        throw std::runtime_error("Error getting service data-structure: " + std::string(e.what()));
    }
}

void TrainServiceParser::debugPrintServiceStruct(size_t serviceIndex) {
    try {
        if (serviceIndex >= number_of_services) {
            throw std::out_of_range("Service index out of range");
        }
        std::cout << "Service: " << serviceIndex << std::endl;
        
        std::cout << "scheduledTime: " << Services[serviceIndex].scheduledTime << std::endl;
        std::cout << "estimatedTime: " << Services[serviceIndex].estimatedTime << std::endl;
        std::cout << "platform: " << Services[serviceIndex].platform << std::endl;
        std::cout << "destination: " << Services[serviceIndex].destination << std::endl;
        std::cout << "callingPoints: " << Services[serviceIndex].callingPoints << std::endl;
        std::cout << "callingPoints_with_ETD: " << Services[serviceIndex].callingPoints_with_ETD << std::endl;
        std::cout << "operator_name: " << Services[serviceIndex].operator_name << std::endl;
        std::cout << "coaches: " << Services[serviceIndex].coaches << std::endl;
        
        std::cout << "isCancelled: " << Services[serviceIndex].isCancelled << std::endl;
        std::cout << "cancelReason: " << Services[serviceIndex].cancelReason << std::endl;
        
        std::cout << "isDelayed: " << Services[serviceIndex].isDelayed << std::endl;
        std::cout << "delayReason: " << Services[serviceIndex].delayReason << std::endl;
        
        std::cout << "adhocAlerts: " << Services[serviceIndex].adhocAlerts << std::endl;
        
    } catch (const json::exception& e) {
        throw std::runtime_error("Error printing service data-structure: " + std::string(e.what()));
    }
}

void TrainServiceParser::updateData(const std::string& jsonString) {
    json new_data;
    TrainServiceInfo NewServiceInfo;
    std::vector<TrainServiceInfo> parsed_services;
    size_t i;
    size_t coaches;
    std::stringstream ss;
    char c;
    try {
        new_data = json::parse(jsonString);
        
        std::array<size_t, 3> new_service_list;
        new_service_list.fill(999);
        
        // Parse the Meta-data in departure JSON
        
        // Number of Services
        if (new_data.find("trainServices") != new_data.end() &&
            !new_data["trainServices"].is_null()) {
            number_of_services = new_data["trainServices"].size();
        } else {
            number_of_services =0;
        }
        DEBUG_PRINT("Parsing data - " << number_of_services << " services in data");
        
        parsed_services.reserve(number_of_services);
        
        // Location
        if (new_data.find("locationName") != new_data.end() &&
            !new_data["locationName"].is_null() &&
            !new_data["locationName"].empty()) {
            location_name = new_data["locationName"];
        } else {
            location_name = "";
        }
        
        // NRCC messages
        if (new_data.find("nrccMessages") != new_data.end() &&
            new_data["nrccMessages"].is_array() &&
            !new_data["nrccMessages"].empty()) {
                          
            for (size_t i = 0; i < new_data["nrccMessages"].size(); ++i) {
                if (i > 0) ss << " | ";
                std::string message;
                const auto& messageObj = new_data["nrccMessages"][i];
                    
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
                // Remove any /n from the beginning of the message
                c = message[0];
                if (int(c) == 10) {
                    message.erase(0,1);
                }
                ss << message;
                }
            NRCC_message = ss.str();
        } else {
            NRCC_message = "";
        }
        
        // Parse the Services
        // Populate the data=structure for all services in departure JSON
        // Note - calling-points are lazy-loaded in a separate method
        // std::string scheduledTime;
        // std::string estimatedTime;
        // std::string platform;
        // std::string destination;
        // std::string operator_name;
        // std::string coaches;
        // bool isCancelled;
        // bool isDelayed;
        // std::string cancelReason;
        // std::string delayReason;
        // std::string adhocAlerts;
        
        for (i=0; i< number_of_services; i++){
            // scheduledTime
            NewServiceInfo.scheduledTime =  new_data["trainServices"][i]["std"].get<std::string>();
            
            // estimatedTime
            if (new_data["trainServices"][i]["etd"].is_null()) {
                NewServiceInfo.estimatedTime = "null";
            } else {
                NewServiceInfo.estimatedTime = new_data["trainServices"][i]["etd"].get<std::string>();
            }
            
            // platform
            if (new_data["trainServices"][i]["platform"].is_null()) {
                NewServiceInfo.platform = "";
            } else {
                NewServiceInfo.platform = new_data["trainServices"][i]["platform"].get<std::string>();
            }
            
            // destination
            NewServiceInfo.destination = new_data["trainServices"][i]["destination"][0]["locationName"].get<std::string>();
            
            // operator_name
            if (new_data["trainServices"][i].find("operator") != new_data["trainServices"][i].end() &&
                !new_data["trainServices"][i]["operator"].is_null() &&
                !new_data["trainServices"][i]["operator"].get<std::string>().empty()) {
                NewServiceInfo.operator_name = "A " + new_data["trainServices"][i]["operator"].get<std::string>() + " service";
            } else {
                NewServiceInfo.operator_name = "";
            }
            
            // coaches
            NewServiceInfo.coaches = "";
            // If coaches exists (NRE data), is not null, and isn't empty
            if (new_data["trainServices"][i].find("coaches") != new_data["trainServices"][i].end() &&
                !new_data["trainServices"][i]["coaches"].is_null() &&
                !new_data["trainServices"][i]["coaches"].get<std::string>().empty()) {
                NewServiceInfo.coaches = new_data["trainServices"][i]["coaches"].get<std::string>();
            }
            
            // If length exists (Raildata Marketplace), is not 0, and isn't empty, return it
            if (new_data["trainServices"][i].find("length") != new_data["trainServices"][i].end() &&
                !new_data["trainServices"][i]["length"].is_null()) {
                coaches = new_data["trainServices"][i]["length"].get<size_t>();
                if (coaches !=0) {
                    NewServiceInfo.coaches = std::to_string(coaches);
                }
            }
            
            // isCancelled;
            NewServiceInfo.isCancelled = new_data["trainServices"][i]["isCancelled"].get<bool>();
            
            // isDelayed
            if (NewServiceInfo.estimatedTime != "On time" && NewServiceInfo.estimatedTime != "Cancelled" ) {
                NewServiceInfo.isDelayed = true;
            } else {
                NewServiceInfo.isDelayed = false;
            }
            
            // cancelReason
            // If cancelReason exists, is not null, and isn't empty
            if (new_data["trainServices"][i].find("cancelReason") != new_data["trainServices"][i].end() &&
                !new_data["trainServices"][i]["cancelReason"].is_null() &&
                !new_data["trainServices"][i]["cancelReason"].get<std::string>().empty()) {
                NewServiceInfo.cancelReason = new_data["trainServices"][i]["cancelReason"].get<std::string>();
            } else {
                NewServiceInfo.cancelReason = ""; // No cancellation or no reason provided
            }
            
            // delayReason
            // Check if the service is delayed
            if (NewServiceInfo.estimatedTime != "On time" && NewServiceInfo.estimatedTime != "Cancelled" ) {
                // If delayReason exists, is not null, and isn't empty, return it
                if (new_data["trainServices"][i].find("delayReason") != new_data["trainServices"][i].end() &&
                    !new_data["trainServices"][i]["delayReason"].is_null() &&
                    !new_data["trainServices"][i]["delayReason"].get<std::string>().empty()) {
                    NewServiceInfo.delayReason = new_data["trainServices"][i]["delayReason"].get<std::string>();
                } else {
                    NewServiceInfo.delayReason = ""; // No reason given
                }
            } else {
                NewServiceInfo.delayReason = ""; // No delay
            }
            
            // adhocAlerts
            // If adhocAlerts exists, is not null, and isn't empty
            if (new_data["trainServices"][i].find("adhocAlerts") != new_data["trainServices"][i].end() &&
                !new_data["trainServices"][i]["adhocAlerts"].is_null() &&
                !new_data["trainServices"][i]["adhocAlerts"].get<std::string>().empty()) {
                NewServiceInfo.adhocAlerts = new_data["trainServices"][i]["adhocAlerts"].get<std::string>();
            } else {
                NewServiceInfo.adhocAlerts = "";
            }
            parsed_services.push_back(std::move(NewServiceInfo));
        }
        
        {
            std::lock_guard<std::mutex> lock(dataMutex);
            data = std::move(new_data);
            ServiceList = std::move(new_service_list);
            Services.swap(parsed_services);
            data_version.fetch_add(1, std::memory_order_release);
        }
        
        
        
    } catch (const json::parse_error& e) {
        throw std::runtime_error("Failed to parse JSON: " + std::string(e.what()));
    }
}

// set the flag to show estimated departure time in calling points
void TrainServiceParser::setShowCallingPointETD(bool show) {
    std::lock_guard<std::mutex> lock(dataMutex);
    
    showCallingPointETD = show;
}

// set the flag to find departures for a specific platform and store the platform
void TrainServiceParser::setSelectedPlatform(const std::string& platform) {
    std::lock_guard<std::mutex> lock(dataMutex);

    selectPlatform = true;
    selected_platform = platform;
}

// unset the flag to find departures for a specific platform
void TrainServiceParser::unsetSelectedPlatform() {
    std::lock_guard<std::mutex> lock(dataMutex);
   
    selectPlatform = false;
}

// Create a list of departures in the order of when they're going to happen
// This is by the later of scheduled and estimated time of departure
void TrainServiceParser::createOrderedDepartureList() {
    std::array<std::time_t, 10> time_list;
    std::string platform;
    
    try {
        // Get the number of train services (might be less than 10)
        
        // Get current time to use for date information
        std::time_t now = std::time(nullptr);
        std::tm *now_tm = std::localtime(&now);
        
        // Fill time_list with departure times
        for (size_t i = 0; i < number_of_services; i++) {
            std::tm departure_time = *now_tm; // Start with today's date
            std::string time_str;
            
            // Parse the scheduled time
            //time_str = data["trainServices"][i]["std"].get<std::string>();
            time_str = Services[i].scheduledTime;
            int hours, minutes;
            if (sscanf(time_str.c_str(), "%d:%d", &hours, &minutes) == 2) {
                departure_time.tm_hour = hours;
                departure_time.tm_min = minutes;
                departure_time.tm_sec = 0; // Reset seconds
            }
            
            std::time_t td_time = mktime(&departure_time);
            
            // Check if there's an estimated time and use it if it's not "On Time" or "Cancelled"
            std::string etd = Services[i].estimatedTime;
            
            if (!(etd == "On Time" || etd == "On time" || etd == "Cancelled" || etd == "null")) {
                // It's an actual time, parse it
                departure_time = *now_tm; // Reset with today's date
                if (sscanf(etd.c_str(), "%d:%d", &hours, &minutes) == 2) {
                    departure_time.tm_hour = hours;
                    departure_time.tm_min = minutes;
                    departure_time.tm_sec = 0; // Reset seconds
                    td_time = mktime(&departure_time);
                }
            }
            
            time_list[i] = td_time;
        }
        
        if (debug_mode) {
            DEBUG_PRINT("----- list of departure times -----");
            for (size_t i = 0; i < number_of_services; i++) {
                // Format time as HH:MM for easier reading
                std::tm* tm_time = localtime(&time_list[i]);
                char buffer[10];
                strftime(buffer, sizeof(buffer), "%H:%M", tm_time);
                
                platform = Services[i].platform;
                
                DEBUG_PRINT("Element " << i << " of time_list array. Platform " << platform << " Departure time " << buffer << " derived from" <<
                            //" std: " << data["trainServices"][i]["std"].get<std::string>() <<
                            //" etd: " << data["trainServices"][i]["etd"].get<std::string>());
                            " std: " << Services[i].scheduledTime <<
                            " etd: " << Services[i].estimatedTime);
            }
            if (number_of_services == 0) {
                DEBUG_PRINT("No train services available");
            }
        }
        
        // Initialize ETDOrderedList for actual number of services
        for (size_t i = 0; i < number_of_services; i++) {
            ETDOrderedList[i] = i;
        }
        
        // Fill the rest with invalid indices if num_services < 10
        for (size_t i = number_of_services; i < 10; i++) {
            ETDOrderedList[i] = 999; // Use 999 as an invalid index
        }
        
        // Sort only the valid indices based on time values (from earliest to latest)
        if (number_of_services > 0) {
            std::sort(ETDOrderedList.begin(), ETDOrderedList.begin() + number_of_services,
                      [&time_list](size_t a, size_t b) {
                return time_list[a] < time_list[b];
            });
        }
        
        // Debug output for only valid services
        if (debug_mode) {
            DEBUG_PRINT("----- Indices of departures in time order -----");
            for (size_t i = 0; i < number_of_services; i++) {
                size_t idx = ETDOrderedList[i];
                // Format time as HH:MM
                std::tm* tm_time = localtime(&time_list[idx]);
                char buffer[10];
                strftime(buffer, sizeof(buffer), "%H:%M", tm_time);
                
                DEBUG_PRINT("Position: " << i << " Index: " << idx << " Platform: " << Services[idx].platform <<
                           " Departure time: " << buffer << " derived from" <<
                            " std: " << Services[idx].scheduledTime <<
                            " etd: " << Services[idx].estimatedTime);
            }
            if (number_of_services == 0) {
                DEBUG_PRINT("No train services available");
            }
            DEBUG_PRINT(" ");
        }
    } catch (const json::exception& e) {
        throw std::runtime_error("Error creating ordered list of departure times: " + std::string(e.what()));
    }
}

// Get the indices of the first three departures
// If a platform of selected then limit departures to that platform
void TrainServiceParser::findServices() {
    size_t i;
    size_t index;
    
    try {
        // Check if train services exist in the data
        if (number_of_services == 0) {
            DEBUG_PRINT("No train services found in the data");
            return;
        }
        
        // Clear saved services
        ServiceList.fill(999);
        
        // Get ordered list of departures
        createOrderedDepartureList();
        
        if (selectPlatform) {
            // Find departures for the selected platform
            size_t serviceCount = 0;
            
            DEBUG_PRINT("Searching for services at platform " << selected_platform);
            
            // Iterate through all train services
            for (i = 0; i < number_of_services && serviceCount < ServiceList.size(); ++i) {
                index = ETDOrderedList[i];
                //const auto& service = data["trainServices"][index];
                
                // Check if the platform matches the selected platform
                if (Services[index].platform == selected_platform) {
                    // Found a service for the selected platform, add its index to the array
                    DEBUG_PRINT("Found service for platform " << selected_platform << " at position " << index);
                    ServiceList[serviceCount] = index;
                    
                    // Increment the count of services found
                    ++serviceCount;
                }
            }
        } else {
            // Find the first 3 departures
            for (i = 0; i < 3; i++) {
                if (i < number_of_services) {
                    ServiceList[i] = ETDOrderedList[i];
                }
            }
        }

        // Debug information about the found service
        if (debug_mode) {
            if (selectPlatform) {
                DEBUG_PRINT("Finding the first 3 departures for platform " << selected_platform);
            } else {
                DEBUG_PRINT("Finding the first 3 departures ");
            }
            for (i=0; i < 3; i++) {
                index = ServiceList[i];
                if ( ServiceList[i] == 999) {
                    DEBUG_PRINT("Index " << i << " - Service " << index <<". No service found");
                } else {
                    DEBUG_PRINT("Index " << i << " - Service " << index << " Platform " << Services[index].platform
                                << "    Destination: " << Services[index].destination
                                << " - Scheduled departure: " << Services[index].scheduledTime
                                << " - Estimated departure: " << Services[index].estimatedTime);
                }
            }
        }
    } catch (const json::exception& e) {
        DEBUG_PRINT("Error finding services: " << e.what());
    }
}

// Stop HTML code being included in the NRCC messages
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

// Lazy-load the calling points for the selected service into the data-structure
std::string TrainServiceParser::getCallingPoints(size_t serviceIndex) {
    std::lock_guard<std::mutex> lock(dataMutex);
    try {
        if (serviceIndex >= number_of_services) {
            throw std::out_of_range("Service index out of range");
        }
        const auto& service = data["trainServices"][serviceIndex];
        const auto& callingPoints = service["subsequentCallingPoints"][0]["callingPoint"];
        
        // If we're showing calling points, return the stored parsed calling points
        
        // Return calling points with the Time of Departure
        if (showCallingPointETD) {
            // Check if we have this stored - generate the content and store it if we don't
            if(Services[serviceIndex].callingPoints_with_ETD == "") {
                std::stringstream ss;
                for (size_t i = 0; i < callingPoints.size(); ++i) {
                    if (i > 0) ss << ", ";
                    ss << callingPoints[i]["locationName"].get<std::string>();
                    
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
                Services[serviceIndex].callingPoints_with_ETD = ss.str();
            }
            return Services[serviceIndex].callingPoints_with_ETD;
        } else { // Return calling points without the Time of Departure
            // Check if we have this stored - generate the content and store it if we don't
            if (Services[serviceIndex].callingPoints == "" ) {
                std::stringstream ss;
                for (size_t i = 0; i < callingPoints.size(); ++i) {
                    if (i > 0) ss << ", ";
                    ss << callingPoints[i]["locationName"].get<std::string>();
                }
                Services[serviceIndex].callingPoints = ss.str();
            }
            return Services[serviceIndex].callingPoints;
        }
    } catch (const json::exception& e) {
        DEBUG_PRINT(data);
        throw std::runtime_error("Error creating calling points: " + std::string(e.what()));
    }
}

// All the 'getter' functions

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

size_t TrainServiceParser::getNumberOfServices() {
        return number_of_services;
}

std::string TrainServiceParser::getSelectedPlatform() {
    std::lock_guard<std::mutex> lock(dataMutex);
    if (selectPlatform) {
        return selected_platform;
    } else {
        return "";
    }
}

// Return destination, std, etd, platform, coaches, operator, cancelled
std::tuple<std::string, std::string, std::string, std::string, std::string, std::string, bool> TrainServiceParser::getBasicServiceInfo(size_t serviceIndex) {
    std::lock_guard<std::mutex> lock(dataMutex);
    
    try {
        if (serviceIndex >= number_of_services) {
            throw std::out_of_range("Service index out of range");
        }
        return std::make_tuple(
                               Services[serviceIndex].destination,
                               Services[serviceIndex].scheduledTime,
                               Services[serviceIndex].estimatedTime,
                               Services[serviceIndex].platform,
                               Services[serviceIndex].coaches,
                               Services[serviceIndex].operator_name,
                               Services[serviceIndex].isCancelled
                               );
    }
    catch (const json::exception& e) {
        throw std::runtime_error("Error getting Departure Time: " + std::string(e.what()));
    }
}

std::string TrainServiceParser::getScheduledDepartureTime(size_t serviceIndex) {
    std::lock_guard<std::mutex> lock(dataMutex);
    
    try {
        if (serviceIndex >= number_of_services) {
            throw std::out_of_range("Service index out of range");
        }
        return Services[serviceIndex].scheduledTime;
    } catch (const json::exception& e) {
        throw std::runtime_error("Error getting Departure Time: " + std::string(e.what()));
    }
}


std::string TrainServiceParser::getEstimatedDepartureTime(size_t serviceIndex) {
    std::lock_guard<std::mutex> lock(dataMutex);
    
    try {
        if (serviceIndex >= number_of_services) {
            throw std::out_of_range("Service index out of range");
        }
        return Services[serviceIndex].estimatedTime;
    } catch (const json::exception& e) {
        throw std::runtime_error("Error getting EstimatedDeparture Time: " + std::string(e.what()));
    }
}

std::string TrainServiceParser::getPlatform(size_t serviceIndex) {
    std::lock_guard<std::mutex> lock(dataMutex);
    
    try {
        if (serviceIndex >= number_of_services) {
            throw std::out_of_range("Service index out of range");
        }
        return Services[serviceIndex].platform;
    } catch (const json::exception& e) {
        throw std::runtime_error("Error getting Platform: " + std::string(e.what()));
    }
}

std::string TrainServiceParser::getDestination(size_t serviceIndex) {
    std::lock_guard<std::mutex> lock(dataMutex);
    
    try {
        if (serviceIndex >= number_of_services) {
            throw std::out_of_range("Service index out of range");
        }
        return Services[serviceIndex].destination;
    } catch (const json::exception& e) {
        throw std::runtime_error("Error getting Destination: " + std::string(e.what()));
    }
}

bool TrainServiceParser::isCancelled(size_t serviceIndex) {
    std::lock_guard<std::mutex> lock(dataMutex);
    
    try {
        if (serviceIndex >= number_of_services) {
            throw std::out_of_range("Service index out of range");
        }
        return Services[serviceIndex].isCancelled;
    } catch (const json::exception& e) {
        throw std::runtime_error("Error getting Cancellation status: " + std::string(e.what()));
    }
}

std::string TrainServiceParser::getCancelReason(size_t serviceIndex) {
    std::lock_guard<std::mutex> lock(dataMutex);
    
    try {
        if (serviceIndex >= number_of_services) {
            throw std::out_of_range("Service index out of range");
        }
        return Services[serviceIndex].cancelReason;
    } catch (const json::exception& e) {
        throw std::runtime_error("Error getting Cancellation reason: " + std::string(e.what()));
    }
}

bool TrainServiceParser::isDelayed(size_t serviceIndex) {
    std::lock_guard<std::mutex> lock(dataMutex);
    
    try {
        if (serviceIndex >= number_of_services) {
            throw std::out_of_range("Service index out of range");
        }
        return Services[serviceIndex].isDelayed;
    } catch (const json::exception& e) {
        throw std::runtime_error("Error getting delay status: " + std::string(e.what()));
    }
}

std::string TrainServiceParser::getDelayReason(size_t serviceIndex) {
    std::lock_guard<std::mutex> lock(dataMutex);
    
    try {
        if (serviceIndex >= number_of_services) {
            throw std::out_of_range("Service index out of range");
        }
        return Services[serviceIndex].delayReason;
    } catch (const json::exception& e) {
        throw std::runtime_error("Error getting delay reason: " + std::string(e.what()));
    }
}

std::string TrainServiceParser::getadhocAlerts(size_t serviceIndex){
    std::lock_guard<std::mutex> lock(dataMutex);
    
    try {
        if (serviceIndex >= number_of_services) {
            throw std::out_of_range("Service index out of range");
        }
        return Services[serviceIndex].adhocAlerts;
    } catch (const json::exception& e) {
        throw std::runtime_error("Error getting adhoc alerts: " + std::string(e.what()));
    }
}

std::string TrainServiceParser::getCoaches(size_t serviceIndex, bool addText) {
    std::lock_guard<std::mutex> lock(dataMutex);
    
    try {
        if (serviceIndex >= number_of_services) {
            throw std::out_of_range("Service index out of range");
        }
        std::string text = Services[serviceIndex].coaches;
        if (!text.empty()) {
            if (addText) {
                text = " formed of " + text + " coaches";
            }
        }
        return text;
    } catch (const json::exception& e) {
        throw std::runtime_error("Error getting number of coaches: " + std::string(e.what()));
    }
}

std::string TrainServiceParser::getOperator(size_t serviceIndex) {
    std::lock_guard<std::mutex> lock(dataMutex);
    
    try {
        if (serviceIndex >= number_of_services) {
            throw std::out_of_range("Service index out of range");
        }
        return Services[serviceIndex].operator_name;
    } catch (const json::exception& e) {
        throw std::runtime_error("Error getting operator name: " + std::string(e.what()));
    }
}

std::string TrainServiceParser::getNrccMessages() {
    std::lock_guard<std::mutex> lock(dataMutex);
    
    return NRCC_message;
}

std::string TrainServiceParser::getLocationName(){
    std::lock_guard<std::mutex> lock(dataMutex);
    
    return location_name;
}



