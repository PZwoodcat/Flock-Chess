add_subdirectory(utils)
add_subdirectory(bitboards)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/variants.ini
    ${CMAKE_CURRENT_BINARY_DIR}/variants.ini
    COPYONLY
)

add_library(multiply multiply.cpp)
target_include_directories(multiply PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
add_library(parser parser.cpp)
target_include_directories(parser PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(movegen movegen.cpp)
target_include_directories(movegen PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(movegen PRIVATE bitboards bitutils parser)

add_executable(entry entry.cpp)
target_link_libraries(entry PRIVATE multiply bitboards movegen)
add_executable(analyze_test analyze_test.cpp)
target_link_libraries(analyze_test PRIVATE parser bitboards movegen)
