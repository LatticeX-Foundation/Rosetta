# chain: (outer) ---> op ---> io ---> common
# tensorflow op libraries
cmake_minimum_required(VERSION 2.8)
project(modules)

# at least one of [gmssl, openssl]
option(USE_GMTASSL "" OFF) # ON OFF
add_definitions(-DUSE_SSL_SOCKET=1)
IF(USE_GMTASSL)
    message(FATAL_ERROR "NOT SUPPORTED GMSSL!")
    add_definitions(-DUSE_GMTASSL=1) # GMSSL
ELSE()
    add_definitions(-DUSE_OPENSSL=1) # OpenSSL
ENDIF()

include(SetThirdParty)
include(common.cmake)
include(protocol.cmake)
include(iowrapper.cmake)
