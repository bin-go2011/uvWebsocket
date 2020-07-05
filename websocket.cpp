#include "websocket.hpp"

WebSocket::WebSocket(uv_loop_t * loop, uv_tcp_t * socket)
:loop(loop), socket(socket), state(WebSocketState::kClosed)
{
}

WebSocket::~WebSocket()
{

}

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

}