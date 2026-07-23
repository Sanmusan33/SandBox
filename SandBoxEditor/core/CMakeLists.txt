add_library(CoreLibrary OBJECT
    Plugin.h
    PluginManager.h
    PluginManager.cpp
    PluginLoader.h
    PluginLoader.cpp
)

target_link_libraries(CoreLibrary PUBLIC
    Qt6::Widgets
    SandBoxCore
)

target_include_directories(CoreLibrary PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
