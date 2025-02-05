cmake_minimum_required(VERSION 3.16)
project(qnanopainterdep)

include(FetchContent)

set(QNANOPAINTER_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install/QNanoPainter")
set(CMAKE_INSTALL_PREFIX "${QNANOPAINTER_INSTALL_PREFIX}" CACHE PATH "QNanoPainter install path" FORCE)

FetchContent_Declare(
    qnanopainter
    GIT_REPOSITORY https://github.com/willmh93/qnanopainter
    GIT_TAG master
)
FetchContent_MakeAvailable(qnanopainter)

message(STATUS "qnanopainter_SOURCE_DIR: ${qnanopainter_SOURCE_DIR}")

# Install QNanoPainter headers and libraries into the parent project
install(DIRECTORY ${qnanopainter_SOURCE_DIR}/libqnanopainter/ DESTINATION include)
install(TARGETS qnanopainter LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)