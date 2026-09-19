cmake_minimum_required(VERSION 3.20)
project(QrCodeGeneratorCore CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories(include)

set(CORE_SOURCES
    src/BitBuffer.cpp
    src/QrSegment.cpp
    src/QrCode.cpp
    src/QrTopology.cpp
    src/QrShapeRenderer.cpp
    src/QrEyeRenderer.cpp
    src/QrSvgBuilder.cpp
)

add_library(qrcode_core STATIC ${CORE_SOURCES})

enable_testing()
add_executable(test_topology tests/test_topology.cpp)
target_link_libraries(test_topology PRIVATE qrcode_core)
add_test(NAME TopologyTest COMMAND test_topology)

add_executable(test_svg tests/test_svg.cpp)
target_link_libraries(test_svg PRIVATE qrcode_core)
add_test(NAME SvgTest COMMAND test_svg)
