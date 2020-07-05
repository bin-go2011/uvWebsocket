#ifndef WEBSOCKET_HPP_
#define WEBSOCKET_HPP_

#include <map>
#include <string>
#include <stdint.h>

//#include <http_parser.h>
#include <uv.h>
#include "url.hpp"

static constexpr char* ws_magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

typedef std::map<std::string, std::string> WebSocketHeaders;

enum WebSocketState {
    kClosed = 0,
    kClosing = 1,
    kOpen = 2,
    kOpening = 3,
};

enum WebSocketOpcode {
    kContinuation = 0,
    kTextFrame = 1,
    kBinaryFrame = 2,
    kClose = 8,
    kPing = 9,
    kPong = 10,
};

struct WebSocketFrame {
    size_t payload_length;
    WebSocketOpcode opcode;
    bool is_masked;
    uint32_t mask;
    size_t frame_length;
};

class HttpParser {
};

class HttpRequest {
};

class HttpResponse {
};

class WebSocketException : public std::exception
{
private:
    std::string message;
public:
    WebSocketException() 
        : std::exception() {}
    WebSocketException(std::string _What)
        : message(_What), std::exception() {}
};

class WebSocket {
protected:
    uv_tcp_t* socket;
    uv_loop_t* loop;

    WebSocketState state;

public:
    WebSocket(uv_loop_t* loop, uv_tcp_t* socket);
    ~WebSocket();
};

class WebSocketClient : public WebSocket {
private:
    Url::Query query_params;
    std::string path, host, port;

    static void on_getaddrinfo_end(uv_getaddrinfo_t* req, int status,
                addrinfo* res);
    static void on_connect_end(uv_connect_t* req, int status);
    
    void on_tcp_connect();
public:
    WebSocketClient(uv_loop_t* loop, uv_tcp_t* socket = nullptr);

    void connect(std::string uri);
    void connect(addrinfo* addr);
};

#endif