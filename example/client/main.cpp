#include "websocket.hpp"

int main() {

    uv_loop_t* loop = uv_default_loop();

    WebSocketClient ws(loop);

    ws.connect("ws://localhost:1501");

    return 0;
}