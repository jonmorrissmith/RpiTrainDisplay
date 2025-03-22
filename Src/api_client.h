// Train Display API Client
// Jon Morris Smith - Feb 2025
// Version 1.0
// Instructions, fixes and issues at https://github.com/jonmorrissmith/RGB_Matrix_Train_Departure_Board
//
//
#ifndef API_CLIENT_H
#define API_CLIENT_H

#include <curl/curl.h>
#include <string>
#include <stdexcept>
#include <iostream>

// Forward declaration for the debug printing macro
extern bool debug_mode;
#define DEBUG_PRINT(x) if(debug_mode) { std::cerr << x << std::endl; }

// Forward declaration of the callback function
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

class TrainAPIClient {
private:
    std::string base_url;
    std::string base_url_key;
    bool rail_data_marketplace;
    
public:
    TrainAPIClient(const std::string& api_url, const std::string& api_key, bool use_rdm);
    
    std::string fetchDepartures(const std::string& from, const std::string& to) const;
};

#endif // API_CLIENT_H

