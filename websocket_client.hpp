#ifndef WEBSOCKET_CLIENT_HPP_
#define WEBSOCKET_CLIENT_HPP_

#include "websocket_base.hpp"

class WebSocketClient : public WebSocket {
private:
    std::string path, host, port, query;

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

    std::function<void(WebSocket*, const HttpResponse&)> on_connection;
};

#endif
