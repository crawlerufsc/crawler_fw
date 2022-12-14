cmake_minimum_required(VERSION 3.9)
project(crawler_control VERSION 1.0.0 DESCRIPTION "crawler hardware control library")

include(GNUInstallDirs)

find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED) 
pkg_check_modules(deps REQUIRED IMPORTED_TARGET glib-2.0)
pkg_search_module(GLIB REQUIRED glib-2.0) 
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
pkg_check_modules(GST_APP REQUIRED gstreamer-app-1.0)
pkg_check_modules(GST_VIDEO REQUIRED gstreamer-video-1.0)
pkg_check_modules(Mosquitto IMPORTED_TARGET libmosquitto REQUIRED)
pkg_check_modules(Mosquittopp IMPORTED_TARGET libmosquittopp REQUIRED)

message(STATUS "OpenCV_INCLUDE_DIRS = ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV_LIBS = ${OpenCV_LIBS}")

include_directories(
    /usr/include/gstreamer-1.0 
    /usr/include/glib-2.0
    /usr/lib/arm-linux-gnueabihf/glib-2.0/include/
    /usr/lib/aarch64-linux-gnu/glib-2.0/include
    /usr/lib/x86_64-linux-gnu/glib-2.0/include
    ${OpenCV_INCLUDE_DIRS}
)

#link_directories(/usr/lib/aarch64-linux-gnu/tegra /usr/lib/aarch64-linux-gnu)


SET(TARGET_H
    include/frame.h
    include/pubsub_client.h
    include/network_stream_reader.h
    include/dataset_stream_reader.h
    include/file_utils.h
    include/gstream_client.h
    include/direct_process_pipeline.h
    include/network_stream_logger.h
)

add_library(crawler_fw SHARED
    src/pubsub_client.cpp
    src/gstream_client.cpp
    src/network_stream_reader.cpp
    src/dataset_stream_reader.cpp
    src/network_stream_logger.cpp
)

set_target_properties(crawler_fw PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

target_link_libraries(
    crawler_fw
    ${GST_APP_LIBRARIES} 
    ${GST_VIDEO_LIBRARIES} 
    PkgConfig::Mosquitto
    PkgConfig::Mosquittopp
    ${CMAKE_THREAD_LIBS_INIT}
    ${OpenCV_LIBS} 
)

target_include_directories(crawler_fw PRIVATE .)

install(TARGETS crawler_fw
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(FILES ${CMAKE_BINARY_DIR}/crawler_fw.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)