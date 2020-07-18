#ifndef HTTP_PARSER_HPP_
#define HTTP_PARSER_HPP_

#include <functional>
#include <map>

extern "C" {
#include "http_parser.h"
}

class HttpParser {
    friend class HttpResponse;
private:
    http_parser parser;
    http_parser_settings settings;
public:
    HttpParser();
    ~HttpParser();

    size_t parse_request(std::string input);
    size_t parse_response(std::string input);

    std::function<void()>      on_message_begin;
    std::function<void(const char*, size_t)> on_url;
    std::function<void(const char*, size_t)> on_status;
    std::function<void(const char*, size_t)> on_header_field;
    std::function<void(const char*, size_t)> on_header_value;
    std::function<void()>      on_headers_complete;
    std::function<void(const char*, size_t)> on_body;
    std::function<void()>      on_message_complete;
    std::function<void()>      on_chunk_header;
    std::function<void()>      on_chunk_complete;
};

class HttpRequest {
public:
    HttpRequest();
    ~HttpRequest();
    std::map<std::string, std::string> headers;
    std::string header, url, status;
    size_t res = 0;

    std::function<void()> end;
};

class HttpResponse {
private:
    HttpParser p;

    std::map<std::string, std::string> headers;
    std::string header, url, status;
public:
    HttpResponse();
    ~HttpResponse();
    int parse(const char* str, size_t len);
    unsigned int status_code() { return p.parser.status_code; }
};

#endif