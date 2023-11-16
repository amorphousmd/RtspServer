cmake_minimum_required(VERSION 3.5)
project(test-appsrc)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
pkg_check_modules(GST_RTSP_SERVER REQUIRED gstreamer-rtsp-server-1.0)
pkg_check_modules(OPENCV REQUIRED opencv4)

set(CMAKE_CXX_STANDARD 11)

add_executable(main main.cpp RtspServer.cpp)

target_include_directories(main PRIVATE
    ${GSTREAMER_INCLUDE_DIRS}
    ${GST_RTSP_SERVER_INCLUDE_DIRS}
    ${OPENCV_INCLUDE_DIRS}
)

target_link_libraries(main PRIVATE
    ${GSTREAMER_LIBRARIES}
    ${GST_RTSP_SERVER_LIBRARIES}
    ${OPENCV_LIBRARIES}
)


