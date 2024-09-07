# library/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(async_server)

add_library(async_server async_server_peer.cpp async_server_accept.cpp no_block.cpp block.cpp block_plus.cpp)

set(BOOST_ROOT_PREFIX ${CMAKE_SOURCE_DIR}/lib/boost)

set(CMAKE_PREFIX_PATH ${BOOST_ROOT_PREFIX})

set(Boost_INCLUDE_DIRS ${BOOST_ROOT_PREFIX}/include)

include_directories(${Boost_INCLUDE_DIRS})

# Указание директории с заголовочными файлами
target_include_directories(async_server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


