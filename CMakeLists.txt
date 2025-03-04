cmake_minimum_required(VERSION 3.16)
project(sticker_searcher)

# Set the default build type
if(NOT CMAKE_BUILD_TYPE)
    set(default_build_type "Debug")

    message(STATUS "${CMAKE_PROJECT_NAME} Set the build type to `${default_build_type}` as none was specified.")
    set(CMAKE_BUILD_TYPE ${default_build_type} CACHE STRING "Choose the build type." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()
message(STATUS "${CMAKE_PROJECT_NAME} Build Type: ${CMAKE_BUILD_TYPE}")

# Set the version for sticker_searcher
set(sticker_searcher_Version_Major 0)
set(sticker_searcher_Version_Minor 1)
set(sticker_searcher_Version_Patch 0)
set(sticker_searcher_Version_Status "-dev")
set(PROJECT_VERSION
    "${sticker_searcher_Version_Major}.${sticker_searcher_Version_Minor}.${sticker_searcher_Version_Patch}${sticker_searcher_Version_Status}"
)
message(STATUS "${CMAKE_PROJECT_NAME} version: ${PROJECT_VERSION}")

# options
set(CMAKE_CXX_STANDDARD 23)
set(CMAKE_CXX_STANDDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# dependencies
find_package(rapidfuzz REQUIRED)
find_package(SqliteOrm REQUIRED)
find_package(stduuid REQUIRED)
find_package(tgbot REQUIRED)

# main program
add_executable(main src/main.cpp)

# setup
target_compile_definitions(main PRIVATE 
    __cpp_pack_indexing=0
    LIBUUID_CPP20_OR_GREATER=1)

# set(TgBot_INCLUDE_DIRS )
target_include_directories(main PRIVATE 
    ${rapidfuzz_INCLUDE_DIRS}
    ${SqliteOrm_INCLUDE_DIRS}
    ${stduuid_INCLUDE_DIRS}
    ${tgbot_INCLUDE_DIRS})

target_link_libraries(main PRIVATE
    ${tgbot_LIBRARIES}
    ${SqliteOrm_LIBRARIES}
)

