#include "http_parser.hpp"

HttpParser::HttpParser() : parser{}, settings{}
{
    settings.on_body = [](http_parser* p, const char* at,
        size_t len) -> int {
        auto _this = (HttpParser*)p->data;
        if (_this->on_body)
            _this->on_body(at, len);
        return 0;
    };
    settings.on_header_field = [](http_parser* p, const char* at, 
        size_t len) -> int {
        auto _this = (HttpParser*)p->data;
        if (_this->on_header_field)
            _this->on_header_field(at, len);
        return 0;
    };
    settings.on_header_value = [](http_parser* p, const char* at, 
        size_t len) -> int {
        auto _this = (HttpParser*)p->data;
        if (_this->on_header_value)
            _this->on_header_value(at, len);
        return 0;
    };
    settings.on_status = [](http_parser* p, const char* at, 
        size_t len) -> int {
        auto _this = (HttpParser*)p->data;
        if (_this->on_status)
            _this->on_status(at, len);
        return 0;
    };
    settings.on_url = [](http_parser* p, const char* at, 
        size_t len) -> int {
        auto _this = (HttpParser*)p->data;
        if (_this->on_url)
            _this->on_url(at, len);
        return 0;
    };
    settings.on_chunk_complete = [](http_parser* p) -> int {
        auto _this = (HttpParser*)p->data;
        if (_this->on_chunk_complete)
            _this->on_chunk_complete();
        return 0;
    };
    settings.on_chunk_header = [](http_parser* p) -> int {
        auto _this = (HttpParser*)p->data;
        if (_this->on_chunk_header)
            _this->on_chunk_header();
        return 0;
    };
    settings.on_headers_complete = [](http_parser* p) -> int {
        auto _this = (HttpParser*)p->data;
        if (_this->on_headers_complete)
            _this->on_headers_complete();
        return 0;
    };
    settings.on_message_begin = [](http_parser* p) -> int {
        auto _this = (HttpParser*)p->data;
        if (_this->on_message_begin)
            _this->on_message_begin();
        return 0;
    };
    settings.on_message_complete = [](http_parser* p) -> int {
        auto _this = (HttpParser*)p->data;
        if (_this->on_message_complete)
            _this->on_message_complete();
        return 0;
    };
}

HttpParser::~HttpParser()
{
}

size_t HttpParser::parse_request(std::string input)
{
    parser = {};
    parser.data = this;
    http_parser_init(&parser, http_parser_type::HTTP_REQUEST);
    return http_parser_execute(&parser, &settings, 
        input.data(), input.size());
}

size_t HttpParser::parse_response(std::string input)
{
    parser = {};
    parser.data = this;
    http_parser_init(&parser, http_parser_type::HTTP_RESPONSE);
    return http_parser_execute(&parser, &settings,
        input.data(), input.size());
}
