// Train Display API Client
// Jon Morris Smith - Feb 2025
// Version 1.0
// Instructions, fixes and issues at https://github.com/jonmorrissmith/RGB_Matrix_Train_Departure_Board
//
//
#include "api_client.h"

// Callback function to handle API response
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

TrainAPIClient::TrainAPIClient(const std::string& api_url, const std::string& api_key, bool use_rdm) {
    base_url = api_url;
    base_url_key = api_key;
    rail_data_marketplace = use_rdm;
}

std::string TrainAPIClient::fetchDepartures(const std::string& from, const std::string& to) const {
    CURL* curl = curl_easy_init();
    struct curl_slist *headers = NULL;
    std::string readBuffer;
    std::string api_header;
    std::string url;
    FILE* curl_log_file;
    FILE* curl_json_file;
    
    if(!curl) {
        throw std::runtime_error("Failed to initialize CURL");
    }

    if(rail_data_marketplace) {   

    DEBUG_PRINT("Creating a Rail Data Marketplace URL"); 
    // Craft a rail data marketplace URL 
    // Format: https://api1.raildata.org.uk/1010-live-departure-board-dep1_2/LDBWS/api/20220120/GetDepBoardWithDetails/<CRS>?numRows=10 
    // or
    // Format: https://api1.raildata.org.uk/1010-live-departure-board-dep1_2/LDBWS/api/20220120/GetDepBoardWithDetails/<CRS>?filterCrs=<CRS>&filterType=to
    //
    // The API URL configuration can be ignored... so we'll ignore it.

       if(to.empty()) {
           url = "https://api1.raildata.org.uk/1010-live-departure-board-dep1_2/LDBWS/api/20220120/GetDepBoardWithDetails/" + from + "?numRows=10";
       } else {
           url = "https://api1.raildata.org.uk/1010-live-departure-board-dep1_2/LDBWS/api/20220120/GetDepBoardWithDetails/" + from + "?numRows=10&filterCrs=" + to + "&filterType=to";
       }
       DEBUG_PRINT("Making Rail Data Marketplace API call to: " << url); 
    } else {

    DEBUG_PRINT("Creating a Network Rail URL");
    // Craft a NRE/Huxley2 URL
    // Format: https://<URL>/departures/<CRS>/10?expand=true
    // or
    // Format: https://<URL>/departures/<CRS>/to/<CRS>/10?expand=true

       if(to.empty()) {
           url = base_url + "/departures/" + from + "/10?expand=true";
       } else {
           url = base_url + "/departures/" + from + "/to/" + to + "/10?expand=true";
       }
       DEBUG_PRINT("Making NRE/Huxley2 API call to: " << url); 
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    if(debug_mode) {
       // Enable verbose curl logging
       DEBUG_PRINT("Curl logs dumped into /tmp/traindisplay_curl_debug.log as they're quite verbose!");
       DEBUG_PRINT("JSON from API call dumped into /tmp/traindisplay_payload.json");
       curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

       curl_log_file = fopen("/tmp/traindisplay_curl_debug.log", "w");
       if(curl_log_file) {
           curl_easy_setopt(curl, CURLOPT_STDERR, curl_log_file);
       } else {
           DEBUG_PRINT("Warning: Could not open /tmp/curl_debug.log for writing");
       }
    }

    if(!base_url_key.empty()) {
        api_header = "x-apikey:" + base_url_key;
        headers = curl_slist_append(headers, api_header.c_str());

        // *WARNING* uncommenting the next line means your API key is included in log/debug info.
        //DEBUG_PRINT("API header: " << api_header.c_str());

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }

    CURLcode res = curl_easy_perform(curl);
   
    curl_slist_free_all(headers);

    if(debug_mode) {
       // Write the JSON payload to a file and close logfiles.

       curl_json_file = fopen("/tmp/traindisplay_payload.json", "w");
       if(curl_json_file) {
           if(fputs(readBuffer.c_str(), curl_json_file) == EOF) {
               DEBUG_PRINT("Error writing API response to log file");
           }
        }

        fflush(curl_log_file);
        fflush(curl_json_file);

        fclose(curl_log_file);
        fclose(curl_json_file);
    }
 
    if(res != CURLE_OK) {
        curl_easy_cleanup(curl);
        throw std::runtime_error("Failed to make API call: " + std::string(curl_easy_strerror(res)));
    }

    curl_easy_cleanup(curl);
    
    DEBUG_PRINT("API Response length: " << readBuffer.length());
    
    return readBuffer;
}

