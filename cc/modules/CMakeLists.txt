# chain: (outer) ---> op ---> io ---> common
# tensorflow op libraries
cmake_minimum_required(VERSION 2.8)
project(modules)

include(SetThirdParty)
include(common.cmake)
include(io.cmake)
include(protocol.cmake)
