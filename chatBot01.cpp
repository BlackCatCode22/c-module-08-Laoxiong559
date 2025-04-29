#include <iostream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include <json/json.h>
#include <chrono>
#include <thread>

// Function to handle API call errors recursively
std::string recursiveApiCall(const std::string& url, const std::string& query, int retries = 3) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if(curl) {
        std::string api_url = url + "?query=" + query;
        curl_easy_setopt(curl, CURLOPT_URL, api_url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](void *data, size_t size, size_t nmemb, void *userp) {
            std::string& buffer = *reinterpret_cast<std::string*>(userp);
            buffer.assign(static_cast<const char *>(data), size * nmemb);
            return size * nmemb;
        });
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            if(retries > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Wait for 500ms before retrying
                return recursiveApiCall(url, query, retries - 1);
            } else {
                return "Error: API call failed";
            }
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    return readBuffer;
}

int main() {
    std::string api_url = "https://api.openai.com/v1/completions";
    std::string api_key = "YOUR_API_KEY"; // Replace with your OpenAI API key
    std::string model = "text-davinci-002";

    int interactionCount = 0;
    std::vector<std::string> conversationHistory;

    while(true) {
        std::string userQuery;
        std::cout << "User: ";
        std::getline(std::cin, userQuery);

        if(userQuery.empty()) {
            std::cout << "Error: Please enter a valid query." << std::endl;
            continue;
        }

        if(userQuery.length() > 2048) {
            std::cout << "Error: Query exceeds the maximum character limit." << std::endl;
            continue;
        }

        std::string apiResponse = recursiveApiCall(api_url, userQuery);

        if(apiResponse.find("Error") != std::string::npos) {
            std::cout << apiResponse << std::endl;
            continue;
        }

        Json::Value jsonData;
        Json::Reader jsonReader;

        if(jsonReader.parse(apiResponse, jsonData)) {
            std::string chatbotResponse = jsonData["choices"][0]["text"].asString();
            conversationHistory.push_back("User: " + userQuery);
            conversationHistory.push_back("Chatbot: " + chatbotResponse);

            std::cout << "Chatbot: " << chatbotResponse << std::endl;

            interactionCount++;
            std::cout << "Interaction Count: " << interactionCount << std::endl;

            std::cout << "Conversation History:" << std::endl;
            for(const auto& message : conversationHistory) {
                std::cout << message << std::endl;
            }
        } else {
            std::cout << "Error: Failed to parse JSON response." << std::endl;
        }

        // Measure API response time
        auto start_time = std::chrono::high_resolution_clock::now();
        auto end_time = std::chrono::high_resolution_clock::now();
        auto response_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
        std::cout << "API Response Time: " << response_time << "ms" << std::endl;

        // Calculate average response time
        double average_response_time = 0.0;
        if(interactionCount > 0) {
            average_response_time = response_time / interactionCount;
        }
        std::cout << "Average Response Time: " << average_response_time << "ms" << std::endl;
    }

    return 0;
}
