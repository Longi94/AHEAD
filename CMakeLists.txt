# author: Till Kolditz (C) 2016 Till.Kolditz@TU-Dresden.de

CMAKE_MINIMUM_REQUIRED(VERSION 3.1 FATAL_ERROR)

IF(POLICY CMP0048)
    CMAKE_POLICY(SET CMP0048 NEW)
    PROJECT(V2ROOT VERSION 0.5 LANGUAGES CXX)
ELSE()
    PROJECT(V2ROOT LANGUAGES CXX)
ENDIF()

MESSAGE("V2 root CMAKE script. Including lib and build:")

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

IF(CMAKE_BUILD_TYPE)
ELSE()
    SET(CMAKE_BUILD_TYPE Release)
ENDIF()

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -Wall -Wextra -Weffc++ -march=native -fexceptions -fnon-call-exceptions")
#SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -DDEBUG -rdynamic -g")
#SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
# see
# http://stackoverflow.com/questions/11437692/how-to-add-a-custom-build-type-to-cmake-targetting-make
# for adding custom build types
#SET(CMAKE_CXX_FLAGS_PROFILEGENERATE "${CMAKE_CXX_FLAGS_PROFILEGENERATE} -O0 -DDEBUG -rdynamic -g -pg -fprofile-generate")
#SET(CMAKE_CXX_FLAGS_PROFILEUSE "${CMAKE_CXX_FLAGS_PROFILEUSE} -O3 -DNDEBUG -fprofile-use")

INCLUDE_DIRECTORIES("${PROJECT_INCLUDE_DIR}")

ADD_SUBDIRECTORY(lib)
ADD_SUBDIRECTORY(tools)