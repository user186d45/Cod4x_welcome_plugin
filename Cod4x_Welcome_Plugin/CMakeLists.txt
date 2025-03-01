cmake_minimum_required(VERSION 3.10)

# Set some basic project attributes
project(Cod4x_Welcome_Plugin VERSION 1.0.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32 -Wall -O1 -g -fvisibility=hidden -mtune=core2")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -m32 -g -shared")


set(SQLITE3_INCLUDE_DIR "/usr/include/i386-linux-gnu")
set(SQLITE3_LIBRARY "/lib/i386-linux-gnu/libsqlite3.so")
include_directories(${SQLITE3_INCLUDE_DIR})
set(SQLite3_LIBRARIES ${SQLITE3_LIBRARY})  # Set the library variable manually


# Include the configuration header in the build
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/version.h"
    @ONLY
)

# This project will output a shared library
add_library(${PROJECT_NAME} SHARED src/Cod4x_Welcome_Plugin.c)

# Include the SQLite3 headers
target_include_directories(${PROJECT_NAME} PUBLIC ${SQLite3_INCLUDE_DIRS} ${SQLITE3_INCLUDE_DIR})

# Link the SQLite3 library
target_link_libraries(${PROJECT_NAME} PRIVATE ${SQLite3_LIBRARIES})

# Include the configuration header in the build
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_BINARY_DIR}")
