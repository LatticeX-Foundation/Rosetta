
# default directory
SET(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/third_party)

# header-only
include_directories(${THIRD_PARTY_DIR}) # for catch2, cmdline
include_directories(${THIRD_PARTY_DIR}/eigen)
include_directories(${THIRD_PARTY_DIR}/rapidjson/include)
include_directories(${THIRD_PARTY_DIR}/pybind11/include)
include_directories(${THIRD_PARTY_DIR}/spdlog/include)
include_directories(${THIRD_PARTY_DIR}/emp-toolkit/wizard-zk-fp)
include_directories(${THIRD_PARTY_DIR}/io/include)
