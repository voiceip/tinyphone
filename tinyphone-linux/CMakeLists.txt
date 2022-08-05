# CMakeLists.txt

cmake_minimum_required(VERSION 3.11)

project(tinyphone LANGUAGES C CXX)
set(PACKAGE "${PROJECT_NAME}")
set(PACKAGE_NAME "${PROJECT_NAME}")
set(PACKAGE_VERSION "1.0.0")
set(PACKAGE_STRING "${PACKAGE_NAME} ${PACKAGE_VERSION}")
set(PACKAGE_BUGREPORT "hi@kinsh.uk")
set(PACKAGE_TARNAME "${PROJECT_NAME}")
set(PACKAGE_URL "")
set(VERSION "${PACKAGE_VERSION}")

# CMake Policies

# CMP0074: find_package() uses <PackageName>_ROOT variables.
if(POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)
endif()

# CMP0075 Include file check macros honor CMAKE_REQUIRED_LIBRARIES
if(POLICY CMP0075)
    cmake_policy(SET CMP0075 NEW)
endif()


# set(CMAKE_C_COMPILER "/usr/bin/gcc")
# set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_WARN_DEPRECATED FALSE)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-deprecated")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

include(CheckLibraryExists)
include(CheckSymbolExists)
include(CheckCSourceCompiles)
# include(CMakePushCheckState)
# include(GNUInstallDirs)
# include(CMakeDependentOption)


set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
# set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost 1.71 REQUIRED COMPONENTS date_time filesystem system) 
include_directories(${Boost_INCLUDE_DIRS}) 

find_package(PkgConfig REQUIRED) 
pkg_check_modules(PJSIP libpjproject REQUIRED)
include_directories(${PJSIP_INCLUDE_DIRS})

check_symbol_exists("res_ndestroy" "resolv.h" HAVE_RES_NDESTROY)
set(CMAKE_REQUIRED_LIBRARIES resolv)

find_package(Threads)


file(GLOB SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../tinyphone/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../tinyphone/*.cpp
)
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../tinyphone/guicon.cpp)
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../tinyphone/splash.cpp)
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../tinyphone/guicon.h)
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../tinyphone/splash.h)
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/../tinyphone/winapp.cpp)

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
target_include_directories(tinyphone PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../lib/spdlog/include)
link_directories(tinyphone ${Boost_LIBRARY_DIRS})
target_link_libraries(tinyphone ${Boost_LIBRARIES})
target_link_libraries(tinyphone ${PJSIP_LIBRARIES})

set(JSON_Install OFF CACHE INTERNAL "")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../lib/json ${CMAKE_CURRENT_SOURCE_DIR}/../lib/json/build)
target_link_libraries(tinyphone nlohmann_json::nlohmann_json)

add_library(libcryptopp_static STATIC IMPORTED)
set_target_properties(libcryptopp_static PROPERTIES IMPORTED_LOCATION /usr/local/lib/libcryptopp.a)
set_target_properties(libcryptopp_static PROPERTIES INTERFACE_INCLUDE_DIRECTORIES /usr/local/include)
target_link_libraries(tinyphone libcryptopp_static)
target_link_libraries(tinyphone "cryptopp")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../lib/statsd-cpp ${CMAKE_CURRENT_SOURCE_DIR}/../lib/statsd-cpp/build)
target_link_libraries(tinyphone statsd_static)

pkg_check_modules(CURL libcurl REQUIRED)
include_directories(${CURL_INCLUDE_DIR})
target_link_libraries(tinyphone ${CURL_LIBRARIES})


