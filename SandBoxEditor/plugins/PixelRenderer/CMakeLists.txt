# PixelRenderer 插件构建定义
# 注意：需要 Qt 的 OpenGL 模块（QOpenGLWidget 位于 Qt6OpenGLWidgets）
add_library(PixelRenderer MODULE            # MODULE 库：编译为可动态加载的插件 DLL
    PixelRendererPlugin.h                   # 插件入口头文件（根）
    core/PixelTypes.h                       # 模块类型（管线层 + 绘制层，core/）
    core/PixelPipeline.h                    # 模拟管线头文件（core/）
    core/PixelPipeline.cpp                  # 模拟管线实现
    core/PixelPainter.h                     # 像素绘制器头文件（core/）
    core/PixelPainter.cpp                   # 像素绘制器实现
    ui/PixelWidget.h                        # 呈现窗口头文件（ui/）
    ui/PixelWidget.cpp                      # 呈现窗口实现
)

# 必须显式 find_package 加载 Qt6 OpenGL 模块，否则 Qt6::OpenGLWidgets 目标不存在
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS OpenGLWidgets OpenGL)

target_link_libraries(PixelRenderer PUBLIC  # 链接库
    Qt6::Widgets                            # Qt 窗口模块
    Qt6::OpenGLWidgets                      # QOpenGLWidget（核心）
    Qt6::OpenGL                             # OpenGL 函数
    CoreLibrary                             # 沙盒核心库（Plugin 基类）
    SandBoxCore                             # 沙盒数据核心库
)

target_include_directories(PixelRenderer PUBLIC   # 头文件搜索路径
    ${CMAKE_CURRENT_SOURCE_DIR}                   # 插件根目录（含 core/ ui/ 子目录）
)

# 将插件 DLL 输出到子目录，方便 PluginLoader 按 plugins.json 路径扫描
set_target_properties(PixelRenderer PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins/PixelRenderer"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins/PixelRenderer"
)
