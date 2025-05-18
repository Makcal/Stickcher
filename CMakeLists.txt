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
set(sticker_searcher_Version_Minor 3)
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
add_compile_options(-ftime-report)
add_compile_options(-O0 -g)

# dependencies
find_package(Sqlpp11 REQUIRED)
find_package(PostgreSQL REQUIRED)
find_package(stduuid REQUIRED)
find_package(TgBotStater REQUIRED)
find_package(rapidfuzz REQUIRED)
find_package(httplib REQUIRED)

add_custom_target(ddl2cpp
    COMMAND bash -c "cd ${CMAKE_SOURCE_DIR}/src/bot/db && \
        python3 ${sqlpp11_PACKAGE_FOLDER_RELEASE}/bin/ddl2cpp \
            --datatype-file=<(echo Text, uuid) init.sql tables.hpp tables && \
        mv tables.hpp.h tables.hpp"
    COMMENT "Convert SQL DDL to C++ header"
    VERBATIM
)

# main program
add_executable(main src/bot/main.cpp)

# setup
target_include_directories(main PRIVATE
    ${CMAKE_SOURCE_DIR}/src/bot
)

target_precompile_headers(main PRIVATE
    <tg_stater/bot.hpp>
    <tgbot/Bot.h>
    <tgbot/Api.h>
    <tgbot/types/Message.h>
    <sqlpp11/sqlpp11.h>
    <uuid.h>
    <httplib.h>
    [["db/db.hpp"]]
    [["db/tables.hpp"]]
    [["utils.hpp"]]
)

target_link_libraries(main PRIVATE
    ${Sqlpp11_LIBRARIES}
    ${PostgreSQL_LIBRARIES}
    ${stduuid_LIBRARIES}
    ${tgbotstater_LIBRARIES}
    ${rapidfuzz_LIBRARIES}
    ${httplib_LIBRARIES}
)

