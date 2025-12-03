add_library(multiply multiply.cpp)

target_include_directories(multiply PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(entry entry.cpp)
target_link_libraries(entry PRIVATE multiply)