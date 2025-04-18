cmake_minimum_required(VERSION 3.16)
project(QtSim VERSION 0.1 LANGUAGES CXX)
include(FetchContent)
include(ExternalProject)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable IPO/LTO if available
include(CheckIPOSupported)
check_ipo_supported(RESULT result)
if(result)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# packages
find_package(QT NAMES Qt6 Qt5 REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets Quick Xml OpenGL OpenGLWidgets)


if (EXISTS "${CMAKE_BINARY_DIR}/CMakeCache.txt")
    file(READ "${CMAKE_BINARY_DIR}/CMakeCache.txt" CACHE_CONTENTS)

    # Detect MSVC compiler in a MinGW build
    if (CMAKE_GENERATOR MATCHES "MinGW" AND CACHE_CONTENTS MATCHES "CMAKE_CXX_COMPILER:INTERNAL=.*cl.exe")
        message(WARNING "Detected MSVC toolchain in a MinGW build. Cleaning build directory...")
        execute_process(COMMAND ${CMAKE_COMMAND} --fresh .)
    endif()

    # Detect MinGW compiler in a Visual Studio build
    if (CMAKE_GENERATOR MATCHES "Visual Studio" AND CACHE_CONTENTS MATCHES "CMAKE_CXX_COMPILER:INTERNAL=.*g\\+\\+")
        message(WARNING "Detected MinGW toolchain in a Visual Studio build. Cleaning build directory...")
        execute_process(COMMAND ${CMAKE_COMMAND} --fresh .)
    endif()
endif()

# sets default startup project in Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT QtSim)
if (MSVC)
    add_compile_options(/MP) # Multi-threaded compilation
    add_compile_options(/permissive- /W3 /WX) # Enforce stricter conformance to C++ standard
    add_compile_options(/wd4244 /wd4267 /wd4101 /wd4996)

    #add_compile_options(/O2 /arch:AVX2)
    #add_compile_options(/O2 /arch:AVX2 /fp:fast)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    #add_compile_options(-O3 -mavx2 -march=native)
    #add_compile_options(-O3 -mavx2 -ffast-math -march=native)
endif()

# Linux Locate ffmpeg
if(UNIX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(FFMPEG REQUIRED libavcodec libavformat libavutil libswscale)
endif()

# Define an external directory for caching LLVM
#set(LLVM_CACHE_DIR "${CMAKE_SOURCE_DIR}/_deps/llvm")


# Collect all simulation header files
file(GLOB SIMULATION_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/simulations/*.h")
file(GLOB SIMULATION_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/simulations/*.cpp")
list(SORT SIMULATION_HEADERS)
list(SORT SIMULATION_SOURCES)

file(GLOB CORE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"

    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.h"

    "${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/*.h"

    "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/*.ui"

    "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/*.h"

    #"${CMAKE_CURRENT_SOURCE_DIR}/src/ui/imgui/*.h"
    #"${CMAKE_CURRENT_SOURCE_DIR}/src/ui/imgui/*.cpp"
    #"${CMAKE_CURRENT_SOURCE_DIR}/src/ui/imgui/backends/imgui_impl_opengl3.h"
    #"${CMAKE_CURRENT_SOURCE_DIR}/src/ui/imgui/backends/imgui_impl_opengl3.cpp"
)

file(GLOB PROJECT_SOURCES CONFIGURE_DEPENDS
    ${CORE_SOURCES}
    ${SIMULATION_HEADERS}
    ${SIMULATION_SOURCES}
    "resources.qrc"
)

file(GLOB UI_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/*.ui")

message(STATUS "Setting Source Filters")
source_group("Project" FILES ${CORE_SOURCES})
source_group("Simulations" FILES ${SIMULATION_HEADERS} ${SIMULATION_SOURCES})
source_group("UI" FILES ${UI_SOURCES})

# Remove precompiled pch.cpp from the glob results
#list(FILTER PROJECT_SOURCES EXCLUDE REGEX "src/core/project\\.h$")

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
target_include_directories(QtSim PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/core)
target_include_directories(QtSim PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics)
target_include_directories(QtSim PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/ui)
#target_include_directories(QtSim PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/imgui)
target_include_directories(QtSim PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/widgets)
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

target_precompile_headers(QtSim PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/core/project.h")

# Detect build system
if (CMAKE_GENERATOR MATCHES "Ninja")
    set(BUILD_SYSTEM_NAME "ninja")
elseif (CMAKE_GENERATOR MATCHES "Visual Studio")
    set(BUILD_SYSTEM_NAME "vs")
else()
    set(BUILD_SYSTEM_NAME "other") # Fallback
endif()

# Build dependencies (e.g. qnanopainter) and store outside build/
set(FETCHCONTENT_BASE_DIR ${CMAKE_SOURCE_DIR}/build_fetched)

# FFmpeg
function(add_ffmpeg)
    ## Fetch dlls (windows)
    if (WIN32)
        set(GITHUB_API_URL "https://api.github.com/repos/BtbN/FFmpeg-Builds/releases/latest")
        set(JSON_FILE "${CMAKE_BINARY_DIR}/ffmpeg_release.json")
        set(FFMPEG_ZIP "${CMAKE_CURRENT_SOURCE_DIR}/external/ffmpeg.zip")
        set(FFMPEG_EXTRACT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/ffmpeg_extracted")
        set(FFMPEG_DEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/ffmpeg/windows")

        if (NOT EXISTS "${FFMPEG_DEST_DIR}")
            message(STATUS "FFmpeg not found at: ${FFMPEG_DEST_DIR}")
            message(STATUS "Downloading...")

            # Get latest release JSON from GitHub
            file(DOWNLOAD ${GITHUB_API_URL} ${JSON_FILE} SHOW_PROGRESS)

            # Extract latest shared build URL from JSON
            file(READ ${JSON_FILE} JSON_CONTENT)
            string(REGEX MATCH "\"browser_download_url\": \"([^\"]*-win64-lgpl-shared.zip)\"" _ ${JSON_CONTENT})
            set(FFMPEG_URL "${CMAKE_MATCH_1}")

            if(NOT FFMPEG_URL)
                message(FATAL_ERROR "Could not find the latest FFmpeg shared build download link")
            endif()

            # Download the latest FFmpeg shared DLLs
            file(DOWNLOAD ${FFMPEG_URL} ${FFMPEG_ZIP} SHOW_PROGRESS)

            # Ensure clean extraction directory
            file(REMOVE_RECURSE ${FFMPEG_EXTRACT_DIR})
            file(MAKE_DIRECTORY ${FFMPEG_EXTRACT_DIR})

            # Extract the FFmpeg zip into FFMPEG_EXTRACT_DIR
            message(STATUS "Extracting FFmpeg DLLs...")
            execute_process(
                COMMAND ${CMAKE_COMMAND} -E tar xzf ${FFMPEG_ZIP}
                WORKING_DIRECTORY ${FFMPEG_EXTRACT_DIR}
            )

            # Detect the extracted folder name
            file(GLOB FFMPEG_FOLDERS "${FFMPEG_EXTRACT_DIR}/*win64-lgpl-shared")
            list(LENGTH FFMPEG_FOLDERS NUM_FOLDERS)

            if(NUM_FOLDERS EQUAL 0)
                message(FATAL_ERROR "Could not find extracted FFmpeg folder")
            endif()

            list(GET FFMPEG_FOLDERS 0 FFMPEG_DIR)
            message(STATUS "Detected FFmpeg directory: ${FFMPEG_DIR}")

            # Copy each subfolder (e.g. bin, lib, include) to destination directory
            file(GLOB FFMPEG_SUBDIRS "${FFMPEG_DIR}/*")
            file(MAKE_DIRECTORY ${FFMPEG_DEST_DIR})

            foreach(SUBDIR ${FFMPEG_SUBDIRS})
                get_filename_component(SUBDIR_NAME ${SUBDIR} NAME)
                message(STATUS "Copying dir: ${SUBDIR} to ${FFMPEG_DEST_DIR}")
                file(COPY "${SUBDIR}" DESTINATION "${FFMPEG_DEST_DIR}")
            endforeach()

            message(STATUS "FFmpeg contents copied to ${FFMPEG_DEST_DIR}")
        else()
            message(STATUS "FFmpeg detected: ${FFMPEG_DEST_DIR}")
        endif()

        # Cleanup
        file(REMOVE ${FFMPEG_ZIP})
        file(REMOVE_RECURSE ${FFMPEG_EXTRACT_DIR})
    endif()

    ## Link

    # Paths
    if(WIN32)
        target_include_directories(QtSim PRIVATE ${FFMPEG_DEST_DIR}/include)
        target_link_directories(QtSim PRIVATE ${FFMPEG_DEST_DIR}/lib)
        message(STATUS "FFmpeg Linked: ${FFMPEG_DEST_DIR}")
    elseif(APPLE)
        set(FFMPEG_LIB_DIR ${FFMPEG_DEST_DIR}/ffmpeg/macos/lib)
        target_include_directories(QtSim PRIVATE ${FFMPEG_DEST_DIR}/ffmpeg/macos/include)
        target_link_directories(QtSim PRIVATE ${FFMPEG_DEST_DIR}/ffmpeg/macos/lib)
    elseif(UNIX)
        target_include_directories(QtSim PRIVATE ${FFMPEG_INCLUDE_DIRS})
        target_link_directories(QtSim PRIVATE ${FFMPEG_LIBRARY_DIRS})
    endif()

    # Libs (for symbol definitions)
    if(WIN32)
        target_link_libraries(QtSim PRIVATE
            ${FFMPEG_DEST_DIR}/lib/avcodec.lib
            ${FFMPEG_DEST_DIR}/lib/avformat.lib
            ${FFMPEG_DEST_DIR}/lib/avutil.lib
            ${FFMPEG_DEST_DIR}/lib/swscale.lib
        )
    elseif(UNIX)
        target_link_libraries(QtSim PRIVATE ${FFMPEG_LIBRARIES})
    endif()

    # Dlls
    if(WIN32)
        add_custom_command(TARGET QtSim POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${FFMPEG_DEST_DIR}/bin/avcodec-62.dll"    "${CMAKE_BINARY_DIR}/avcodec-62.dll"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${FFMPEG_DEST_DIR}/bin/avformat-62.dll"   "${CMAKE_BINARY_DIR}/avformat-62.dll"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${FFMPEG_DEST_DIR}/bin/avutil-60.dll"     "${CMAKE_BINARY_DIR}/avutil-60.dll"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${FFMPEG_DEST_DIR}/bin/swscale-9.dll"     "${CMAKE_BINARY_DIR}/swscale-9.dll"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${FFMPEG_DEST_DIR}/bin/swresample-6.dll"  "${CMAKE_BINARY_DIR}/swresample-6.dll"
        )

        find_library(AVCODEC_LIBRARY avcodec PATHS "${DEST_DIR}/bin/avcodec-62.dll")
        find_library(AVFORMAT_LIBRARY avformat PATHS "${FFMPEG_DEST_DIR}/bin/avformat-62.dll")
        find_library(AVUTIL_LIBRARY avutil PATHS "${FFMPEG_DEST_DIR}/bin/avutil-60.dll")
        find_library(SWSCALE_LIBRARY swscale PATHS "${FFMPEG_DEST_DIR}/bin/swscale-9.dll")
        find_library(SWSCALE_LIBRARY swscale PATHS "${FFMPEG_DEST_DIR}/bin/swresample-6.dll")
    endif()
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

function(add_imgui)
    set(IMGUI_BUILD_DIR ${FETCHCONTENT_BASE_DIR}/${BUILD_SYSTEM_NAME}/imgui)

    #FetchContent_Declare(
    #    imgui
    #    GIT_REPOSITORY https://github.com/willmh93/imgui.git
    #    GIT_TAG master
    #    SOURCE_DIR ${IMGUI_BUILD_DIR}/src
    #    BINARY_DIR ${IMGUI_BUILD_DIR}/build
    #    SUBBUILD_DIR ${IMGUI_BUILD_DIR}/subbuild
    #)

    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.91.9
        SOURCE_DIR ${IMGUI_BUILD_DIR}/src
        BINARY_DIR ${IMGUI_BUILD_DIR}/build
        SUBBUILD_DIR ${IMGUI_BUILD_DIR}/subbuild
    )

    FetchContent_Populate(imgui)

    # Add ImGui source files manually
    add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imconfig.h
        ${imgui_SOURCE_DIR}/imgui.h
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_internal.h
        ${imgui_SOURCE_DIR}/imstb_rectpack.h
        ${imgui_SOURCE_DIR}/imstb_textedit.h
        ${imgui_SOURCE_DIR}/imstb_truetype.h
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.h
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    )

    # Add headers only for clarity or IDE support (optional)
    target_sources(imgui PRIVATE
        ${imgui_SOURCE_DIR}/imgui.h
        ${imgui_SOURCE_DIR}/imconfig.h
        ${imgui_SOURCE_DIR}/imgui_internal.h
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.h
    )

    # Make ImGui headers available to targets that link to it
    target_include_directories(imgui PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
    )

    target_link_libraries(QtSim PRIVATE imgui)

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
add_qnanopainter()
add_imgui()



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
