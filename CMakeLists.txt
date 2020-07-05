cmake_minimum_required(VERSION 3.14)
project(uvWebsocket)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(libuv)

add_library(${PROJECT_NAME} 
    websocket.cpp websocket.hpp
    url.cpp url.hpp
)
target_link_libraries(${PROJECT_NAME} uv)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(example)