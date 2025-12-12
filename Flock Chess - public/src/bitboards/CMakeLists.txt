# Expose headers in this folder to anything that links to this library
add_library(bitboards
    bishops.cpp
    rook.cpp
    knight.cpp
    duck.cpp
    king.cpp
    pawn.cpp
    magic.cpp
)

target_include_directories(bitboards PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(bitboards PUBLIC bitutils)
