// Parser tester - Jon Morris Smith - March 2025
//
// The parser is the most fragile part of the traindisplay.
//
// A suite of tests to check parsing works and for bug reporting!
//
// -debug       Switches on debug info in the parser code
// -platform    For use if you need to test a specific platform
// -data        Data to process - the output from an API call. Can be formatted or formatted JSON
//              The -d flag on traindisplay dumps API output to /tmp/traindisplay_payload.json
// -clean       If set to 'y' then all whitespace is removed
// -f           Used to specify a traindisplay configuration file (not currently coded)
//

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include "train_service_parser.h"

// Global debug flag
bool debug_mode = false;


//  Check to see if a file is text or binary

bool isBinaryFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        exit(1);
    }
    
    // Read first 4KB or less if file is smaller
    const int chunkSize = 4096;
    char buffer[chunkSize];
    file.read(buffer, chunkSize);
    int bytesRead = file.gcount();
    
    if (bytesRead == 0) {
        return false; // Empty file, consider it text
    }
    
    // Count binary characters (NULL bytes and certain control characters)
    int binaryCount = 0;
    for (int i = 0; i < bytesRead; i++) {
        unsigned char c = buffer[i];
        if (c == 0 || (c < 9 && c != 7) || (c > 13 && c < 32)) {
            binaryCount++;
        }
    }
    
    // If more than 10% of the first chunk contains binary characters, consider it binary
    return (static_cast<double>(binaryCount) / bytesRead) > 0.1;
}

// Read test from the file into a string

std::string readTextFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        exit(1);
    }
    
    std::string content;
    char c;
    while (file.get(c)) {
        // Skip spaces and tabs as per requirements
        // if (c != ' ' && c != '\t') {
            content += c;
        //}
    }
    
    return content;
}

// Remove whitespce

void removeAllWhitespace(std::string& str) {
    str.erase(std::remove_if(str.begin(), str.end(),
                            [](unsigned char c) { return std::isspace(c); }),
              str.end());
}

int main(int argc, char* argv[]) {

    // Declare variables for parameters
    std::string debug;
    std::string data_file;
    std::string platform;
    std::string clean_data;
    std::string config_file;
    TrainServiceParser parser;
    size_t num_services;
    size_t service;
    int departureNumber;
    size_t i;
    bool yesno;
     
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string param = argv[i];
        
        if (param == "-debug" && i + 1 < argc) {
            debug = argv[++i];
        } else if (param == "-data" && i + 1 < argc) {
            data_file = argv[++i];
        } else if (param == "-platform" && i + 1 < argc) {
            platform = argv[++i];
        } else if (param == "-clean" && i + 1 < argc) {
            clean_data = argv[++i];
        } else if (param == "-f" && i + 1 < argc) {
            config_file = argv[++i];
        } else {
            std::cerr << "Error: Invalid parameter or missing value: " << param << std::endl;
            std::cerr << "Usage: " << argv[0] << " -data <string> [-platform <string>] [-clean <string>] [-f <string>] [-debug <string>]" << std::endl;
            std::cerr << "-data json data file" << std::endl;
            std::cerr << "-platform select a platform" << std::endl;
            std::cerr << "-clean y  remove whitespace" << std::endl;
            std::cerr << "-f config file (not currently in use)" << std::endl;
            std::cerr << "-debug y  switch on debug info in the parser code" << std::endl;
            return 1;
        }
    }
    
    if (data_file.empty()) {
        std::cerr << "Error: Mandatory parameter -data is missing." << std::endl;
        return 1;
    }
    
    // Check if the file is binary
    if (isBinaryFile(data_file)) {
        std::cerr << "Error: File " << data_file << " appears to be a binary file." << std::endl;
        return 1;
    }
    
    // Read the file content, removing spaces and tabs during reading
    std::string data = readTextFile(data_file);
    
    // If clean_data is 'y', remove all whitespace
    if (clean_data == "y") {
        removeAllWhitespace(data);
    }

    if (debug == "y") {
        debug_mode = true;
    }

    // Display parameters for use when output sent as debug output
    std::cout << "--- Parser Test Parameters ---" << std::endl;
     std::cout << "Data file: " << data_file << std::endl;
     if (!platform.empty()) {
         std::cout << "Platform: " << platform << std::endl;
     }
     if (!clean_data.empty()) {
         std::cout << "Clean data: " << clean_data << std::endl;
     }
     if (!config_file.empty()) {
         std::cout << "Config file: " << config_file << std::endl;
     }
     if (!debug.empty()) {
         std::cout << "Debug:: " << debug << " debug_mode: "<< debug_mode << std::endl;
     } else {
         std::cout << "debug_mode: "<< debug_mode << std::endl;
     }

     std::cout << "------------------------------" << std::endl;

     parser.updateData(data);

    // Functions to test
    //
    // void setShowCallingPointETD(bool show);
    // void setSelectedPlatform(const std::string& platform);
    // void unsetSelectedPlatform();
    // void findServices();
    // bool isCancelled(size_t serviceIndex);
    // bool isDelayed(size_t serviceIndex);
    // size_t getNumberOfServices();
    // size_t getFirstDeparture();
    // size_t getSecondDeparture();
    // size_t getThirdDeparture();
    // std::string getScheduledDepartureTime(size_t serviceIndex);
    // std::string getEstimatedDepartureTime(size_t serviceIndex);
    // std::string getPlatform(size_t serviceIndex);
    // std::string getDestination(size_t serviceIndex);
    // std::string getCallingPoints(size_t serviceIndex);
    // std::string getCoaches(size_t serviceIndex, bool addText);
    // std::string getOperator(size_t serviceIndex);
    // std::string getNrccMessages();
    // std::string getDelayReason(size_t serviceIndex);
    // std::string getCancelReason(size_t serviceIndex);
    // std::string getadhocAlerts(size_t serviceIndex);

    num_services = parser.getNumberOfServices();
    std::cout << "Number of Services: " << num_services << std::endl;;

    for (i=0; i< num_services; i++) {
       std::cout << "==========================================================" << std::endl;
       std::cout << "serviceIndex: " << i << std::endl;


       std::cout << "getScheduledDepartureTime: " << parser.getScheduledDepartureTime(i) << std::endl;
       std::cout << "getEstimatedDepartureTime: " << parser.getEstimatedDepartureTime(i) << std::endl;
       std::cout << "getPlatform: " << parser.getPlatform(i) << std::endl;
       std::cout << "getDestination: " << parser.getDestination(i) << std::endl;
       parser.setShowCallingPointETD(true);
       std::cout << "getCallingPoints - ETD true: " << parser.getCallingPoints(i) << std::endl;
       parser.setShowCallingPointETD(false);
       std::cout << "getCallingPoints - ETD false: " << parser.getCallingPoints(i) << std::endl;
       std::cout << "isDelayed: " << parser.isDelayed(i) << std::endl;
       std::cout << "getDelayReason: " << parser.getDelayReason(i) << std::endl;
       std::cout << "isCancelled: " << parser.isCancelled(i) << std::endl;
       std::cout << "getCancelReason: " << parser.getCancelReason(i) << std::endl;
       std::cout << "getOperator: " << parser.getOperator(i) << std::endl;
       std::cout << "getCoaches (with message): " << parser.getCoaches(i, true) << std::endl;
       std::cout << "getCoaches (without message): " << parser.getCoaches(i, false) << std::endl;
       std::cout << "getadhocAlerts: " << parser.getadhocAlerts(i) << std::endl;
       std::cout << "\n\n\n" << std::endl;
    }

    std::cout << "==========================================================" << std::endl;
    std::cout << "=============== Network Rail messages ====================" << std::endl;
    std::cout << "NRCC messages: " << parser.getNrccMessages() << std::endl;
    std::cout << "==========================================================" << std::endl;
    std::cout << "=========== Getting the first 3 departures ===============" << std::endl;

    parser.findServices();

    std::cout << "First three departures" << std::endl;
    service = parser.getFirstDeparture();
    std::cout << "First: Platform " << parser.getPlatform(service) << " at " << parser.getScheduledDepartureTime(service) << " to " << parser.getDestination(service) << std::endl;
    service = parser.getSecondDeparture();
    std::cout << "Second: Platform " << parser.getPlatform(service) << " at " << parser.getScheduledDepartureTime(service) << " to " << parser.getDestination(service) << std::endl;
    service = parser.getThirdDeparture();
    std::cout << "Third: Platform " << parser.getPlatform(service) << " at " << parser.getScheduledDepartureTime(service) << " to " << parser.getDestination(service) << std::endl;
    

    if (!platform.empty()) {
        std::cout << "==========================================================" << std::endl;
        std::cout << "=========== Testing platform parsing =====================" << std::endl;
        std::cout << "======= First three departures from platform " << platform << " ===========" <<std::endl;
        parser.setSelectedPlatform(platform);
        parser.findServices();
        std::cout << "First three departures" << std::endl;
        service = parser.getFirstDeparture();
        std::cout << "First: Platform " << parser.getPlatform(service) << " at " << parser.getScheduledDepartureTime(service) << " to " << parser.getDestination(service) << std::endl;
        service = parser.getSecondDeparture();
        std::cout << "Second: Platform " << parser.getPlatform(service) << " at " << parser.getScheduledDepartureTime(service) << " to " << parser.getDestination(service) << std::endl;
        service = parser.getThirdDeparture();
        std::cout << "Third: Platform " << parser.getPlatform(service) << " at " << parser.getScheduledDepartureTime(service) << " to " << parser.getDestination(service) << std::endl;
        std::cout << "==========================================================" << std::endl;
        std::cout << "============= Unsetting Platform Selection ===============" << std::endl;
        std::cout << "=============== First three departures ===================" << std::endl;
        parser.unsetSelectedPlatform();
        parser.findServices();
        std::cout << "First three departures" << std::endl;
        service = parser.getFirstDeparture();
        std::cout << "First: Platform " << parser.getPlatform(service) << " at " << parser.getScheduledDepartureTime(service) << " to " << parser.getDestination(service) << std::endl;
        service = parser.getSecondDeparture();
        std::cout << "Second: Platform " << parser.getPlatform(service) << " at " << parser.getScheduledDepartureTime(service) << " to " << parser.getDestination(service) << std::endl;
        service = parser.getThirdDeparture();
        std::cout << "Third: Platform " << parser.getPlatform(service) << " at " << parser.getScheduledDepartureTime(service) << " to " << parser.getDestination(service) << std::endl;
       }
    
    std::cout << "==========================================================" << std::endl;
    std::cout << "====================== Location ==========================" << std::endl;
    std::cout << parser.getLocationName() << std::endl << std::endl;
    
     return 0;
 }

