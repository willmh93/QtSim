cmake_minimum_required(VERSION 3.16)
project(qnanopainterdep)

include(FetchContent)

set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/build")
set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install")  # Install dependencies here



FetchContent_Declare(
    qnanopainter
    GIT_REPOSITORY https://github.com/willmh93/qnanopainter
    GIT_TAG master
    SOURCE_DIR qnanopainter-src
    BINARY_DIR qnanopainter-build
)

FetchContent_Populate(qnanopainter)

message(STATUS "HERE")

# Build & install QNanoPainter

# Set custom build defines here if necessary
#set(QNANO_BUILD_GLES_BACKENDS OFF CACHE BOOL "Set QNANO_BUILD_GLES_BACKENDS - OFF QNanoPainter" FORCE)

add_subdirectory(build/qnanopainter-src qnanopainter-build)
install(DIRECTORY build/qnanopainter-src/libqnanopainter/ DESTINATION include)
install(TARGETS qnanopainter LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)