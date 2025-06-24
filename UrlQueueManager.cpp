#include <stdexcept>
#include "UrlQueueManager.h"

void UrlQueueManager::addUrl(const std::string &url) {
    if (processedUrls.find(url) == processedUrls.end()) {
        urlQueue.push(url);
    }
}

bool UrlQueueManager::hasUrls() const {
    return !urlQueue.empty();
}

std::string UrlQueueManager::getNextUrl() {
    while (!urlQueue.empty()) {
        std::string url = urlQueue.front();
        urlQueue.pop();
        if (processedUrls.find(url) != processedUrls.end()) {
            continue;
        }
        processedUrls.insert(url);
        return url;
    }
    return "";
}

bool UrlQueueManager::isProcessed(const std::string &url) const {
    return processedUrls.find(url) != processedUrls.end();
}
