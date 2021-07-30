cmake_minimum_required(VERSION 2.8)
SET(LIBNAME iowrapper)
project(${LIBNAME})

# setup directory where we should look for cmake modules files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# threads
#find_package(Threads REQUIRED)

SET(ADD_LINK_LIB_FLAGS "-Wl,--rpath=$ORIGIN../:$ORIGIN")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${ADD_LINK_LIB_FLAGS}")


############################### Begin
# libraries
file(GLOB_RECURSE srcs src/*.cpp)
add_library(${LIBNAME} SHARED ${srcs})
target_include_directories(${LIBNAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${LIBNAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../common/include/utils)
#target_link_libraries(${LIBNAME} PUBLIC io common protocol-utility)
#target_include_directories(${LIBNAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../thrid_party/spdlog-1.6.1/include)
#target_include_directories(${LIBNAME} PUBLIC ${CMAKE_SOURCE_DIR}/thrid_party/spdlog-1.6.1/include)
target_include_directories(${LIBNAME} PUBLIC ${CMAKE_SOURCE_DIR}/thrid_party/spdlog/include)
add_dependencies(${LIBNAME} io common protocol-utility spdlog)
target_link_libraries(${LIBNAME} PUBLIC io common protocol-utility spdlog)

