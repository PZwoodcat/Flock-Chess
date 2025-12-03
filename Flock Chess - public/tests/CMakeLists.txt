enable_testing()
include(CTest)

include(FetchContent)

# Ensure GoogleTest uses the same runtime (/MDd or /MD)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/heads/main.zip
)

FetchContent_MakeAvailable(googletest)

add_executable(test_multiply test_multiply.cpp)

target_link_libraries(test_multiply
    PRIVATE
        multiply
        gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_multiply)
