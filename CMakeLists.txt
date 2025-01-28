cmake_minimum_required(VERSION 3.16)

project(QtSim VERSION 0.1 LANGUAGES CXX)

include(FetchContent)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets Xml OpenGL OpenGLWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets Xml OpenGL OpenGLWidgets)

if(UNIX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(FFMPEG REQUIRED libavcodec libavformat libavutil libswscale)
endif()

# Fetch qnanopainter
FetchContent_Declare(
    QNanoPainter
    GIT_REPOSITORY https://github.com/willmh93/qnanopainter
    GIT_TAG master # Replace 'master' with a specific commit/branch/tag if desired
)

# Make the library available
FetchContent_MakeAvailable(QNanoPainter)

# Add FFmpeg paths
if(WIN32)
    set(FFMPEG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/ffmpeg/windows)
    include_directories(${FFMPEG_DIR}/include)
    link_directories(${FFMPEG_DIR}/lib)
elseif(APPLE)
    #set(FFMPEG_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/ffmpeg/macos/lib)
    #include_directories(${FFMPEG_DIR}/include)
    #link_directories(${FFMPEG_DIR}/lib)
elseif(UNIX)
    include_directories(${FFMPEG_INCLUDE_DIRS})
    link_directories(${FFMPEG_LIBRARY_DIRS})
endif()


#set(PROJECT_SOURCES
#        src/main.cpp
#        src/qtsim.cpp
#        src/qtsim.h
#)

#file(GLOB UI_FILES
#    "src/*.ui"
#)

file(GLOB PROJECT_SOURCES
    "src/*.cpp"
    "src/*.h"
    "src/simulations/*.cpp"
    "src/simulations/*.h"
    "src/*.ui"
    "src/Options.ui"
    #${UI_FILES}
)



# Debug output
#message(STATUS "PROJECT_SOURCES: ${PROJECT_SOURCES}")
#message(STATUS "UI_FILES: ${UI_FILES}")

#set(PROJECT_SOURCES
#        src/*.cpp
#        src/*.h
#)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(QtSim
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET QtSim APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(QtSim SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(QtSim
            ${PROJECT_SOURCES}
        )
    endif()
endif()


# Link Qt libraries
target_link_libraries(QtSim PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Xml Qt${QT_VERSION_MAJOR}::OpenGL Qt${QT_VERSION_MAJOR}::OpenGLWidgets)
#target_link_libraries(QtSim PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Link FFmpeg libraries
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

# Link qnanopainter to your target
target_link_libraries(QtSim PRIVATE qnanopainter)


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
install(TARGETS QtSim
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(QtSim)
endif()
