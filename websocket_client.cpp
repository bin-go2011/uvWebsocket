#include "websocket_client.hpp"

WebSocketClient::WebSocketClient(uv_loop_t* loop, uv_tcp_t* socket)
:WebSocket(loop, socket)
{
}

void WebSocketClient::connect(std::string uri)
{
    Url u(uri);
    host = u.host();
    path = u.path();
    port = u.port();

    if (port.empty())
        port = "80";

    query_params = u.query();

    if (!socket) {
        socket = new uv_tcp_t{};
        socket->data = this;

        if (int res = uv_tcp_init(loop, socket)) {
            throw WebSocketException("failed to init the socket");
        }
    }

    auto getaddrinfo_req = new uv_getaddrinfo_t{};
    getaddrinfo_req->data = this;

    addrinfo hints = {};
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = u.ip_version() == 6 ? AF_INET6 : AF_INET;

    if (int res = uv_getaddrinfo(loop,
        getaddrinfo_req,
        on_getaddrinfo_end,
        host.c_str(),
        port.c_str(),
        &hints)) {
        throw WebSocketException("failed to call getaddrinfo");
    }
}

void WebSocketClient::connect(addrinfo * addr)
{
    if (state != WebSocketState::kClosed)
        return;

    state = WebSocketState::kOpening;

    auto connect_req = new uv_connect_t{};
    connect_req->data = this;

    if (int res = uv_tcp_connect(connect_req,
        socket, addr->ai_addr, on_connect_end)) {
        printf("connection error: %s\n", uv_strerror(res));
        throw WebSocketException("failed to connect");
    }
}

void WebSocketClient::on_getaddrinfo_end(uv_getaddrinfo_t * req,
            int status, addrinfo * res)
{
    auto _this = (WebSocketClient*)req->data;
    delete req;

    if (status) {
        _this->state = WebSocketState::kClosed;
        // if (_this->on_error)
        //     _this->on_error(_this, uv_err_name(status), uv_strerror(status));
        // else
        //     throw WebSocketException("failed to getaddrinfo");
        return;
    }

    _this->connect(res);
}

void WebSocketClient::on_connect_end(uv_connect_t * req, int status)
{
    auto _this = (WebSocketClient*)req->data;
    delete req;

    if (status) {
        _this->state = WebSocketState::kClosed;
        // if (_this->on_error)
        //     _this->on_error(_this, uv_err_name(status), uv_strerror(status));
        // else
        //     throw WebSocketException("failed to connect");
        return;
    }

    _this->on_tcp_connect();
}

void WebSocketClient::on_tcp_connect()
{
    send_http_request(path, host, query_params, custom_headers);

    if (int res = uv_read_start((uv_stream_t*)socket,
                on_alloc_callback, on_read_callback)) {
        throw WebSocketException("failed to start reading");
    }
}

void WebSocketClient::on_alloc_callback(uv_handle_t * handle, 
            size_t suggested_size, uv_buf_t * buf)
{
    buf->base = new char[suggested_size];
    buf->len = suggested_size;
}

void WebSocketClient::on_read_callback(uv_stream_t * client, 
            ssize_t nbuf, const uv_buf_t * buf)
{
    auto sender = (WebSocketClient*)client->data;

    if (nbuf > 0) {
        sender->on_tcp_packet(buf->base, nbuf);
        delete[] buf->base;
    }
    else {
        sender->on_tcp_close(nbuf);
    }
}

void WebSocketClient::on_tcp_packet(char * packet, ssize_t len)
{
    //if (http_handshake_done) {
    //    handle_packet(packet, len);
    //}
    //else {
    //    auto res = parse_http_response(packet, len);
    //    if (res->res) {
    //        res->end = std::bind(&WebSocketClient::on_response_end, this, res);
    //        if (res->res < len) {
    //            enque_fragment(packet + res->res, len - res->res);
    //        }

    //        if (on_connection)
    //            on_connection(this, res);
    //    }
    //}    
}

void WebSocketClient::on_tcp_close(int status)
{
    close();
}