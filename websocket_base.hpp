#ifndef WEBSOCKET_HPP_
#define WEBSOCKET_HPP_

#include <map>
#include <string>
#include <stdint.h>

#include <uv.h>
#include "websocket_parser.hpp"

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

class WebSocket {
protected:
    uv_tcp_t* socket;
    uv_loop_t* loop;

    WebSocketState state;
    bool http_handshake_done = false;

    void send_http_request(std::string path,
                std::string host, std::string query, const WebSocketHeaders& custom_headers);
    
    static void on_write(uv_write_t* req, int status);
    static void on_shutdown(uv_shutdown_t* req, int status);
    static void on_handle_close(uv_handle_t* handle);

    void enque_fragment(const char* buf, size_t len);
    void handle_packet(char* buf, size_t len);
    size_t decode_frame(char* buf, size_t len);

    std::string fragment_buffer;
    size_t fragment_offset = 0;
public:
    WebSocket(uv_loop_t* loop, uv_tcp_t* socket);
    ~WebSocket();

    virtual void close();
    virtual void send_raw(char* str, size_t len);

    std::function<void(WebSocket*, const char*, const char*)> on_close;
    std::function<void(WebSocket*, const char*, const char*)> on_error;
    std::function<void(WebSocket*, char*, size_t, WebSocketOpcode)> on_message;

    WebSocketHeaders custom_headers;

    void send(std::string message, WebSocketOpcode op = kTextFrame);
    void send(const char* buf, size_t length, WebSocketOpcode op);
};


#endif