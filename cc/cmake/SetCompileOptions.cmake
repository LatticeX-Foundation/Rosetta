# C++ standard
set(CMAKE_CXX_STANDARD 11)

# Compiler warnings

# Store origin compile flags
set(CMAKE_C_FLAGS_ORIGIN ${CMAKE_C_FLAGS})
set(CMAKE_CXX_FLAGS_ORIGIN ${CMAKE_CXX_FLAGS})

# Create custom compile flags
set(CMAKE_C_FLAGS_CUSTOM ${CMAKE_C_FLAGS})
set(CMAKE_CXX_FLAGS_CUSTOM ${CMAKE_CXX_FLAGS})

if(MSVC)
  # Set warnings level 4
  set(CMAKE_C_WARNING_LEVEL 4)
  set(CMAKE_CXX_WARNING_LEVEL 4)
  if(CMAKE_C_FLAGS_CUSTOM MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_C_FLAGS_CUSTOM ${CMAKE_C_FLAGS_CUSTOM})
  else()
    set(CMAKE_C_FLAGS_CUSTOM "${CMAKE_C_FLAGS_CUSTOM} /W4")
  endif()
  if(CMAKE_CXX_FLAGS_CUSTOM MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS_CUSTOM ${CMAKE_CXX_FLAGS_CUSTOM})
  else()
    set(CMAKE_CXX_FLAGS_CUSTOM "${CMAKE_CXX_FLAGS_CUSTOM} /W4")
  endif()

  # Make all warnings into errors and increases the number of sections that an object file can contain
  set(CMAKE_C_FLAGS_CUSTOM "${CMAKE_C_FLAGS_CUSTOM} /WX /bigobj")
  set(CMAKE_CXX_FLAGS_CUSTOM "${CMAKE_CXX_FLAGS_CUSTOM} /WX /bigobj")

  # Common compile flags
  # C4100: 'identifier' : unreferenced formal parameter
  # C4250: 'class1' : inherits 'class2::member' via dominance
  set(COMMON_COMPILE_FLAGS "/wd4100 /wd4250")

  # Pedantic compile flags
  set(PEDANTIC_COMPILE_FLAGS ${COMMON_COMPILE_FLAGS})

  # special options
  add_compile_options(/wd4068 /wd4251 /wd4275 /wd4819 /wd4244 /wd4996 /W3 /nologo /EHsc)
else()

  # Make all warnings into errors
  set(CMAKE_C_FLAGS_CUSTOM "${CMAKE_C_FLAGS_CUSTOM} -Wall -Werror")
  set(CMAKE_CXX_FLAGS_CUSTOM "${CMAKE_CXX_FLAGS_CUSTOM} -Wall -Werror")

  # Common compile flags
  set(COMMON_COMPILE_FLAGS "")

  # Pedantic compile flags
  #set(PEDANTIC_COMPILE_FLAGS "${COMMON_COMPILE_FLAGS} -Wshadow -pedantic")

  # below warnings will be solved later
  set(NO_WARN_FLAGS "${NO_WARN_FLAGS} -Wno-error=restrict")
  set(NO_WARN_FLAGS "${NO_WARN_FLAGS} -Wno-error")
  set(NO_WARN_FLAGS "${NO_WARN_FLAGS} -Wno-ignored-attributes")
  #set(NO_WARN_FLAGS "${NO_WARN_FLAGS} -Wno-class-memaccess")
  # set(NO_WARN_FLAGS "${NO_WARN_FLAGS} -Wno-return-type")
  set(NO_WARN_FLAGS "${NO_WARN_FLAGS} -Wno-attributes")
  #set(NO_WARN_FLAGS "${NO_WARN_FLAGS} -Wno-stringop-overflow")
  #set(NO_WARN_FLAGS "${NO_WARN_FLAGS} -Wno-error=pessimizing-move")
  set(NO_WARN_FLAGS "${NO_WARN_FLAGS} -Wno-error=array-bounds")
  set(NO_WARN_FLAGS "${NO_WARN_FLAGS} -Wno-error=format-overflow=")
  set(NO_WARN_FLAGS "${NO_WARN_FLAGS} -Wno-format-overflow")
  set(NO_WARN_FLAGS "${NO_WARN_FLAGS} -Wno-maybe-uninitialized")
  set(NO_WARN_FLAGS "${NO_WARN_FLAGS} -Wno-unused-but-set-variable")
  set(NO_WARN_FLAGS "${NO_WARN_FLAGS} -Wno-unused-variable")
  set(NO_WARN_FLAGS "${NO_WARN_FLAGS} -Wno-unused-function")
  set(NO_WARN_FLAGS "${NO_WARN_FLAGS} -Wno-unused-result")
  set(NO_WARN_FLAGS "${NO_WARN_FLAGS} -Wno-unused-value")
  set(NO_WARN_FLAGS "${NO_WARN_FLAGS} -Wno-sign-compare")
  set(NO_WARN_FLAGS "${NO_WARN_FLAGS} -Wno-deprecated-declarations")
  set(NO_WARN_FLAGS "${NO_WARN_FLAGS} -Wno-int-in-bool-context")
  set(NO_WARN_FLAGS "${NO_WARN_FLAGS} -Wno-unknown-pragmas")
  set(NO_WARN_FLAGS "${NO_WARN_FLAGS} -Wno-parentheses")
  set(NO_WARN_FLAGS "${NO_WARN_FLAGS} -Wno-restrict")
  set(NO_WARN_FLAGS "${NO_WARN_FLAGS} -Wno-invalid-pch")
  IF(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 9.0)
    set(NO_WARN_FLAGS "${NO_WARN_FLAGS} -Wno-error=class-memaccess")
  ENDIF()
  
  IF(USE_OMP)
      set(OMP_FLAGS "${CMAKE_C_FLAGS} -fopenmp")
  ENDIF()
  IF(USE_PERF)
      SET(PG_FLAGS "-pg -ggdb3")
  ENDIF()

  SET(COMM_FLAGS "-pthread -msse4.1 -maes -msse2 -mrdseed -Wall -lpthread -fPIC -mpclmul -fpermissive")
  SET(COMM_FLAGS "-pthread -march=native -mrdseed -Wall -lpthread -fPIC -mpclmul")
  SET(COMM_FLAGS "${COMM_FLAGS} ${OMP_FLAGS} ${PG_FLAGS} ${NO_WARN_FLAGS}")

  SET(COMM_C_FLAGS "${COMM_FLAGS} ${OMP_FLAGS} ${PG_FLAGS} ${NO_WARN_FLAGS}")
  SET(EXTRA_FLAGS "-Wno-reorder -fpermissive") # extra, only for c++
  SET(COMM_CXX_FLAGS "${COMM_C_FLAGS} ${EXTRA_FLAGS}")

  # set custom flags
  set(CMAKE_C_FLAGS_CUSTOM "${CMAKE_C_FLAGS_CUSTOM} ${COMM_C_FLAGS}")
  set(CMAKE_CXX_FLAGS_CUSTOM "${CMAKE_CXX_FLAGS_CUSTOM} ${COMM_CXX_FLAGS}")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${COMM_C_FLAGS} -DDEBUG")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${COMM_C_FLAGS} -DNDEBUG -O2")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${COMM_CXX_FLAGS} -DDEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${COMM_CXX_FLAGS} -DNDEBUG -O2")
endif()

# Update compile flags
set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS_CUSTOM})
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS_CUSTOM})

IF(_GLIBCXX_USE_CXX11_ABI)
  add_definitions(-D_GLIBCXX_USE_CXX11_ABI=1)
ELSE()
  add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
ENDIF()
