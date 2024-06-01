cmake_minimum_required(VERSION 3.1...3.29)
project(
    SlurmJobMonitor
    VERSION 1.2.0
    LANGUAGES CXX)

include(FetchContent)
include(cmake/Doxygen.cmake)

add_subdirectory(configuredFiles)

option(SJM_ENABLE_DOXYGEN "Enable doxygen" OFF)

set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
FetchContent_Declare(
  ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG v5.0.0
)
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)

FetchContent_MakeAvailable(argparse)
FetchContent_GetProperties(ftxui)
if(NOT ftxui_POPULATED)
  FetchContent_Populate(ftxui)
  add_subdirectory(${ftxui_SOURCE_DIR} ${ftxui_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

set(CMAKE_CXX_STANDARD 20) # this is a stupid fix for my stupid InteliSense so knows that I'm using the stupid c++20 standard
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

if(SJM_ENABLE_DOXYGEN)
  SlurmJobMonitor_enable_doxygen("awesome-sidebar")
endif()

add_compile_options(-Wall -Wextra -Wpedantic)

add_library(base 
include/FileHandler.hxx
include/GlobalConstants.hxx
include/Job.hxx
include/JobManager.hxx
src/FileHandler.cxx
src/Job.cxx
src/JobManager.cxx)
target_include_directories(base PUBLIC ${ftxui_SOURCE_DIR}/include)
target_include_directories(base PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_executable(monitor main.cxx)

target_link_libraries(monitor PRIVATE base PRIVATE argparse PRIVATE ftxui::screen PRIVATE ftxui::dom )
target_compile_features(monitor PRIVATE cxx_std_20)