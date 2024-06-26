cmake_minimum_required(VERSION 3.0)

project(ClockServer)

if(DEFINED ENV{SNAPCRAFT_PROJECT_NAME})
    add_definitions(-DSNAPCRAFT_BUILD=1)
    set(SNAPCRAFT_BUILD YES)
endif()

set(CMAKE_CXX_STANDARD 14)

set(ONVIF_SUPPORT OFF CACHE BOOL "ONVIF support disabled")
add_subdirectory(WebRTSP)

file(GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    *.cpp
    *.h
    *.conf.sample
    README.md
    *.sh
    *.cmake)

file(GLOB_RECURSE SNAP RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    snap/*)

file(GLOB_RECURSE WWW RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    www/[^.]*)

add_executable(${PROJECT_NAME} ${SOURCES} ${SNAP} ${WWW})
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/WebRTSP)
target_link_libraries(${PROJECT_NAME}
    Http
    RtspParser
    Signalling
    RtStreaming)

if(SNAPCRAFT_BUILD)
    install(TARGETS ${PROJECT_NAME} DESTINATION bin)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/www
        DESTINATION /opt/$ENV{SNAPCRAFT_PROJECT_NAME}
        FILES_MATCHING
        PATTERN "www/3rd_party/*.js"
        PATTERN "www/3rd_party/*.css"
        PATTERN "www/WebRTSP/*.mjs"
        PATTERN "www/css/*.css"
        PATTERN "www/index.html"
        PATTERN "www/favicon.ico")
endif()
