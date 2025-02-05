cmake_minimum_required(VERSION 3.16)
project(QtSim VERSION 0.1 LANGUAGES CXX)
include(FetchContent)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# sets default startup project in Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT QtSim)

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
    qt_add_executable(QtSim MANUAL_FINALIZATION ${PROJECT_SOURCES})
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

# QNanoPainter
function(add_prebuilt_dependencies)
    set(QNANOPAINTER_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/install/QNanoPainter/include")
    set(QNANOPAINTER_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/install/QNanoPainter/lib")

    target_include_directories(QtSim PRIVATE ${QNANOPAINTER_INCLUDE_DIR})
    target_link_libraries(QtSim PRIVATE ${QNANOPAINTER_LIB_DIR}/qnanopainter.lib)

    # Match compiled definitions (removes GLES/gl2.h from included files)
    target_compile_definitions(QtSim PRIVATE QNANO_QT_GL_INCLUDE)
endfunction()

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
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${FFMPEG_DIR}/bin/avcodec.dll"  "${CMAKE_BINARY_DIR}/avcodec.dll"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${FFMPEG_DIR}/bin/avformat.dll" "${CMAKE_BINARY_DIR}/avformat.dll"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${FFMPEG_DIR}/bin/avutil.dll"   "${CMAKE_BINARY_DIR}/avutil.dll"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${FFMPEG_DIR}/bin/swscale.dll"  "${CMAKE_BINARY_DIR}/swscale.dll"
            )

            find_library(AVCODEC_LIBRARY avcodec PATHS "${FFMPEG_DIR}/bin/avcodec.dll")
            find_library(AVFORMAT_LIBRARY avformat PATHS "${FFMPEG_DIR}/bin/avformat.dll")
            find_library(AVUTIL_LIBRARY avutil PATHS "${FFMPEG_DIR}/bin/avutil.dll")
            find_library(SWSCALE_LIBRARY swscale PATHS "${FFMPEG_DIR}/bin/swscale.dll")
        endif()
endfunction()

# Setup libraries
add_prebuilt_dependencies()
add_ffmpeg()

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
