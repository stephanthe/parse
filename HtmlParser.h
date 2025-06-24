#ifndef HTMLPARSER_H
#define HTMLPARSER_H

#include <libxml/HTMLparser.h>
#include <libxml/tree.h>
#include <vector>
#include <string>

class HtmlParser {
public:
    std::vector<std::string> extractDetails(const std::string &html, const std::string &baseUrl);
    std::vector<std::string> extractLinks(const std::string &html, const std::string &baseUrl);
private:
    void findLinks(xmlNodePtr node, const std::string &baseUrl, std::vector<std::string> &links);
    void findEventDetails(xmlNodePtr node, std::vector<std::string> &titles);
};

#endif // HTMLPARSER_H
