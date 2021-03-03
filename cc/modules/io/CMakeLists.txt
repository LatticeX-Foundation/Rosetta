cmake_minimum_required(VERSION 2.8)
SET(LIBNAME mpc-io)
project(${LIBNAME})

# setup directory where we should look for cmake modules files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#3rd
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd)

# threads
find_package(Threads REQUIRED)

SET(ADD_LINK_LIB_FLAGS "-Wl,--rpath=$ORIGIN../:$ORIGIN")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${ADD_LINK_LIB_FLAGS}")

IF(USE_GMTASSL)
    # GMSSL
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd/gmssl/include)
    link_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd/gmssl/lib)
    link_libraries(tacrypto tassl)
ELSE()
    # OpenSSL
    find_package(OpenSSL REQUIRED)
    include_directories(${OPENSSL_INCLUDE_DIR})
    link_libraries(${OPENSSL_LIBRARIES})
ENDIF()


############################### Begin
# libraries
file(GLOB_RECURSE srcs src/*.cpp)
add_library(${LIBNAME} SHARED ${srcs})
target_include_directories(${LIBNAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${LIBNAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../common/include/utils)
target_link_libraries(${LIBNAME} PUBLIC pthread common)
if(USE_LIBEVENT)
target_link_libraries(${LIBNAME} PUBLIC event event_core event_extra event_pthreads event_openssl)
endif()
set_target_properties(${LIBNAME} PROPERTIES FOLDER "io"
    APPEND_STRING PROPERTY LINK_FLAGS " ${ADD_LINK_LIB_FLAGS}")

IF(ROSETTA_COMPILE_TESTS)
# tests
function(compile_tests projname)
    set(proj ${LIBNAME}-tests-${projname})
    add_executable(${proj} ./tests/${projname}.cpp ./tests/test.cpp) 
    target_link_libraries(${proj} ${LIBNAME})
endfunction()
compile_tests(test_net_io)
compile_tests(test_parallel_net_io)
############################### End
ENDIF()
