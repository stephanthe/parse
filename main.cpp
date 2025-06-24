#include "HtmlParser.h"
#include "UrlQueueManager.h"
#include "WebDownloader.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <unordered_set>

std::string getFilenameFromUrl(const std::string &url) {

  size_t protocolEnd = url.find("://");
  if (protocolEnd == std::string::npos) {
    throw std::invalid_argument(
        "URL does not contain a valid protocol (http/https)");
  }
  std::string transformedUrl = url.substr(protocolEnd + 3);
  std::replace(transformedUrl.begin(), transformedUrl.end(), '/', '_');

  return transformedUrl;
}

void saveHtmlToFile(const std::string &htmlContent,
                    const std::string &filename) {
  std::ofstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file for writing");
  }
  file << htmlContent;
  file.close();
}

bool isTargetUrl(const std::string &url) {
  static const std::unordered_set<std::string> targetUrls = {
      "https://otus.ru/events/near/", "https://otus.ru/events/subscribed/",
      "https://otus.ru/events/archive/"};
  return targetUrls.find(url) != targetUrls.end();
}

int main() {
  std::string baseUrl = "https://otus.ru";
  std::string cookieFile = "cookies.txt";
  WebDownloader downloader;
  HtmlParser parser;
  UrlQueueManager queueManager;

  queueManager.addUrl("https://otus.ru/events/near/");
  queueManager.addUrl("https://otus.ru/events/subscribed/");
  queueManager.addUrl("https://otus.ru/events/archive/");

  try {
    while (queueManager.hasUrls()) {
      std::string currentUrl = queueManager.getNextUrl();
      if (!isTargetUrl(currentUrl) || currentUrl == "") {
        continue;
      }

      std::string htmlContent = downloader.downloadPage(currentUrl, cookieFile);
      bool Subscribed = htmlContent.find("вы записаны") != std::string::npos;

      std::string message;
      if (currentUrl == "https://otus.ru/events/near/") {
        message = "Ближайшие мероприятия:";
      }
      if (currentUrl == "https://otus.ru/events/subscribed/") {
        if (!Subscribed) {
            continue;
        }
        message = "Мероприятия, на которые вы подписаны:";
      }
      if (currentUrl == "https://otus.ru/events/archive/") {
        message = "Архив мероприятий:";
      }
      
      std::cout << "\n\n" << message << "\n\n";
/*
      // Сохраняем HTML-контент в файл
      std::string filename = getFilenameFromUrl(currentUrl);
      saveHtmlToFile(htmlContent, filename);
*/
      auto titles = parser.extractDetails(htmlContent, baseUrl);

      for (size_t i = 0; i < titles.size(); ++i) {
        std::cout << "Title: " << titles[i] << "\n";
      }
      /*
                  // Добавляем новые ссылки в очередь
                  for (const std::string &link :
         parser.extractLinks(htmlContent, baseUrl)) { if (isTargetUrl(link) &&
         !queueManager.isProcessed(link)) { queueManager.addUrl(link);
                      }
                  }
      */
    }

    // Здесь можно добавить логику обработки содержимого страниц, если
    // необходимо
  } catch (const std::exception &e) {
    std::cerr << "Error processing URL: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
