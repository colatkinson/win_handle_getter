# CMakeList.txt : CMake project for win_handle_getter, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_library (win_handle_getter "win_handle_getter.cpp" "win_handle_getter.h")
target_include_directories(win_handle_getter PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>)

# TODO: Add tests and install targets if needed.
