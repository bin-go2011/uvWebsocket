#include "websocket_client.hpp"

WebSocketClient::WebSocketClient(uv_loop_t* loop, uv_tcp_t* socket)
:WebSocket(loop, socket)
{
}

void WebSocketClient::connect(std::string uri)
{
    http_parser_url u;
    http_parser_url_init(&u);
    http_parser_parse_url(uri.c_str(), uri.length(), 0, &u);
    host = uri.substr(u.field_data[UF_HOST].off, u.field_data[UF_HOST].len);
    path = uri.substr(u.field_data[UF_PATH].off, u.field_data[UF_PATH].len);
    query = uri.substr(u.field_data[UF_QUERY].off, u.field_data[UF_QUERY].len);

    port = std::to_string(u.port);
    if (port.empty())
        port = "80";

    if (!socket) {
        socket = new uv_tcp_t{};
        socket->data = this;

        if (int res = uv_tcp_init(loop, socket)) {
            //throw WebSocketException("failed to init the socket");
        }
    }

    uv_getaddrinfo_t* getaddrinfo_req = new uv_getaddrinfo_t{};
    getaddrinfo_req->data = this;

    addrinfo hints = {};
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_socktype = SOCK_STREAM;

    if (int res = uv_getaddrinfo(loop,
        getaddrinfo_req,
        on_getaddrinfo_end,
        host.c_str(),
        port.c_str(),
        &hints)) {
        // throw WebSocketException("failed to call getaddrinfo");
    }
}

void WebSocketClient::connect(addrinfo * addr)
{
    if (state != WebSocketState::kClosed)
        return;

    state = WebSocketState::kOpening;

    uv_connect_t* connect_req = new uv_connect_t{};
    connect_req->data = this;

    if (int res = uv_tcp_connect(connect_req,
        socket, addr->ai_addr, on_connect_end)) {
        printf("connection error: %s\n", uv_strerror(res));
        // throw WebSocketException("failed to connect");
    }
}

void WebSocketClient::on_getaddrinfo_end(uv_getaddrinfo_t * req,
            int status, addrinfo * res)
{
    WebSocketClient* _this = (WebSocketClient*)req->data;
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
    WebSocketClient* _this = (WebSocketClient*)req->data;
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
    send_http_request(path, host, query, custom_headers);

    if (int res = uv_read_start((uv_stream_t*)socket,
                on_alloc_callback, on_read_callback)) {
        // throw WebSocketException("failed to start reading");
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
    WebSocketClient* sender = (WebSocketClient*)client->data;

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
    
    if (http_handshake_done) {
       handle_packet(packet, len);
    }
    else {
		HttpResponse resp;
		size_t nparsed = resp.parse(packet, len);
        if (nparsed > 0 && resp.status_code() == 101) {

            http_handshake_done = true;
            state = WebSocketState::kOpen;

          if (on_connection)
              on_connection(this, resp);
        }
       
    }    
}

void WebSocketClient::on_tcp_close(int status)
{
    close();
}