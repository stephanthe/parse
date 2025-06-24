#include "WebDownloader.h"
#include <curl/curl.h>
#include <stdexcept>

// Определение конструктора
WebDownloader::WebDownloader() {
    curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize libcurl");
    }
}

// Определение деструктора
WebDownloader::~WebDownloader() {
    if (curl) {
        curl_easy_cleanup(curl);
    }
}

size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp) {
    size_t totalSize = size * nmemb;
    userp->append((char *)contents, totalSize);
    return totalSize;
}

std::string WebDownloader::downloadPage(const std::string &url, const std::string &cookieFile) {
    std::string response;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cookieFile.c_str());
    curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookieFile.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        throw std::runtime_error("Failed to download page: " + std::string(curl_easy_strerror(res)));
    }

    return response;
}
