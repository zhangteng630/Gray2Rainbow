cmake_minimum_required(VERSION 3.2)
 
project(Gray2Rainbow)
 
if (CMAKE_COMPILER_IS_GNUCC)
    execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
                    OUTPUT_VARIABLE GCC_VERSION)
    message(STATUS ${GCC_VERSION})
    if (GCC_VERSION VERSION_LESS 5.0)
        message(FATAL_ERROR "GCC version is less than 5.0. Please update.")
    endif()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14")
    
endif()

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -no-pie")

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

aux_source_directory(. SRC_LIST)
add_executable(Gray2Rainbow ${SRC_LIST})
target_link_libraries(Gray2Rainbow ${ITK_LIBRARIES})
