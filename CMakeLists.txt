cmake_minimum_required(VERSION 2.8)

set(PROJECT RoboMobile)
project(${PROJECT})

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -std=c++14 -Wall -g")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
endif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")

find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)

set(LIBS ${LIBS}
	pthread
)

aux_source_directory(sources SOURCES)
set(SOURCES ${SOURCES}
	${PROJECT}.cpp
)

include_directories(
	sources
)

link_libraries(${JSONCPP_LIBRARIES})

add_executable(${PROJECT} ${SOURCES})

target_link_libraries(${PROJECT}
	${LIBS}
	${JSONCPP_LIBRARIES}
)


