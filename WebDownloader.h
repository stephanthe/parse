#ifndef WEBDOWNLOADER_H
#define WEBDOWNLOADER_H

#include <curl/curl.h>
#include <string>

class WebDownloader {
public:
    WebDownloader();
    ~WebDownloader();

    std::string downloadPage(const std::string &url, const std::string &cookieFile);

private:
    CURL *curl;
};

#endif // WEBDOWNLOADER_H