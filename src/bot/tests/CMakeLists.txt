add_executable(
    test_utils
    test.cpp
)

target_include_directories(test_utils PRIVATE
    ${CMAKE_SOURCE_DIR}/src/bot
)

target_link_libraries(test_utils PRIVATE
    ${Sqlpp11_LIBRARIES}
    ${PostgreSQL_LIBRARIES}
    ${stduuid_LIBRARIES}
    ${tgbotstater_LIBRARIES}
    ${rapidfuzz_LIBRARIES}
    ${httplib_LIBRARIES}
)

add_test(NAME test_utils COMMAND test_utils)
