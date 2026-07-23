add_library(OctreeVisualizer MODULE
    OctreeWidget.h
    OctreeWidget.cpp
    OctreePlugin.h
)

target_link_libraries(OctreeVisualizer PUBLIC
    Qt6::Widgets
    CoreLibrary
    SandBoxCore
)

target_include_directories(OctreeVisualizer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 将插件 DLL 输出到子目录，方便管理和查找
set_target_properties(OctreeVisualizer PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins/OctreeVisualizer"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins/OctreeVisualizer"
)