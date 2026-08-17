cmake_minimum_required(VERSION 3.16)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(SandBoxEditor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Core)

# 引用同级 SandBoxCore 静态库
add_subdirectory(${CMAKE_SOURCE_DIR}/../SandBoxCore ${CMAKE_BINARY_DIR}/SandBoxCore)

# UI 框架层
add_subdirectory(core)

# 可视化插件
add_subdirectory(plugins)


set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(SandBoxEditor
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    add_executable(SandBoxEditor ${PROJECT_SOURCES})
endif()

target_link_libraries(SandBoxEditor PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    SandBoxCore
    CoreLibrary
    # OctreeVisualizer
)

if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.SandBoxEditor)
endif()
set_target_properties(SandBoxEditor PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS SandBoxEditor
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# 复制运行时配置文件到构建输出目录
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/plugins.json ${CMAKE_BINARY_DIR}/plugins.json COPYONLY)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(SandBoxEditor)
endif()
add_subdirectory(PixelCreator)