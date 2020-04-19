cmake_minimum_required (VERSION 3.10)
SET(LIBNAME mpc-io)
project(${LIBNAME})

# setup directory where we should look for cmake modules files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(GlobalSettings)

#3rd
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd)

# threads
find_package(Threads REQUIRED)


# OpenSSL
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
link_libraries(${OPENSSL_LIBRARIES})

############################### Begin
# libraries
file(GLOB_RECURSE srcs src/*.cpp)
add_library(${LIBNAME} SHARED ${srcs})
target_include_directories(${LIBNAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${LIBNAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../common/include/utils)
target_link_libraries(${LIBNAME} PUBLIC pthread)
if(USE_LIBEVENT)
target_link_libraries(${LIBNAME} PUBLIC event event_core event_extra event_pthreads event_openssl)
endif()

# examples
function(compile_examples projname)
    set(proj ${LIBNAME}-examples-${projname})
    add_executable(${proj} ./examples/${projname}.cpp) 
    target_link_libraries(${proj} ${LIBNAME})
endfunction()
compile_examples(net_broadcast)
compile_examples(net_interactive)
compile_examples(net_multiclients)
compile_examples(net_sync)
compile_examples(net_msgid)
