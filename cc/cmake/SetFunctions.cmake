FUNCTION(GenOrganizedTree VSPREFORDER SOURCEDIR)
    SET(FTYPES "")
    FOREACH(FTYPE IN ITEMS ${ARGN})
        LIST(APPEND FTYPES ${FTYPE})
    ENDFOREACH()
    FILE(GLOB_RECURSE source_list RELATIVE
        "${SOURCEDIR}" ${FTYPES}
        )
    foreach(source IN LISTS source_list)
        get_filename_component(source_path "${source}" PATH)
        string(REPLACE "/" "\\" vs_source_path "${VSPREFORDER}/${source_path}")
        source_group("${vs_source_path}" FILES "${source}")
    endforeach()
ENDFUNCTION()

# absolute path version
FUNCTION(GenOrganizedTree2 VSPREFORDER SOURCEDIR)
    SET(source_lists "")
    FOREACH(FTYPE IN ITEMS ${ARGN})
        FILE(GLOB_RECURSE source_list ${SOURCEDIR}/${FTYPE})
        LIST(APPEND source_lists ${source_list})
    ENDFOREACH()
    foreach(source IN LISTS source_lists)
        get_filename_component(source_path "${source}" PATH)
        string(REPLACE ${SOURCEDIR} ${VSPREFORDER} relate_path ${source_path})
        string(REPLACE "/" "\\" vs_source_path "${relate_path}")
        source_group("${vs_source_path}" FILES "${source}")
    endforeach()
ENDFUNCTION()


# install libraries
FUNCTION(install_libraries) # install_libraries lib1 lib2 lib3 ...
	INSTALL(TARGETS ${ARGN}
		RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
		LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
		ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
		)
ENDFUNCTION()


# fetch sub module
function(FetchSubModule git_path)
	message(STATUS "git_path: ${git_path}")
    if (EXISTS ${git_path}/.git)
        message(STATUS "Submodule update")

        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY "${git_path}"
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)

        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT} for pb repository, please checkout submodules")
        else()
            message(STATUS "git submodule update --init success, with ${GIT_SUBMOD_RESULT} submodules")
            #add the protobuf as sub-projuect
            add_subdirectory(${git_path}/cmake)
        endif()
    else()
        message(FATAL_ERROR "${CMAKE_BUILD_PATH} cannot find the ${git_path}/.git !")
    endif()
endfunction()


function(link_protobuf)
    message(STATUS "")
    #SET(LIBPROTOBUF protobuf-lite)
    #include_directories(${THIRD_CODE_DIR}/protobuf/src)
    #if(WIN32)
    #    link_libraries(lib${LIBPROTOBUF})
    #else()
    #    link_libraries(debug ${LIBPROTOBUF}d)
    #    link_libraries(optimized ${LIBPROTOBUF})
    #endif()
endfunction()

function(link_gmp)
    if(WIN32)
        if(OT_NP_USE_RELIC_WIN)
            include_directories(${WIN_THIRD_COMPILED_DIR}/gmp/include)
            link_directories(${WIN_THIRD_COMPILED_DIR}/gmp/lib/${PLATDIR})
            link_libraries(gmp)
        endif()
    else()
        # emp or relic needed on linux
        list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
	    find_package(GMP REQUIRED)
        include_directories(${GMP_INCLUDE_DIR})
        link_libraries(${GMP_LIBRARIES})
    endif()
endfunction()

function(link_relic_or_miracl)
    if(NOT OT_NP_USE_MIRACL)
	    message(STATUS "USE RELIC")

        IF(WIN32)
	        IF(OT_NP_USE_RELIC_WIN)
                SET(RELIC_INCLUDE_DIR ${CMAKE_BINARY_DIR}/third-code/relic-win/include 
                    ${THIRD_CODE_DIR}/relic-win/include ${THIRD_CODE_DIR}/relic-win/include/low)
                set(RELIC_LIBRARIES relic_s)
	        ELSE()
		        message(FATAL "Not support relic on Windows !")
	        ENDIF()
        ELSE()
	        # setup directory where we should look for cmake files
	        #list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
	        list(APPEND CMAKE_MODULE_PATH "${EMP_TOOL_DIR}/cmake")
	        find_package(relic) # compile relic should refer to emp-readme github repository (https://github.com/emp-toolkit/emp-readme)
	        #set(RELIC_LIBRARIES relic_s)
        ENDIF()
        include_directories(${RELIC_INCLUDE_DIR})
        link_libraries(${RELIC_LIBRARIES})
        
        # openssl for relic
        find_package(OpenSSL REQUIRED)
        include_directories(${OPENSSL_INCLUDE_DIR})
        link_libraries(${OPENSSL_LIBRARIES})

        # gmp for relic
        if(OT_NP_USE_RELIC_WIN)
            include_directories(${WIN_THIRD_COMPILED_DIR}/gmp/include)
            link_directories(${WIN_THIRD_COMPILED_DIR}/gmp/lib/${PLATDIR})
            link_libraries(gmp)
        else()
	        list(APPEND CMAKE_MODULE_PATH "${EMP_TOOL_DIR}/cmake")
	        find_package(GMP REQUIRED)
            include_directories(${GMP_INCLUDE_DIR})
            link_libraries(${GMP_LIBRARIES})
        endif()
    else()
	    message(STATUS "USE MIRACL")
	    include_directories(${THIRD_CODE_DIR}/miracl/include)
	    link_libraries(miracl)

        if(NOT WIN32)
            # emp also needed on linux
	        list(APPEND CMAKE_MODULE_PATH "${EMP_TOOL_DIR}/cmake")
	        find_package(GMP REQUIRED)
            include_directories(${GMP_INCLUDE_DIR})
            link_libraries(${GMP_LIBRARIES})
        endif()
    endif()
endfunction()

function(link_mpc_jit)
    IF(WIN32)
	    link_directories(${THIRD_CODE_DIR}/jit/lib/win/${PLATDIR})
    ELSE()
	    link_directories(${THIRD_CODE_DIR}/jit/lib)
    ENDIF()
    link_libraries(mpc-jit)
endfunction()

function(support_jsonrpc)
    include_directories(${THIRD_CODE_DIR}/libjson-rpc-cpp/src)
    IF(WIN32)
	    ##include_directories(${WIN_THIRD_COMPILED_DIR}/libjson-rpc-cpp/include)
	    ##link_directories(${WIN_THIRD_COMPILED_DIR}/libjson-rpc-cpp/lib/${PLATDIR})
	    ##
	    ##include_directories(${WIN_THIRD_COMPILED_DIR}/jsoncpp/include)
	    ##link_directories(${WIN_THIRD_COMPILED_DIR}/jsoncpp/lib/${PLATDIR})
        
	    include_directories(${THIRD_CODE_DIR}/libjson-rpc-cpp/src)
	    #link_directories(${WIN_THIRD_COMPILED_DIR}/libjson-rpc-cpp/lib/${PLATDIR})
	    
	    #include_directories(${THIRD_CODE_DIR}/jsoncpp/include)
	    #link_directories(${WIN_THIRD_COMPILED_DIR}/jsoncpp/lib/${PLATDIR})
		
	    include_directories(${WIN_THIRD_COMPILED_DIR}/curl/include)
	    link_directories(${WIN_THIRD_COMPILED_DIR}/curl/lib/${PLATDIR})

	    #link_libraries(libcurl jsoncpp jsonrpccpp-common jsonrpccpp-client)
	    link_libraries(libcurl jsoncpp common client)
    ELSE()
        #find_package(Threads)
        find_package(CURL REQUIRED)
        #find_package(MHD REQUIRED)
        #find_package(Jsoncpp REQUIRED)
        #find_package(libjson-rpc-cpp REQUIRED)
    
	    link_directories(/usr/lib64/c++11)
	    #link_libraries(curl jsoncpp jsonrpccpp-common jsonrpccpp-client jsonrpccpp-server)
        link_libraries(curl jsoncpp common client)
    ENDIF()
endfunction()

function(link_libevent)
    include_directories(${CMAKE_BINARY_DIR}/third-code/libevent/include)
    include_directories(${THIRD_CODE_DIR}/libevent/include)
    include_directories(${THIRD_CODE_DIR}/lev/include)
    link_libraries(event)
endfunction()
