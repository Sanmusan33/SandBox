add_library(PixelCreator MODULE    # 声明生成一个 MODULE 类型的库：编译为可动态加载的插件 DLL
    PixelPlugin.h                   # 插件入口头文件（Plugin 接口实现）
    RasterMath.h                    # 轻量数学库头文件（Vec3/Mat4）
    RasterCamera.h                  # 相机类头文件（视角旋转与平移）
    RasterCamera.cpp                # 相机类实现文件
    ViewportWidget.h                # 视口窗口头文件（渲染与交互）
    ViewportWidget.cpp              # 视口窗口实现文件
)

target_link_libraries(PixelCreator PUBLIC   # 为插件目标声明链接的库
    Qt6::Widgets                            # Qt 窗口模块：提供 QWidget/QPainter 等
    CoreLibrary                             # 沙盒核心库：提供 Plugin 基类接口
    SandBoxCore                             # 沙盒数据核心库
)

target_include_directories(PixelCreator PUBLIC  # 声明头文件搜索路径
    ${CMAKE_CURRENT_SOURCE_DIR}                 # 当前源码目录（允许引同目录下私有头文件）
)

# 将插件 DLL 输出到子目录，方便 PluginLoader 按 plugins.json 路径扫描
set_target_properties(PixelCreator PROPERTIES      # 设置目标构建属性
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins/PixelCreator"  # 静态库输出目录（Linux 下 .so 走此路径）
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins/PixelCreator"  # 运行时输出目录（Windows 下 DLL 走此路径）
)
