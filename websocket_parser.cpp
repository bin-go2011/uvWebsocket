#include "websocket_parser.hpp"

HttpParser::HttpParser() : parser{}, settings{}
{
    settings.on_body = [](http_parser* p, const char* at,
        size_t len) -> int {
        HttpParser* _this = (HttpParser*)p->data;
        if (_this->on_body)
            _this->on_body(at, len);
        return 0;
    };
    settings.on_header_field = [](http_parser* p, const char* at, 
        size_t len) -> int {
        HttpParser* _this = (HttpParser*)p->data;
        if (_this->on_header_field)
            _this->on_header_field(at, len);
        return 0;
    };
    settings.on_header_value = [](http_parser* p, const char* at, 
        size_t len) -> int {
        HttpParser* _this = (HttpParser*)p->data;
        if (_this->on_header_value)
            _this->on_header_value(at, len);
        return 0;
    };
    settings.on_status = [](http_parser* p, const char* at, 
        size_t len) -> int {
        HttpParser* _this = (HttpParser*)p->data;
        if (_this->on_status)
            _this->on_status(at, len);
        return 0;
    };
    settings.on_url = [](http_parser* p, const char* at, 
        size_t len) -> int {
        HttpParser* _this = (HttpParser*)p->data;
        if (_this->on_url)
            _this->on_url(at, len);
        return 0;
    };
    settings.on_chunk_complete = [](http_parser* p) -> int {
        HttpParser* _this = (HttpParser*)p->data;
        if (_this->on_chunk_complete)
            _this->on_chunk_complete();
        return 0;
    };
    settings.on_chunk_header = [](http_parser* p) -> int {
        HttpParser* _this = (HttpParser*)p->data;
        if (_this->on_chunk_header)
            _this->on_chunk_header();
        return 0;
    };
    settings.on_headers_complete = [](http_parser* p) -> int {
        HttpParser* _this = (HttpParser*)p->data;
        if (_this->on_headers_complete)
            _this->on_headers_complete();
        return 0;
    };
    settings.on_message_begin = [](http_parser* p) -> int {
        HttpParser* _this = (HttpParser*)p->data;
        if (_this->on_message_begin)
            _this->on_message_begin();
        return 0;
    };
    settings.on_message_complete = [](http_parser* p) -> int {
        HttpParser* _this = (HttpParser*)p->data;
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

HttpRequest::HttpRequest()
{

}

HttpRequest::~HttpRequest()
{

}

HttpResponse::HttpResponse()
{

}

HttpResponse::~HttpResponse()
{

}

int HttpResponse::parse(const char* str, size_t len)
{
    bool completed = false;

    p.on_header_field = [this](const char* cstr, size_t len) {
        this->header = std::string(cstr, len);
    };
    p.on_header_value = [this](const char* cstr, size_t len) {
        std::string str(cstr, len);
        this->headers[this->header] = str;
    };
    p.on_url = [this](const char* cstr, size_t len) {
        this->url = std::string(cstr, len);
    };
    p.on_status = [this](const char* cstr, size_t len) {
        this->status = std::string(cstr, len);
    };
    p.on_message_complete = [&completed]() {
        completed = true;
    };

    size_t parsed = p.parse_response(std::string(str, len));
    return completed ? parsed : 0;
}