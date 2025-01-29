cmake_minimum_required(VERSION 3.16)

project(QtSim VERSION 0.1 LANGUAGES CXX)

include(FetchContent)



set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT QtSim)


find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets Quick Xml OpenGL OpenGLWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets Quick Xml OpenGL OpenGLWidgets)

if(UNIX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(FFMPEG REQUIRED libavcodec libavformat libavutil libswscale)
endif()

# Force QNANO_USE_RENDERNODE to be ON
#set(QNANO_USE_RENDERNODE ON CACHE BOOL "Enable QRenderNode in QNanoPainter" FORCE)
#set(IS_QUICK_IN_QT OFF CACHE BOOL "Disable Quick" FORCE)

# Fetch qnanopainter
FetchContent_Declare(
    QNanoPainter
    GIT_REPOSITORY https://github.com/willmh93/qnanopainter
    GIT_TAG master # Replace 'master' with a specific commit/branch/tag if desiredbin
)

#set(IS_QUICK_IN_QT OFF CACHE BOOL "Disable Quick in QNanoPainter" FORCE)

# Make the library available
#FetchContent_MakeAvailable(QNanoPainter)
FetchContent_Populate(QNanoPainter)
add_subdirectory(${qnanopainter_SOURCE_DIR}/libqnanopainter
                 ${qnanopainter_BINARY_DIR}/libqnanopainter)

# Ensure the macro is applied to QNanoPainter
target_compile_definitions(qnanopainter PUBLIC QNANO_USE_RENDERNODE)

# Add FFmpeg paths
if(WIN32)
    set(FFMPEG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/ffmpeg/windows)
    include_directories(${FFMPEG_DIR}/include)
    link_directories(${FFMPEG_DIR}/lib)
    message(STATUS "FFMPEG_DIR: ${FFMPEG_DIR}")
elseif(APPLE)
    #set(FFMPEG_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/ffmpeg/macos/lib)
    #include_directories(${FFMPEG_DIR}/include)
    #link_directories(${FFMPEG_DIR}/lib)
elseif(UNIX)
    include_directories(${FFMPEG_INCLUDE_DIRS})
    link_directories(${FFMPEG_LIBRARY_DIRS})
endif()

# Auto-generate header simulations.h
set(SIMULATIONS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/simulations")  # Replace 'include' with your actual folder
set(GENERATED_SIMULATIONS "${CMAKE_BINARY_DIR}/generated_simulations.h")

# Collect all simulation header files
file(GLOB SIMULATION_HEADERS "${SIMULATIONS_DIR}/*.h")
file(GLOB SIMULATION_SOURCES "${SIMULATIONS_DIR}/*.cpp")

# Create a list of include commands dynamically
set(INCLUDE_COMMANDS)
foreach(HEADER ${SIMULATION_HEADERS})
    get_filename_component(HEADER_NAME ${HEADER} NAME) # Get only the file name
    list(APPEND INCLUDE_COMMANDS COMMAND ${CMAKE_COMMAND} -E echo "#include \\\"simulations/${HEADER_NAME}\\\"" >> ${GENERATED_SIMULATIONS})
endforeach()

# Create the generated header file
add_custom_command(
    OUTPUT ${GENERATED_SIMULATIONS}
    COMMAND ${CMAKE_COMMAND} -E echo "/* Auto-generated header file including all headers from ${SIMULATIONS_DIR} */" > ${GENERATED_SIMULATIONS}
    COMMAND ${CMAKE_COMMAND} -E echo >> ${GENERATED_SIMULATIONS}
    COMMAND ${CMAKE_COMMAND} -E echo "#ifndef GENERATED_SIMULATIONS_H" >> ${GENERATED_SIMULATIONS}
    COMMAND ${CMAKE_COMMAND} -E echo "#define GENERATED_SIMULATIONS_H" >> ${GENERATED_SIMULATIONS}
    COMMAND ${CMAKE_COMMAND} -E echo >> ${GENERATED_SIMULATIONS}
    ${INCLUDE_COMMANDS}  # Dynamically add all include commands
    COMMAND ${CMAKE_COMMAND} -E echo >> ${GENERATED_SIMULATIONS}
    COMMAND ${CMAKE_COMMAND} -E echo "#endif // GENERATED_SIMULATIONS_H" >> ${GENERATED_SIMULATIONS}
    DEPENDS ${SIMULATION_HEADERS}  # Regenerate if any header changes
    COMMENT "Generating ${GENERATED_SIMULATIONS} with includes for all headers in ${SIMULATIONS_DIR}"
)

file(GLOB PROJECT_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/simulations/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/simulations/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.ui"
    "resources.qrc"
    #"${CMAKE_CURRENT_SOURCE_DIR}/src/Options.ui"
    #${UI_FILES}
)

#set(PYTHON_SCRIPTS
#    new_sim.py
#)
#source_group("Scripts" FILES ${PYTHON_SCRIPTS})


source_group("Simulations" FILES
    ${SIMULATION_HEADERS}
    ${SIMULATION_SOURCES}
)

# Custom target to trigger generation
add_custom_target(GenerateSimulationsHeader DEPENDS ${GENERATED_SIMULATIONS})

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(QtSim
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        #${PYTHON_SCRIPTS}
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

# Ensure QtSim depends on the generated header
add_dependencies(QtSim GenerateSimulationsHeader)
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


if(WIN32)
    #set(FFMPEG_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/ffmpeg/windows/bin)
    message(STATUS "QtSim Source File Directory: ${FFMPEG_DIR}/bin/")
    message(STATUS "QtSim Target File Directory: ${CMAKE_BINARY_DIR}")
    add_custom_command(TARGET QtSim POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${FFMPEG_DIR}/bin/avcodec.dll"  "${CMAKE_BINARY_DIR}/avcodec.dll"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${FFMPEG_DIR}/bin/avformat.dll" "${CMAKE_BINARY_DIR}/avformat.dll"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${FFMPEG_DIR}/bin/avutil.dll"   "${CMAKE_BINARY_DIR}/avutil.dll"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${FFMPEG_DIR}/bin/swscale.dll"  "${CMAKE_BINARY_DIR}/swscale.dll"
    )
endif()


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

