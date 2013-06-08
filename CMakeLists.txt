cmake_minimum_required(VERSION 2.6)
project(PMBP)

set(CMAKE_BUILD_TYPE "Release")

find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIR})

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR})

find_package(X11 REQUIRED)


IF(APPLE)
include_directories(/opt/X11/include)
ENDIF(APPLE)

include_directories(${PMBP_SOURCE_DIR}/include ${PMBP_SOURCE_DIR}/tools/CImg)

add_executable(pmbp src/colorcode.cc src/graph_2d_flow.cc src/image_operator.cc src/graph_discrete.cc src/graph_particles.cc src/graph_pmbp.cc src/graph_stereo.cc src/image.cc src/image_reader_cimg.cc src/main.cc src/message.cc)


SET(CMAKE_CXX_FLAGS "-std=c++0x -stdlib=libc++")


target_link_libraries(pmbp ${PNG_LIBRARY} ${X11_LIBRARIES})
