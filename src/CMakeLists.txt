# Add include directory
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/cmd)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Library for the vector addition implementations
add_library(vector_add_lib
    scalar_add.cpp
    neon_add.cpp
)

# Add include directories to the library
target_include_directories(vector_add_lib PUBLIC include)