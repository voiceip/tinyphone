cmake_minimum_required(VERSION 3.5)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
set (CMAKE_CXX_STANDARD 14)

project(tinyphone VERSION 0.0.0 LANGUAGES C CXX)

file(GLOB SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../tinyphone/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../tinyphone/*.cpp
)


list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../tinyphone/guicon.cpp)
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../tinyphone/splash.cpp)
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../tinyphone/guicon.h)
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../tinyphone/splash.h)


# foreach(file_path ${SOURCES})
#     message(${file_path})
# endforeach()

set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
# set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost 1.78.0 COMPONENTS date_time filesystem system) 

# if(Boost_FOUND)
# endif()

include_directories(${Boost_INCLUDE_DIRS}) 
add_executable(tinyphone ${SOURCES} console.cpp)
target_include_directories(tinyphone PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../lib/pjproject/pjlib/include)
target_include_directories(tinyphone PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../lib/pjproject/pjlib-util/include)
target_include_directories(tinyphone PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../lib/pjproject/pjmedia/include)
target_include_directories(tinyphone PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../lib/pjproject/pjnath/include)
target_include_directories(tinyphone PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../lib/pjproject/pjsip/include)
target_include_directories(tinyphone PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../lib/json/include)
target_include_directories(tinyphone PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../lib/statsd-cpp/include)
target_include_directories(tinyphone PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../lib/crow/include)
target_include_directories(tinyphone PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../lib/portaudio/include)
target_link_libraries(tinyphone ${Boost_LIBRARIES})