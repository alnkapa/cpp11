cmake_minimum_required(VERSION 3.25.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

# set(CMAKE_MESSAGE_LOG_LEVEL VERBOSE)
# set(CMAKE_CXX_STANDARD 20)  
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)  

project(Bayan VERSION 0.0.1)

add_executable(join_server main.cpp)

FIND_PACKAGE(Boost REQUIRED)

TARGET_LINK_LIBRARIES(join_server ${Boost_LIBRARIES})

# install(FILES async.h DESTINATION bin)
install(TARGETS join_server DESTINATION bin)


set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "join_server")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION "join_server")
set(CPACK_PACKAGE_CONTACT "my@my.my") 

include(CPack)




