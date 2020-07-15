#include <iostream>
#include "websocket_client.hpp"
#include "json.hpp"

// for convenience
using json = nlohmann::json;

int main() {

    uv_loop_t* loop = uv_default_loop();

    WebSocketClient ws(loop);

    ws.on_message = [](WebSocket* ws, char* data, size_t len, WebSocketOpcode opcode) {
        auto j = json::parse(std::string(data, len));
        std::cout << j.dump() << std::endl;
    };

    ws.connect("ws://10.220.214.47:8089/ipc/ws/1.0/relay");

    uv_run(loop, UV_RUN_DEFAULT);

    return 0;
}