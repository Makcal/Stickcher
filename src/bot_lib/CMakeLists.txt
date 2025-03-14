cmake_minimum_required(VERSION 3.16)

find_package(tgbot REQUIRED)

add_library(bot_lib INTERFACE)

target_include_directories(bot_lib INTERFACE
    include
    ${tgbot_INCLUDE_DIRS})
target_link_libraries(bot_lib INTERFACE
    ${tgbot_LIBRARIES})
