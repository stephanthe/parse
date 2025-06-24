#include "HtmlParser.h"
#include <stdexcept>
//#include <iostream>

void HtmlParser::findLinks(xmlNodePtr node, const std::string &baseUrl, std::vector<std::string> &links) {
    for (xmlNodePtr cur_node = node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE && xmlStrcmp(cur_node->name, (const xmlChar *)"a") == 0) {
            xmlAttrPtr attr = cur_node->properties;
            while (attr) {
                if (xmlStrcmp(attr->name, (const xmlChar *)"href") == 0) {
                    std::string link = reinterpret_cast<const char*>(xmlNodeGetContent(attr->children));
                    links.push_back(baseUrl + link);
                }
                attr = attr->next;
            }
        }
        findLinks(cur_node->children, baseUrl, links);
    }
}

void HtmlParser::findEventDetails(xmlNodePtr node, std::vector<std::string> &titles) {
    for (xmlNodePtr cur_node = node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE && xmlStrcmp(cur_node->name, (const xmlChar *)"div") == 0) {
            xmlAttrPtr attr = cur_node->properties;
            while (attr) {
                if (xmlStrcmp(attr->name, (const xmlChar *)"class") == 0) {
                    std::string class_name(reinterpret_cast<const char*>(xmlNodeGetContent(attr->children)));
                    if (class_name.find("dod_new-event__title js-dod-new-event-title") != std::string::npos) {
                        xmlNodePtr title_node = cur_node->children;
                        std::string content = reinterpret_cast<const char*>(title_node->content);
                        if (!content.empty()) {
                            int first_symbol = 0;
                            int last_symbol = 0;
                            for (auto &c : content) {
                                if (last_symbol == 0) {
                                    if (c <= ' ' && c > 0) {
                                        first_symbol++;
                                    }
                                    else {
                                        last_symbol = first_symbol;
                                    }
                                }
                                else {
                                    if (c == '\n') {
                                        break;
                                    }
                                    last_symbol++;
                                }
                            }
                            content = content.substr(first_symbol, last_symbol - first_symbol + 1);
                            titles.push_back(content);
                        }
                    }
            }
                attr = attr->next;
            }
        }
        findEventDetails(cur_node->children, titles);
    }
}

std::vector<std::string> HtmlParser::extractDetails(const std::string &html, const std::string &baseUrl) {
    std::vector<std::string> titles;
    std::vector<std::string> times;

    htmlDocPtr doc = htmlReadMemory(html.c_str(), html.size(), nullptr, "UTF-8", HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
    if (!doc) {
        throw std::runtime_error("Failed to parse HTML");
    }
    xmlNodePtr root = xmlDocGetRootElement(doc);
    findEventDetails(root, titles);
    xmlFreeDoc(doc);
    xmlCleanupParser();

    return {titles};
}

std::vector<std::string> HtmlParser::extractLinks(const std::string &html, const std::string &baseUrl) {
    std::vector<std::string> links;
    htmlDocPtr doc = htmlReadMemory(html.c_str(), html.size(), nullptr, "UTF-8", HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
    if (!doc) {
        throw std::runtime_error("Failed to parse HTML");
    }
    xmlNodePtr root = xmlDocGetRootElement(doc);
    findLinks(root, baseUrl, links);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return links;
}

/*
#include "HtmlParser.h"
#include <stdexcept>

void HtmlParser::findLinks(xmlNodePtr node, const std::string &baseUrl, std::vector<std::string> &links) {
    if (!node)
        return;
    if (xmlStrcmp(node->name, (const xmlChar *)"a") == 0) {
        xmlAttr *attr = node->properties;
        while (attr) {
            if (xmlStrcmp(attr->name, (const xmlChar *)"href") == 0) {
                std::string href = (char *)attr->children->content;
                if (!href.empty()) {
                    if (href.find("http") != 0 && href[0] == '/') {
                        std::string absoluteUrl = baseUrl + href;
                        links.push_back(absoluteUrl);
                    } else if (href.find(baseUrl) == 0) {
                        links.push_back(href);
                    }
                }
            }
            attr = attr->next;
        }
    }
    xmlNodePtr child = node->children;
    while (child) {
        findLinks(child, baseUrl, links);
        child = child->next;
    }
}

std::vector<std::string> HtmlParser::extractLinks(const std::string &html, const std::string &baseUrl) {
    std::vector<std::string> links;
    htmlDocPtr doc = htmlReadMemory(html.c_str(), html.size(), nullptr, "UTF-8", HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING);
    if (!doc) {
        throw std::runtime_error("Failed to parse HTML");
    }
    xmlNodePtr root = xmlDocGetRootElement(doc);
    findLinks(root, baseUrl, links);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return links;
}
*/