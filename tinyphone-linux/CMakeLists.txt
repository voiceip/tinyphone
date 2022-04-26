cmake_minimum_required(VERSION 3.5)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
set (CMAKE_CXX_STANDARD 14)
set(CMAKE_WARN_DEPRECATED OFF)

project(tinyphone VERSION 0.0.0 LANGUAGES C CXX)


file(GLOB SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../tinyphone/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../tinyphone/*.cpp
)


list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../tinyphone/guicon.cpp)
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../tinyphone/splash.cpp)
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../tinyphone/guicon.h)
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../tinyphone/splash.h)
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../tinyphone/winapp.cpp)


# foreach(file_path ${SOURCES})
#     message(${file_path})
# endforeach()
# SET(CMAKE_FIND_PACKAGE_SORT_ORDER NATURAL)
# SET(CMAKE_FIND_PACKAGE_SORT_DIRECTION DEC)


# set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
# set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost 1.73 REQUIRED COMPONENTS date_time filesystem system) 
include_directories(${Boost_INCLUDE_DIRS}) 

find_package(PkgConfig REQUIRED) 
pkg_check_modules(PJSIP libpjproject>=1.14 REQUIRED)
include_directories(${PJSIP_INCLUDE_DIRS})


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
target_include_directories(tinyphone PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../lib)
target_include_directories(tinyphone PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../tinyphone)
link_directories(tinyphone ${Boost_LIBRARY_DIRS})
target_link_libraries(tinyphone ${Boost_LIBRARIES})
target_link_libraries(tinyphone ${PJSIP_LIBRARIES})

find_package(nlohmann_json 3.10.5 REQUIRED)
target_link_libraries(tinyphone nlohmann_json::nlohmann_json)

pkg_check_modules(CRYPTOPP libcrypto++ REQUIRED)
target_link_libraries(tinyphone ${CRYPTOPP_LIBRARIES})

# Typically you don't care so much for a third party library's tests to be
# run from your own project's code.
# set(JSON_BuildTests OFF CACHE INTERNAL "")
# set(JSON_Install OFF CACHE INTERNAL "")
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../lib/json build)
# target_link_libraries(tinyphone PRIVATE nlohmann_json::nlohmann_json)
