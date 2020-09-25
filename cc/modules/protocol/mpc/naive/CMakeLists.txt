cmake_minimum_required(VERSION 2.8)
project(mpc-naive)

file(GLOB_RECURSE MPC_NAIVE_SOURCES_FILES "src/*.cpp")

# Library mpc-naive
add_library(mpc-naive SHARED ${MPC_NAIVE_SOURCES_FILES})
target_include_directories(mpc-naive PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(mpc-naive PUBLIC ${LINKLIBS} mpc-comm)
set_target_properties(mpc-naive PROPERTIES FOLDER "protocol/naive"
                    APPEND_STRING PROPERTY LINK_FLAGS " ${ADD_LINK_LIB_FLAGS}"
)
install_libraries(mpc-naive)