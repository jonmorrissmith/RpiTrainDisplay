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
#ifndef API_CLIENT_H
#define API_CLIENT_H

#include <curl/curl.h>
#include <string>
#include <stdexcept>
#include <iostream>

// Forward declaration for the debug printing macro
extern bool debug_mode;
#define DEBUG_PRINT(x) if(debug_mode) { std::cerr << x << std::endl; }

// Callback function to handle API response
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

class TrainAPIClient {
private:
    std::string base_url;
    
public:
    TrainAPIClient(const std::string& api_url) : base_url(api_url) {}
    
    std::string fetchDepartures(const std::string& from, const std::string& to) const {
        CURL* curl = curl_easy_init();
        std::string readBuffer;
        
        if(!curl) {
            throw std::runtime_error("Failed to initialize CURL");
        }
        
        std::string url;
        if(to.empty()) {
            url = base_url + "/departures/" + from + "/3?expand=true";
        } else {
            url = base_url + "/departures/" + from + "/to/" + to + "/3?expand=true";
        }

        DEBUG_PRINT("Making API call to: " << url);

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        CURLcode res = curl_easy_perform(curl);
        
        if(res != CURLE_OK) {
            curl_easy_cleanup(curl);
            throw std::runtime_error("Failed to make API call: " + std::string(curl_easy_strerror(res)));
        }

        curl_easy_cleanup(curl);
        
        DEBUG_PRINT("API Response length: " << readBuffer.length());
        
        return readBuffer;
    }
};

#endif // API_CLIENT_H
