cmake_minimum_required(VERSION 3.20)
project(insynic VERSION 1.22 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(QT_NO_DEBUG_OUTPUT)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_MACOSX_RPATH ON)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH OFF)
set(CMAKE_BUILD_RPATH "@executable_path/../Frameworks")

find_package(PkgConfig REQUIRED)

pkg_check_modules(SDL3 REQUIRED sdl3)
pkg_check_modules(AVFORMAT REQUIRED libavformat)
pkg_check_modules(AVCODEC REQUIRED libavcodec)
pkg_check_modules(AVUTIL REQUIRED libavutil)
pkg_check_modules(SWRESAMPLE REQUIRED libswresample)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)

set(CMAKE_PREFIX_PATH "/usr/local/opt/qtbase/lib/cmake" ${CMAKE_PREFIX_PATH})

find_package(Qt6 COMPONENTS Core Widgets Gui Concurrent REQUIRED)

set(SCRCPY_ROOT "${CMAKE_SOURCE_DIR}/scrcpy")
set(SCRCPY_APP_SRC "${SCRCPY_ROOT}/app/src")

include_directories(
    ${SCRCPY_APP_SRC}
    ${CMAKE_SOURCE_DIR}/src/scrcpy_lib
    ${CMAKE_SOURCE_DIR}/src
)

add_subdirectory(src/scrcpy_lib)
add_subdirectory(src/filemanager)
add_subdirectory(src/controlpanel)
add_subdirectory(src/ui)

add_executable(insynic MACOSX_BUNDLE
    src/main.cpp
)

target_link_libraries(insynic PRIVATE
    insynic_ui
    insynic_scrcpy_lib
    insynic_filemanager
    insynic_controlpanel
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
)

set_target_properties(insynic PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.insynic.app
    MACOSX_BUNDLE_BUNDLE_NAME "insynic"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/Info.plist.in"
    MACOSX_BUNDLE_ICON_FILE insynic.icns
)

set(TRANSLATION_TS_FILES
    ${CMAKE_SOURCE_DIR}/translations/insynic_zh.ts
)

add_custom_command(TARGET insynic POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        ${CMAKE_BINARY_DIR}/insynic.app/Contents/Resources/
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${TRANSLATION_TS_FILES}
        ${CMAKE_BINARY_DIR}/insynic.app/Contents/Resources/
    COMMENT "Copying translation files to bundle"
)

add_custom_command(TARGET insynic POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/insynic.icns
        ${CMAKE_BINARY_DIR}/insynic.app/Contents/Resources/
    COMMENT "Copying app icon to bundle"
)



add_custom_command(TARGET insynic POST_BUILD
    COMMAND printf "APPL????" > ${CMAKE_BINARY_DIR}/insynic.app/Contents/PkgInfo
    COMMENT "Creating PkgInfo file"
)

# Bundle adb, scrcpy-server, and scrcpy (for OTG mode) into app Resources
set(SCRCPY_BIN_DIR "${CMAKE_SOURCE_DIR}/../scrcpy-macos-x86_64-v4.0")
if(EXISTS "${SCRCPY_BIN_DIR}/adb")
    add_custom_command(TARGET insynic POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SCRCPY_BIN_DIR}/adb"
            ${CMAKE_BINARY_DIR}/insynic.app/Contents/Resources/
        COMMENT "Copying adb to bundle"
    )
endif()
if(EXISTS "${SCRCPY_BIN_DIR}/scrcpy-server")
    add_custom_command(TARGET insynic POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SCRCPY_BIN_DIR}/scrcpy-server"
            ${CMAKE_BINARY_DIR}/insynic.app/Contents/Resources/
        COMMENT "Copying scrcpy-server to bundle"
    )
endif()
if(EXISTS "${SCRCPY_BIN_DIR}/scrcpy")
    add_custom_command(TARGET insynic POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SCRCPY_BIN_DIR}/scrcpy"
            ${CMAKE_BINARY_DIR}/insynic.app/Contents/Resources/
        COMMENT "Copying scrcpy (for OTG mode) to bundle"
    )
endif()

find_program(MACDEPLOYQT_EXECUTABLE NAMES macdeployqt HINTS "/usr/local/opt/qtbase/bin")
if(MACDEPLOYQT_EXECUTABLE)
    add_custom_command(TARGET insynic POST_BUILD
        COMMAND ${MACDEPLOYQT_EXECUTABLE} ${CMAKE_BINARY_DIR}/insynic.app -verbose=1
        COMMENT "Deploying Qt frameworks with macdeployqt"
    )
endif()
