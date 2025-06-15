#include <curl/curl.h>
#include <fstream>
#include <iostream>
#include <libxml/HTMLparser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <queue>
#include <regex>
#include <string>
#include <unordered_set>

size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp)
{
    size_t totalSize = size * nmemb;
    userp->append((char *)contents, totalSize);
    return totalSize;
}


std::string downloadPage(const std::string &url, const std::string &cookieFile)
{
    CURL *curl = curl_easy_init();
    if (!curl)
    {
        throw std::runtime_error("Failed to initialize libcurl");
    }
    std::string response;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cookieFile.c_str());
    curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookieFile.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        throw std::runtime_error("Failed to download page: " + std::string(curl_easy_strerror(res)));
    }
    curl_easy_cleanup(curl);
    return response;
}

void findLinks(xmlNodePtr node, const std::string &baseUrl, std::vector<std::string> &links)
{
    if (!node)
        return;
    if (xmlStrcmp(node->name, (const xmlChar *)"a") == 0)
    {
        xmlAttr *attr = node->properties;
        while (attr)
        {
            if (xmlStrcmp(attr->name, (const xmlChar *)"href") == 0)
            {
                std::string href = (char *)attr->children->content;
                if (!href.empty())
                {
                    if (href.find("http") != 0 && href[0] == '/')
                    {
                        std::string absoluteUrl = baseUrl + href;
                        links.push_back(absoluteUrl);
                    }
                    else if (href.find(baseUrl) == 0)
                    {
                        links.push_back(href);
                    }
                }
            }
            attr = attr->next;
        }
    }
    xmlNodePtr child = node->children;
    while (child)
    {
        findLinks(child, baseUrl, links);
        child = child->next;
    }
}

std::vector<std::string> extractLinks(const std::string &html, const std::string &baseUrl)
{
    std::vector<std::string> links;
    htmlDocPtr doc = htmlReadMemory(html.c_str(), html.size(), nullptr, "UTF-8", HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
    if (!doc)
    {
        throw std::runtime_error("Failed to parse HTML");
    }
    xmlNodePtr root = xmlDocGetRootElement(doc);
    findLinks(root, baseUrl, links);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return links;
}

bool isMatchingUrl(const std::string &url, const std::string &baseUrl)
{
    std::regex pattern(baseUrl + R"(/learning/\d{1,7}/)");
    return std::regex_match(url, pattern);
}

bool containsTrainingPeriodText(const std::string &htmlContent)
{
    // Ищем текст "Период обучения" в HTML-коде
    return htmlContent.find("Период обучения") != std::string::npos;
}

std::string &ltrim(std::string &str)
{
    str.erase(str.begin(),
              std::find_if(str.begin(), str.end(), [](unsigned char ch)
                           { return ch != ' ' && ch != '\n'; }));
    return str;
}
std::string &rtrim(std::string &str)
{
    str.erase(
        std::find_if(str.rbegin(), str.rend(),
                     [](unsigned char ch)
                     { return ch != ' ' && ch != '\n'; })
            .base(),
        str.end());
    return str;
}

std::string &trim(std::string &str) { return ltrim(rtrim(str)); }
int main(int argc, char *argv[])
{

    std::string baseUrl = "https://otus.ru";
    std::string cookieFile = "cookies.txt";
    try
    {
        std::queue<std::string> linkQueue;
        linkQueue.push(baseUrl + "/learning/");
        std::unordered_set<std::string> processedLinks;
        std::vector<std::string> learning_links;

        while (!linkQueue.empty())
        {
            std::string currentUrl = linkQueue.front();
            linkQueue.pop();
            if (processedLinks.find(currentUrl) != processedLinks.end())
            {
                continue;
            }

            std::cout << "Processing: " << currentUrl << "\n\n";

            std::string htmlContent = downloadPage(currentUrl, cookieFile);
            std::vector<std::string> links = extractLinks(htmlContent, baseUrl);

            for (const std::string &link : links)
            {
                if (processedLinks.find(link) == processedLinks.end() &&
                    isMatchingUrl(link, baseUrl))
                {
                    linkQueue.push(link);
                    learning_links.push_back(link);
                    processedLinks.insert(link);
                }
            }
            processedLinks.insert(currentUrl);
        }

        for (const auto &currentUrl : learning_links)
        {
            std::string htmlContent = downloadPage(currentUrl, cookieFile);

            if (containsTrainingPeriodText(htmlContent))
            {
                xmlDocPtr doc =
                    htmlReadMemory(htmlContent.c_str(), htmlContent.size(), nullptr,
                                   "UTF-8", HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
                if (!doc)
                {
                    throw std::runtime_error("Failed to parse HTML");
                }
                xmlXPathContextPtr context = xmlXPathNewContext(doc);
                xmlXPathObjectPtr html_elements = xmlXPathEvalExpression(
                    (xmlChar *)"//div[contains(@class, 'courses-list__item "
                               "courses-list__item_active')]",
                    context);
                if (html_elements == NULL)
                {
                    throw std::runtime_error("Failed to evaluate XPath expression");
                }

                xmlNodePtr node = html_elements->nodesetval->nodeTab[0];
                std::string course = (char *)node->children->content;
                std::cout << currentUrl << "\nНайден курс: " << trim(course) << "\n";

                html_elements = xmlXPathEvalExpression(
                    (xmlChar *)"//div[contains(@class, 'learning-cert__text')]",
                    context);
                if (html_elements == NULL)
                {
                    throw std::runtime_error("Failed to evaluate XPath expression");
                }

                node = html_elements->nodesetval->nodeTab[0];
                std::string period = (char *)node->children->content;
                std::cout << "Период обучения: " << trim(period) << "\n\n";
                xmlFreeDoc(doc);
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error processing URL " << baseUrl << ": " << e.what()
                  << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
