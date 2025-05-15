#include <iostream>
#include <string>
#include <curl/curl.h>
#include "json.hpp"

using json = nlohmann::json;


size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}


std::string sendMessageToChatbot(const std::string& userMessage, const std::string& apiKey) {
    std::string responseStr;
    CURL* curl = curl_easy_init();

    if (curl) {
        std::string url = "https://api.openai.com/v1/chat/completions";
        json payload = {
            {"model", "gpt-3.5-turbo"},
            {"messages", {
                {{"role", "user"}, {"content", userMessage}}
            }}
        };

        std::string payloadStr = payload.dump();

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + apiKey).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadStr.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseStr);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Request failed: " << curl_easy_strerror(res) << std::endl;
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    try {
        json responseJson = json::parse(responseStr);
        return responseJson["choices"][0]["message"]["content"];
    } catch (...) {
        return "Error parsing response.";
    }
}

int main() {

    std::string apiKey = "sk-proj";
    std::string userMessage = "Is C++ easy?";
    std::string chatBotName = "Nameless ChatBot";
    std::string userName = "Nameless User";

    std::cout << "Enter your message: ";
    

    std::string response = sendMessageToChatbot(userMessage, apiKey);
    std::cout << "Chatbot Response: " << response << std::endl;

    return 0;
}
