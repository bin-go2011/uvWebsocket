#include "websocket.hpp"

WebSocket::WebSocket(uv_loop_t * loop, uv_tcp_t * socket)
:loop(loop), socket(socket), state(WebSocketState::kClosed)
{
}

WebSocket::~WebSocket()
{

}

void WebSocket::close() 
{
    if (state != WebSocketState::kOpen)
        return;

    state = WebSocketState::kClosing;

    auto sd_req = new uv_shutdown_t{};
    sd_req->data = this;

    if (int res = uv_shutdown(sd_req, (uv_stream_t*)socket, on_shutdown)) {
        throw WebSocketException("failed to shutdown the socket");
    }
}

void WebSocket::send_http_request(std::string path,
            std::string host, Url::Query& query, const WebSocketHeaders& custom_headers)
{
    std::string final_path = path;
    if (query.size() > 0) {
        final_path += "?";
        for (int i = 0; i < query.size(); i++) {
            final_path += query[i].key();
            final_path += "=";
            final_path += query[i].val();
            if (query.size() - i > 1) {
                final_path += "&";
            }
        }
    }

    if (final_path.empty()) {
        final_path = "/";
    }

    std::string request =
        "GET " + final_path + " HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Connection: Upgrade\r\n"
        "Upgrade: websocket\r\n"
        "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
        "Sec-WebSocket-Version: 13\r\n";

    for (auto&& header : custom_headers) {
        request += header.first + ": " + header.second + "\r\n";
    }

    request += "\r\n";
    send_raw((char*)request.c_str(), request.length());
}

void WebSocket::send_raw(char * str, size_t len)
{
    auto write_req = new uv_write_t{};
    write_req->data = this;

    uv_buf_t bufs[1];
    bufs[0].base = str;
    bufs[0].len = len;

    if (int res = uv_write(write_req, (uv_stream_t*)socket, bufs, 1, on_write)) {
        throw WebSocketException("failed to write into socket");
    }
}

void WebSocket::on_write(uv_write_t * req, int status)
{
    auto _this = (WebSocket*)req->data;

    // if (status) {
    //     _this->close();
    // }

    delete req;
}

void WebSocket::on_shutdown(uv_shutdown_t * req, int status)
{
    auto _this = (WebSocket*)req->data;
    delete req;

    if (status) {
        _this->state = WebSocketState::kClosed;
        // if (_this->on_error)
        //     _this->on_error(_this, uv_err_name(status), uv_strerror(status));
        // else
        //     throw WebSocketException("failed to shutdown the socket");
        return;
    }

    uv_close((uv_handle_t*)_this->socket, on_handle_close);
}

void WebSocket::on_handle_close(uv_handle_t * handle)
{
    auto _this = (WebSocket*)handle->data;
    _this->state = WebSocketState::kClosed;
    delete handle;
    _this->socket = nullptr;
}

