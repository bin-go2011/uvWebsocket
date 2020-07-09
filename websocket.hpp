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

    void send_http_request(std::string path,
                std::string host, Url::Query& query, const WebSocketHeaders& custom_headers);
    
    static void on_write(uv_write_t* req, int status);
    static void on_shutdown(uv_shutdown_t* req, int status);
    static void on_handle_close(uv_handle_t* handle);
public:
    WebSocket(uv_loop_t* loop, uv_tcp_t* socket);
    ~WebSocket();

    virtual void close();
    virtual void send_raw(char* str, size_t len);

    WebSocketHeaders custom_headers;
};

class WebSocketClient : public WebSocket {
private:
    Url::Query query_params;
    std::string path, host, port;

    static void on_getaddrinfo_end(uv_getaddrinfo_t* req, int status,
                addrinfo* res);
    static void on_connect_end(uv_connect_t* req, int status);
    static void on_alloc_callback(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
    static void on_read_callback(uv_stream_t* client, ssize_t nbuf, const uv_buf_t* buf);

    void on_tcp_connect();
    void on_tcp_packet(char* packet, ssize_t len);
    void on_tcp_close(int status);

public:
    WebSocketClient(uv_loop_t* loop, uv_tcp_t* socket = nullptr);

    void connect(std::string uri);
    void connect(addrinfo* addr);
};

#endif