# System information
message("---> The System identification is ${CMAKE_SYSTEM} ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION} ${CMAKE_SYSTEM_INFO_FILE}")

# settings
include(SetCommon)
include(SetFunctions)
include(SetPlatformFeatures)
include(SetCompileOptions)
include(SetVersionNumber)
#include(SetThirdParty)
