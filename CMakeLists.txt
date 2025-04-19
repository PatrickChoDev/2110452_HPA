cmake_minimum_required(VERSION 3.15)

# Project Name and Version
project(HPA LANGUAGES CXX VERSION 1.0)


if(APPLE)
    set(CMAKE_C_COMPILER "/opt/homebrew/bin/gcc-14")
    set(CMAKE_CXX_COMPILER "/opt/homebrew/bin/g++-14")
endif()

# Set the C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default Build Type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Define source files and include directories
add_subdirectory(src)

# Target for NEON-enabled executable
add_executable(vectorization_neon src/cmd/vectorization.cpp)
target_link_libraries(vectorization_neon PRIVATE vector_add_lib)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm" OR CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    target_compile_options(vectorization_neon PRIVATE -march=armv8-a+simd -O3)
    message(STATUS "NEON optimizations enabled for 'vectorization_neon'.")
else()
    message(WARNING "NEON optimizations requested but not supported on this architecture.")
endif()

# Target for non-NEON executable
add_executable(vectorization_no_neon src/cmd/vectorization.cpp)
target_link_libraries(vectorization_no_neon PRIVATE vector_add_lib)
message(STATUS "Non-NEON target 'vectorization_no_neon' created.")
target_compile_options(vectorization_no_neon PRIVATE -fno-tree-vectorize -O3)
message(STATUS "Non-NEON target 'vectorization_no_neon' created with -fno-tree-vectorize flag.")

# Enable warnings for both targets
foreach(target vectorization_neon vectorization_no_neon)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -O3)
    elseif(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive- /O3)
    endif()
endforeach()

# Target for Threading executable
add_executable(threading src/cmd/threading.cpp)
target_compile_options(threading PRIVATE -O3)

# Target for openmp executable
add_executable(openmp src/cmd/openmp.cpp)
target_link_directories(openmp PRIVATE "/opt/homebrew/opt/libomp/lib")
target_include_directories(openmp PRIVATE "/opt/homebrew/opt/libomp/include")
target_link_libraries(openmp PRIVATE omp)
target_compile_options(openmp PRIVATE -O3 -fopenmp)


# Output build artifacts in a dedicated directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
