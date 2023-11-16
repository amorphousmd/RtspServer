cmake_minimum_required(VERSION 3.5)
project(test-appsrc)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -lboost_thread-mt -fpermissive")

find_package(PkgConfig REQUIRED)
find_package( Boost REQUIRED timer chrono)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
pkg_check_modules(GST_RTSP_SERVER REQUIRED gstreamer-rtsp-server-1.0)
pkg_check_modules(OPENCV REQUIRED opencv4)

set(CMAKE_CXX_STANDARD 11)

add_executable(main main.cpp RtspServer.cpp)

target_include_directories(main PRIVATE
    ${GSTREAMER_INCLUDE_DIRS}
    ${GST_RTSP_SERVER_INCLUDE_DIRS}
    ${OPENCV_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIR}
)

target_link_libraries(main PRIVATE
    ${GSTREAMER_LIBRARIES}
    ${GST_RTSP_SERVER_LIBRARIES}
    ${OPENCV_LIBRARIES}
    ${Boost_LIBRARIES}
)

# Add flags to suppress warnings
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
endif()
