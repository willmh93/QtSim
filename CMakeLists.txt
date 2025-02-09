cmake_minimum_required(VERSION 3.16)
project(QtSim VERSION 0.1 LANGUAGES CXX)
include(FetchContent)
include(ExternalProject)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# sets default startup project in Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT QtSim)
if (MSVC)
    add_compile_options(/MP)
endif()

# packages
find_package(QT NAMES Qt6 Qt5 REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets Quick Xml OpenGL OpenGLWidgets)

# Linux Locate ffmpeg
if(UNIX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(FFMPEG REQUIRED libavcodec libavformat libavutil libswscale)
endif()

# Define an external directory for caching LLVM
#set(LLVM_CACHE_DIR "${CMAKE_SOURCE_DIR}/_deps/llvm")


# Manage Source Files
message(STATUS "Auto-generating simulations")
    # Collect all simulation header files
    file(GLOB SIMULATION_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/simulations/*.h")
    file(GLOB SIMULATION_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/simulations/*.cpp")
    file(GLOB PROJECT_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/simulations/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/simulations/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.ui"
        "resources.qrc"
    )

    file(GLOB CORE_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    )

    file(GLOB UI_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.ui")

message(STATUS "Setting Source Filters")
    source_group("Project" FILES ${CORE_SOURCES})
    source_group("Simulations" FILES ${SIMULATION_HEADERS} ${SIMULATION_SOURCES})
    source_group("UI" FILES ${UI_SOURCES})

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    if (WIN32)
        message(STATUS "Setting app.rc")
        set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/resources/app.rc")
        #qt_finalize_executable(QtSim WIN32 src/main.cpp ${app_icon_resource_windows})

        qt_add_executable(QtSim MANUAL_FINALIZATION ${PROJECT_SOURCES} ${app_icon_resource_windows})
    else()
        qt_add_executable(QtSim MANUAL_FINALIZATION ${PROJECT_SOURCES})
    endif()

    # Define target properties for Android with Qt 6 as:
    #    set_property(TARGET QtSim APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
    #                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
    # For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(QtSim SHARED ${PROJECT_SOURCES})
        # Define properties for Android with Qt 5 after find_package() calls as:
        #    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(QtSim ${PROJECT_SOURCES})
    endif()
endif()

target_include_directories(QtSim PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(QtSim PRIVATE ${CMAKE_BINARY_DIR})

# Link Qt libraries
target_link_libraries(QtSim PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::Xml
    Qt${QT_VERSION_MAJOR}::OpenGL
    Qt${QT_VERSION_MAJOR}::OpenGLWidgets
)

# Detect build system
if (CMAKE_GENERATOR MATCHES "Ninja")
    set(BUILD_SYSTEM_NAME "ninja")
elseif (CMAKE_GENERATOR MATCHES "Visual Studio")
    set(BUILD_SYSTEM_NAME "vs")
else()
    set(BUILD_SYSTEM_NAME "other") # Fallback
endif()

# FFmpeg
function(add_ffmpeg)
    # Paths
        if(WIN32)
            set(FFMPEG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/ffmpeg/windows)
            target_include_directories(QtSim PRIVATE ${FFMPEG_DIR}/include)
            target_link_directories(QtSim PRIVATE ${FFMPEG_DIR}/lib)
            message(STATUS "FFMPEG_DIR: ${FFMPEG_DIR}")
        elseif(APPLE)
            #set(FFMPEG_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/ffmpeg/macos/lib)
            target_include_directories(QtSim PRIVATE ${FFMPEG_DIR}/include)
            #target_link_directories(QtSim PRIVATE ${FFMPEG_DIR}/lib)
        elseif(UNIX)
            target_include_directories(QtSim PRIVATE ${FFMPEG_INCLUDE_DIRS})
            target_link_directories(QtSim PRIVATE ${FFMPEG_LIBRARY_DIRS})
        endif()

    # Libs (for symbol definitions)
        if(WIN32)
            target_link_libraries(QtSim PRIVATE
                ${FFMPEG_DIR}/lib/avcodec.lib
                ${FFMPEG_DIR}/lib/avformat.lib
                ${FFMPEG_DIR}/lib/avutil.lib
                ${FFMPEG_DIR}/lib/swscale.lib
            )
        elseif(UNIX)
            target_link_libraries(QtSim PRIVATE ${FFMPEG_LIBRARIES})
        endif()

    # Dlls
        if(WIN32)
            message(STATUS "QtSim Source File Directory: ${FFMPEG_DIR}/bin/")
            message(STATUS "QtSim Target File Directory: ${CMAKE_BINARY_DIR}")
            add_custom_command(TARGET QtSim POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${FFMPEG_DIR}/bin/avcodec-61.dll"    "${CMAKE_BINARY_DIR}/avcodec-61.dll"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${FFMPEG_DIR}/bin/avformat-61.dll"   "${CMAKE_BINARY_DIR}/avformat-61.dll"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${FFMPEG_DIR}/bin/avutil-59.dll"     "${CMAKE_BINARY_DIR}/avutil-59.dll"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${FFMPEG_DIR}/bin/swscale-8.dll"     "${CMAKE_BINARY_DIR}/swscale-8.dll"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${FFMPEG_DIR}/bin/swresample-5.dll"  "${CMAKE_BINARY_DIR}/swresample-5.dll"
            )

            find_library(AVCODEC_LIBRARY avcodec PATHS "${FFMPEG_DIR}/bin/avcodec-61.dll")
            find_library(AVFORMAT_LIBRARY avformat PATHS "${FFMPEG_DIR}/bin/avformat-61.dll")
            find_library(AVUTIL_LIBRARY avutil PATHS "${FFMPEG_DIR}/bin/avutil-59.dll")
            find_library(SWSCALE_LIBRARY swscale PATHS "${FFMPEG_DIR}/bin/swscale-8.dll")
            find_library(SWSCALE_LIBRARY swscale PATHS "${FFMPEG_DIR}/bin/swresample-5.dll")
        endif()
endfunction()

set(FETCHCONTENT_BASE_DIR ${CMAKE_SOURCE_DIR}/build_fetched) # Everything goes inside 'external'

function(fetch_ffmpeg)
    set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

    # Ensure dependencies are fetched before FFmpeg
    FetchContent_Declare(
        zlib
        GIT_REPOSITORY https://github.com/madler/zlib.git
        GIT_TAG v1.2.13 # Latest stable release
    )
    FetchContent_MakeAvailable(zlib)

    FetchContent_Declare(
        bzip2
        GIT_REPOSITORY https://gitlab.com/bzip2/bzip2.git
        GIT_TAG bzip2-1.0.8 # Latest stable release
    )
    FetchContent_MakeAvailable(bzip2)

    # Now let CMake find the built dependencies
    set(ZLIB_INCLUDE_DIR ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})
    set(ZLIB_LIBRARY ${zlib_BINARY_DIR}/libz.a)

    set(BZIP2_INCLUDE_DIR ${bzip2_SOURCE_DIR})
    set(BZIP2_LIBRARY ${bzip2_BINARY_DIR}/libbz2.a)

    # Manually set the found paths to ensure CMake finds them
    set(ZLIB_FOUND TRUE)
    set(BZIP2_FOUND TRUE)


    find_package(FFMPEG REQUIRED)
    target_include_directories(QtSim PRIVATE ${FFMPEG_INCLUDE_DIRS})
    target_link_libraries(QtSim PRIVATE ${FFMPEG_LIBRARIES})
endfunction()

function(add_qnanopainter)

    set(QNANO_BUILD_DIR ${FETCHCONTENT_BASE_DIR}/${BUILD_SYSTEM_NAME}/QNanoPainter)

    FetchContent_Declare(
        QNanoPainter
        GIT_REPOSITORY https://github.com/willmh93/qnanopainter
        GIT_TAG master
        SOURCE_DIR ${QNANO_BUILD_DIR}/src
        BINARY_DIR ${QNANO_BUILD_DIR}/build
        SUBBUILD_DIR ${QNANO_BUILD_DIR}/subbuild
    )
    FetchContent_Populate(QNanoPainter)

    add_subdirectory(${qnanopainter_SOURCE_DIR}/libqnanopainter
                     ${qnanopainter_BINARY_DIR}/libqnanopainter)

    ## Ensure the macro is applied to QNanoPainter
    target_compile_definitions(qnanopainter PUBLIC QNANO_USE_RENDERNODE)

    #FetchContent_MakeAvailable(QNanoPainter)
    target_link_libraries(QtSim PRIVATE qnanopainter)
endfunction()

# QNanoPainter
#function(add_prebuilt_dependencies)
#    set(QNANOPAINTER_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/install/QNanoPainter/include")
#    set(QNANOPAINTER_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/install/QNanoPainter/lib")
#
#    target_include_directories(QtSim PRIVATE ${QNANOPAINTER_INCLUDE_DIR})
#    target_link_libraries(QtSim PRIVATE ${QNANOPAINTER_LIB_DIR}/qnanopainter.lib)
#
#    # Match compiled definitions (removes GLES/gl2.h from included files)
#    target_compile_definitions(QtSim PRIVATE QNANO_QT_GL_INCLUDE)
#endfunction()


# Setup libraries
#add_prebuilt_dependencies()

add_ffmpeg()
#fetch_ffmpeg()

add_qnanopainter()





#add_subdirectory(dependencies)
#include_directories(external/qnanopainter/src)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.QtSim)
endif()
set_target_properties(QtSim PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS QtSim BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(QtSim)
endif()
