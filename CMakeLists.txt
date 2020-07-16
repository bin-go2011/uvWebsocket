cmake_minimum_required(VERSION 3.14)
project(uvWebsocket)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(libuv)

add_library(http_parser
    http-parser/http_parser.c
    http-parser/http_parser.h
)
target_include_directories(http_parser PUBLIC http-parser)

add_library(${PROJECT_NAME} 
    websocket_base.cpp websocket_base.hpp
    websocket_client.cpp websocket_client.hpp
    websocket_parser.cpp websocket_parser.hpp
    url.cpp url.hpp
)
target_link_libraries(${PROJECT_NAME} uv http_parser)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(example)