add_library(insynic_filemanager STATIC
    insynic_filemanager.cpp
)

target_include_directories(insynic_filemanager PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(insynic_filemanager PUBLIC
    Qt6::Core
    Qt6::Widgets
)
