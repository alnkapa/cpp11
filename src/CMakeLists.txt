cmake_minimum_required(VERSION 3.25.1)

set(CMAKE_MESSAGE_LOG_LEVEL VERBOSE)
set(CMAKE_CXX_STANDARD 20)  # Установите стандарт C++20
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # Обязательно использовать указанный стандарт
set(CMAKE_CXX_EXTENSIONS OFF)  # Отключите расширения компилятора (например, GNU)

project(Bayan VERSION 0.0.1)

add_subdirectory(async_server)

add_executable(bayan main.cpp)

set(BOOST_ROOT_PREFIX ${CMAKE_SOURCE_DIR}/lib/boost)

set(CMAKE_PREFIX_PATH ${BOOST_ROOT_PREFIX})

set(Boost_INCLUDE_DIRS ${BOOST_ROOT_PREFIX}/include)

include_directories(${Boost_INCLUDE_DIRS})

target_link_libraries(bayan PRIVATE async_server)

set(CMAKE_BUILD_TYPE Debug)

