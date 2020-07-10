# chain: (outer) ---> op ---> io ---> common
# tensorflow op libraries
cmake_minimum_required(VERSION 3.10)
project(modules)

include(SetThirdParty)
include(common.cmake)
include(io.cmake)
include(protocol.cmake)
