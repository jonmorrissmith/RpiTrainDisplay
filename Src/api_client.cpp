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
#include "api_client.h"

// Callback function to handle API response
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

TrainAPIClient::TrainAPIClient(const std::string& api_url) : base_url(api_url) {}

std::string TrainAPIClient::fetchDepartures(const std::string& from, const std::string& to) const {
    CURL* curl = curl_easy_init();
    std::string readBuffer;
    
    if(!curl) {
        throw std::runtime_error("Failed to initialize CURL");
    }
    
    std::string url;
    if(to.empty()) {
        url = base_url + "/departures/" + from + "/10?expand=true";
    } else {
        url = base_url + "/departures/" + from + "/to/" + to + "/10?expand=true";
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
    // Uncomment the next line for verbose output 
    // DEBUG_PRINT("API Response: " << readBuffer);
    
    return readBuffer;
}

