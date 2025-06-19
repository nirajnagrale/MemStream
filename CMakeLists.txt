cmake_minimum_required(VERSION 3.15)
project(cpp-pipeline VERSION 2.0.0 LANGUAGES CXX)

#–– Build Settings
set(CMAKE_CXX_STANDARD       17)
set(CMAKE_INSTALL_PREFIX     "/usr" CACHE PATH "Install prefix" FORCE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

#–– Dependencies
find_package(nlohmann_json REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(PkgConfig REQUIRED)
pkg_search_module(ZMQ REQUIRED IMPORTED_TARGET libzmq)

#–– Make our headers available
# (assumes your shared headers live in <project-root>/include/)
include_directories(${CMAKE_SOURCE_DIR}/include)

#–– engine (includes config_parser.cpp)
add_executable(cp_engine
  src/engine.cpp
)
target_link_libraries(cp_engine
  PRIVATE
    nlohmann_json::nlohmann_json
    PkgConfig::ZMQ
)
install(TARGETS cp_engine
        RUNTIME DESTINATION bin)

#–– RandomSource node
add_executable(RandomSource
  nodes/RandomSource.cpp
)
target_link_libraries(RandomSource
  PRIVATE
    PkgConfig::ZMQ
)
install(TARGETS RandomSource
        RUNTIME DESTINATION bin)

#–– TCPWebSocket node
add_executable(TcpSocket
  nodes/TcpSocket.cpp
)
target_link_libraries(TcpSocket
  PRIVATE
    Boost::system
    PkgConfig::ZMQ
)
install(TARGETS TcpSocket
        RUNTIME DESTINATION bin)

#–– UDPWebSocket node
add_executable(UdpSocket
  nodes/UdpSocket.cpp
)
target_link_libraries(UdpSocket
  PRIVATE
    Boost::system
    PkgConfig::ZMQ
)
install(TARGETS UdpSocket
        RUNTIME DESTINATION bin)

#–– Config files into /etc/cpp-pipeline
install(DIRECTORY ${CMAKE_SOURCE_DIR}/config/
        DESTINATION /etc/cpp-pipeline
        FILES_MATCHING PATTERN "*.json")

#–– CPack (Debian) Packaging 
set(CPACK_GENERATOR          "DEB")
set(CPACK_PACKAGE_NAME       "cpp-pipeline")
set(CPACK_PACKAGE_VERSION    ${PROJECT_VERSION})
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Niraj Nagrale <niraj6192@gmail.com>")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Minimal in-memory, UNIX-socket style data pipeline")
set(CPACK_DEBIAN_PACKAGE_SECTION     "utils")
include(CPack)
