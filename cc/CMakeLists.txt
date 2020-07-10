cmake_minimum_required (VERSION 3.10)
project(rosetta)

set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)

# setup directory where we should look for cmake modules files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(GlobalSettings)


# deps (third_party: relic, emp-toolkit)
execute_process(COMMAND bash ${CMAKE_SOURCE_DIR}/install_basic_deps_by_cmake.sh ${PROJECT_BINARY_DIR} ${CMAKE_BUILD_TYPE} ${THIRD_PARTY_DIR} 
                WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
                
option(BUILD_EMP_TOOL "" OFF)
if(BUILD_EMP_TOOL)
execute_process(COMMAND bash ${CMAKE_SOURCE_DIR}/install_all_deps_by_cmake.sh ${PROJECT_BINARY_DIR} ${CMAKE_BUILD_TYPE} ${THIRD_PARTY_DIR} 
                WORKING_DIRECTORY ${PROJECT_BINARY_DIR})
endif()


# modules
add_subdirectory(modules)
add_subdirectory(tf)
