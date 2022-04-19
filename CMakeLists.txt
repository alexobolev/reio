cmake_minimum_required(VERSION 3.20)
project(reio VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(REIO_BUILD_TESTS "Build and run reio tests" ON)


set(REIO_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(REIO_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)

set(REIO_SOURCES
        ${REIO_INCLUDE_DIR}/reio/allocators.hpp
        ${REIO_INCLUDE_DIR}/reio/asserts.hpp
        ${REIO_INCLUDE_DIR}/reio/types.hpp
        ${REIO_INCLUDE_DIR}/reio/buffers/weak_buffer.hpp
        ${REIO_INCLUDE_DIR}/reio/buffers/owning_buffer.hpp

        ${REIO_SOURCE_DIR}/allocators.cpp
        ${REIO_SOURCE_DIR}/buffers/owning_buffer.cpp
        )


add_library(reio                    STATIC ${REIO_SOURCES})
set_target_properties(reio          PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(reio     PUBLIC ${REIO_INCLUDE_DIR})


if (REIO_BUILD_TESTS)

    add_executable(reio_tests               tests/main.cpp tests/all.cpp)
    target_link_libraries(reio_tests        PRIVATE reio)
    target_include_directories(reio_tests   PRIVATE ${REIO_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/external)

    list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/external/Catch2/extras)
    include(CTest)
    include(Catch)
    catch_discover_tests(reio_tests)

endif()
