cmake_minimum_required(VERSION 3.15)
project(cpp-pipeline VERSION 1.0.0 LANGUAGES CXX)


# ─── Build Settings 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Install prefix" FORCE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")


# ─── Find Dependencies 
find_package(PkgConfig REQUIRED)
pkg_search_module(FFTW3 REQUIRED IMPORTED_TARGET fftw3)



# engine
add_executable(cp_engine src/engine.cpp)

# link against our JSON parser
find_package(nlohmann_json REQUIRED)
target_link_libraries(cp_engine
 PRIVATE
   nlohmann_json::nlohmann_json
)
install(TARGETS cp_engine
        RUNTIME DESTINATION bin)

# RandomSource node
add_executable(RandomSource nodes/RandomSource.cpp)
install(TARGETS RandomSource
        RUNTIME DESTINATION bin)

# FFTTransform node
add_executable(FFTTransform nodes/FFTTransform.cpp)
target_link_libraries(FFTTransform
  PRIVATE
    PkgConfig::FFTW3
)
install(TARGETS FFTTransform
        RUNTIME DESTINATION bin)

# WebSocketSink node 
add_executable(WebSocketSink nodes/WebSocketSink.cpp)
install(TARGETS WebSocketSink
        RUNTIME DESTINATION bin)

find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(WebSocketSink
  PRIVATE
    Boost::system
)
install(TARGETS WebSocketSink
        RUNTIME DESTINATION bin)

# Config files into /etc/cpp-pipeline directory
install(DIRECTORY ${CMAKE_SOURCE_DIR}/config/
        DESTINATION /etc/cpp-pipeline
        FILES_MATCHING PATTERN "*.json")

# CPack (Debian) Packaging 
set(CPACK_GENERATOR          "DEB")
set(CPACK_PACKAGE_NAME       "cpp-pipeline")
set(CPACK_PACKAGE_VERSION    ${PROJECT_VERSION})
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Niraj Nagrale <niraj6192@gmail.com>")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Minimal in-memory, UNIX-socket style data pipeline")
set(CPACK_DEBIAN_PACKAGE_SECTION     "utils")
set(CPACK_DEBIAN_PACKAGE_DEPENDS     "libfftw3-dev (>= 3.3.8)")
include(CPack)
