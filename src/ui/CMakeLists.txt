set_source_files_properties(macos_menu.mm PROPERTIES COMPILE_FLAGS "-x objective-c++")
set_source_files_properties(qt_nsview.mm PROPERTIES COMPILE_FLAGS "-x objective-c++")

add_library(insynic_ui STATIC
    insynic_mainwindow.cpp
    insynic_filebrowser.cpp
    insynic_settingsdialog.cpp
    insynic_recorddialog.cpp
    insynic_networkdialog.cpp
    insynic_customtranslator.cpp
    insynic_devicewindow.cpp
    insynic_sidepanel.cpp
    insynic_controlbar.cpp
    insynic_devicecontrol.cpp
    insynic_devicesession.cpp
    macos_menu.mm
    qt_nsview.mm
)

target_include_directories(insynic_ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(insynic_ui PUBLIC
    insynic_controlpanel
    insynic_filemanager
    insynic_scrcpy_lib
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Concurrent
)

if(APPLE)
    target_link_libraries(insynic_ui PUBLIC "-framework Carbon" "-framework Cocoa")
endif()
