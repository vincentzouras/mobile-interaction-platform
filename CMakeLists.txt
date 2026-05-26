cmake_minimum_required(VERSION 3.10)
project(MobileInteractionPlatform)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Shared network library
add_library(shared_net
    src/shared/net/tcp_client.cpp
    src/shared/net/tcp_server.cpp
    src/shared/net/udp_transmitter.cpp
    src/shared/net/udp_receiver.cpp
)
target_include_directories(shared_net PUBLIC 
    ${CMAKE_SOURCE_DIR}/src/shared/include
)
target_link_libraries(shared_net PRIVATE 
    pthread
)

# RPi executable
add_executable(rpi 
    src/rpi/main.cpp
    src/rpi/perception/camera.cpp
    src/rpi/perception/image_sender.cpp
)
target_link_libraries(rpi PRIVATE 
    shared_net 
    pthread 
    ${OpenCV_LIBS}
)
target_include_directories(rpi PRIVATE 
    ${OpenCV_INCLUDE_DIRS} 
    ${CMAKE_SOURCE_DIR}/src/rpi/include
)

# Laptop executable
add_executable(laptop
    src/laptop/main.cpp
    src/laptop/image_receiver.cpp
)
target_link_libraries(laptop PRIVATE 
    shared_net 
    pthread 
    ${OpenCV_LIBS}
)
target_include_directories(laptop PRIVATE 
    ${OpenCV_INCLUDE_DIRS} 
    ${CMAKE_SOURCE_DIR}/src/laptop/include
)


# Compiler flags
if(UNIX)
    target_compile_options(rpi PRIVATE -Wall -Wextra)
    target_compile_options(laptop PRIVATE -Wall -Wextra)
endif()
