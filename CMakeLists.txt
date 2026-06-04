cmake_minimum_required(VERSION 3.10)
project(LootMenuKarmaPatch)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DEF:\"${CMAKE_CURRENT_SOURCE_DIR}/exports.def\"")

# Add compile definitions
add_compile_definitions(WIN32 NDEBUG _WINDOWS _USRDLL RUNTIME FALLOUT_VERSION=0x01070030)

# Add the stewei_tweaks_src fose directory as an include directory
include_directories("../stewei_tweaks_src")
include_directories("../stewei_tweaks_src/fose")

# Add forced include for fose_prefix.h
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FI\"fose_common/fose_prefix.h\"")

# Find FOSE source files from stewei_tweaks_src
set(FOSE_COMMON_SOURCES 
    ../stewei_tweaks_src/fose/fose_common/SafeWrite.cpp
)

add_library(${PROJECT_NAME} SHARED 
    main.cpp
    ${FOSE_COMMON_SOURCES}
)

# Fallout 3 mods are 32-bit
set_target_properties(${PROJECT_NAME} PROPERTIES 
    PREFIX "" 
    SUFFIX ".dll"
)

target_link_libraries(${PROJECT_NAME} Psapi)
