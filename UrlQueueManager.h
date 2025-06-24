#ifndef URLQUEUEMANAGER_H
#define URLQUEUEMANAGER_H

#include <queue>
#include <unordered_set>
#include <string>

class UrlQueueManager {
public:
    void addUrl(const std::string &url);
    bool hasUrls() const;
    std::string getNextUrl();
    bool isProcessed(const std::string &url) const;

private:
    std::queue<std::string> urlQueue;
    std::unordered_set<std::string> processedUrls;
};

#endif // URLQUEUEMANAGER_H
