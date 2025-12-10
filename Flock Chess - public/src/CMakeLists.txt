add_subdirectory(utils)
add_subdirectory(bitboards)

add_library(multiply multiply.cpp)
target_include_directories(multiply PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(movegen movegen.cpp)
target_include_directories(movegen PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(movegen PRIVATE bitboards bitutils)

add_executable(entry entry.cpp)
target_link_libraries(entry PRIVATE multiply bitboards movegen)
