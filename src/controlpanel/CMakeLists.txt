add_library(insynic_controlpanel STATIC
    insynic_controlpanel.cpp
    insynic_virtualkey.h
    insynic_draggablekey.cpp
    insynic_keyconfigdialog.cpp
    insynic_saveprofiledialog.cpp
    insynic_profilemanager.cpp
    insynic_devicesettingsprofile.cpp
)

target_include_directories(insynic_controlpanel PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(insynic_controlpanel PUBLIC
    insynic_scrcpy_lib
    Qt6::Core
    Qt6::Widgets
)
