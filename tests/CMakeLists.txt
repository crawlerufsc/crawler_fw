# require CMake 2.8 or greater
cmake_minimum_required(VERSION 3.5)

project(unittest)

set(CMAKE_CXX_STANDARD 17)
include_directories(${PROJECT_SOURCE_DIR})

find_package(PkgConfig REQUIRED)
pkg_check_modules(deps REQUIRED IMPORTED_TARGET glib-2.0)
pkg_search_module(GLIB REQUIRED glib-2.0) 
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
pkg_check_modules(GST_APP REQUIRED gstreamer-app-1.0)
pkg_check_modules(GST_VIDEO REQUIRED gstreamer-video-1.0)

pkg_check_modules(Mosquitto IMPORTED_TARGET libmosquitto REQUIRED)
pkg_check_modules(Mosquittopp IMPORTED_TARGET libmosquittopp REQUIRED)

find_package(Threads REQUIRED)
find_package(OpenCV REQUIRED)

# Locate GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(/usr/include/gstreamer-1.0)


SET(TARGET_H
  ../include/frame.h
  ../include/pubsub_client.h
  ../include/gstream_client.h
  ../include/network_stream_reader.h
  ../include/dataset_stream_reader.h
  ../include/file_utils.h
  ../include/direct_process_pipeline.h
  ../include/network_stream_logger.h
  test_video_source.h
)

add_executable(
  unittest
  fw_tests.cpp
#  pubsub_client_test.cpp
#  gstream_client_test.cpp
#  network_stream_reader_test.cpp
  network_stream_logger_test.cpp
  ../src/pubsub_client.cpp
  ../src/gstream_client.cpp
  ../src/network_stream_reader.cpp
  ../src/network_stream_logger.cpp

)

if(THREADS_HAVE_PTHREAD_ARG)
  target_compile_options(unittest PUBLIC "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
  target_link_libraries(unittest "${CMAKE_THREAD_LIBS_INIT}")
endif()

# link my-recognition to jetson-inference library
target_link_libraries(unittest 
  PkgConfig::deps 
  PkgConfig::Mosquitto
  PkgConfig::Mosquittopp
  ${GTEST_LIBRARIES} 
  ${GST_APP_LIBRARIES} 
  ${GST_VIDEO_LIBRARIES} 
  ${OpenCV_LIBS}
  ${CMAKE_THREAD_LIBS_INIT}

)