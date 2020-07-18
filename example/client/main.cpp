#include <iostream>
#include "websocket_client.hpp"
#include "json/json.h"

std::string _mreUUID;

void send_register_req(WebSocket* ws)
{
    Json::Value root;
    root["message"] = "register_req";
    root["mreUUID"] = _mreUUID;
    root["resource"]["bandwidth"] = "4096000";

    root["transport"]["transportType"] = "internet";
    root["transport"]["protocol"] = "RTP";

    Json::Value data;
    data["ipVer"] = "4";
    data["protocol"] = "tcp";
    data["address"] = "10.220.214.133";
    data["port"] = "41504";
    root["transport"]["internet"] = data;

    Json::FastWriter writer;
    const std::string json_file = writer.write(root);
    ws->send(json_file);
}

int main() {

    uv_loop_t* loop = uv_default_loop();

    WebSocketClient ws(loop);

    ws.on_connection = [](WebSocket* ws, const HttpResponse& resp) {
        std::cout << "Connected to " << ws->remote_address() << std::endl;
    };

    ws.on_message = [](WebSocket* ws, char* data, size_t len, WebSocketOpcode opcode) {
        Json::Reader reader;
        Json::Value root;
        reader.parse(std::string(data, len), root);
		std::cout << "Received: \n" << root << std::endl;

		std::string msg_name = root["message"].asString();
		 if (msg_name == "new_re_connect") {
             _mreUUID = root["mreUUID"].asString();

             send_register_req(ws);
		 } else if (msg_name == "register_resp") {

         }
    };

    ws.connect("ws://10.220.214.47:8089/ipc/ws/1.0/relay");

    uv_run(loop, UV_RUN_DEFAULT);

    return 0;
}