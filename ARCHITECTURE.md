# SandBox 架构设计

> 一个基于 Qt 6 的插件式沙盒应用，用于演示和可视化 UE 底层图形学概念（八叉树、剔除算法等）。
> 源码位置：`e:/QTProj/SandBox/`

---

## 1. 总体架构

```
SandBox (解决方案目录)
  │
  ├── SandBoxCore/              # 层 1：算法库（静态库）
  │     依赖: Qt6::Core
  │     产出: SandBoxCore.lib
  │
  └── SandBoxEditor/            # 层 2：宿主应用（可执行文件）
        依赖: Qt6::Widgets + SandBoxCore.lib
        产出: SandBoxEditor.exe
        内部:
          ├── core/             # UI 框架层（插件基类、管理器、加载器）
          └── plugins/          # 具体可视化插件（控件 + Feature 包装器）
```

### 依赖方向（严格单向）

```
SandBoxEditor (exe)
  │
  ├── core/     → 零外部依赖
  ├── plugins/  → 依赖 SandBoxCore (数据源)
  │              依赖 SandBoxEditor::core (Feature 基类)
  └── 整体链接     → SandBoxCore.lib
                     Qt6::Widgets

SandBoxCore (lib)
  └── 仅依赖 Qt6::Core (QVector3D, QColor 等基础类型)
```

---

## 2. SandBoxCore —— 可视化工具库

### 职责

提供底层可复用工具集，供 Editor 中的各个 Plugin 调用。Core 不包含任何业务逻辑，只提供"怎么画"的基础能力。

### 包含内容

| 模块 | 说明 | 位置 |
|------|------|------|
| `PrimitiveRenderer` | 基本图元绘制（线段、矩形、圆、立方体线框、箭头等） | `PrimitiveRenderer.h` |
| `TransformHelper` | 坐标变换、投影、视图矩阵运算 | `TransformHelper.h` |
| `ColorPalette` | 颜色生成与映射（按深度/距离/类型着色） | `ColorPalette.h` |
| `Octree` | 松散八叉树数据结构（TOctree2 语义） | `Octree.h` |
| `GridHelper` | 参考网格与坐标轴绘制辅助 | `GridHelper.h` |
| `PickingHelper` | 鼠标射线与包围盒的相交测试 | `PickingHelper.h` |

### Core 与 Plugin 的交互模式

SandBoxCore 只负责"生成绘制指令"和"提供数据"，不执行任何实际绘制：

```
Plugin::Widget (继承 QWidget)
  │
  ├── paintEvent():
  │     ├── 调用 Core::PrimitiveRenderer → 生成线段/矩形/线框数据
  │     ├── 调用 Core::Octree → 获取节点包围盒列表
  │     └── 上述结果通过 QPainter 执行真正绘制
  │
  ├── mouseEvent():
  │     ├── 调用 Core::PickingHelper → 计算鼠标射线与物体的相交
  │     └── 根据拾取结果更新显示状态
  │
  └── 不直接操作 QPainter 之外的渲染 API
        （绘制指令全部通过 Core 的工具函数生成）
```

---

## 3. SandBoxEditor —— 宿主应用

### 职责

提供插件化的 UI 宿主环境，管理插件的编译期注册和运行时生命周期。

### 3.1 core/ —— UI 框架层（始终编译，不可插件化）

| 文件 | 职责 |
|------|------|
| `Feature.h` | 插件基类（纯虚接口：id, displayName, widget, onActivate, onDeactivate） |
| `FeatureManager.h/cpp` | 运行时管理器。读取 `config/features.json`，管理启用/禁用，发射信号 |
| `PluginLoader.h/cpp` | 编译期注册中心。显示 #include 所有已启用插件的 Feature 头文件，在 `registerAll()` 中实例化 |

### 3.2 plugins/ —— 可视化插件

每个插件是一个独立的子目录，拥有自己的 `CMakeLists.txt`（以 OBJECT 库形式编译）。

| 文件 | 职责 |
|------|------|
| `*Widget.h/cpp` | QWidget 子类，实现 QPainter 绘制与鼠标交互 |
| `*Feature.h` | Feature 子类，包装 Widget 并返回 id/displayName/widget() |

### 3.3 config/ —— 运行时配置

| 文件 | 职责 |
|------|------|
| `features.json` | 存储每个插件的启用/禁用状态，运行时被 FeatureManager 读写 |

### 3.4 构建控制

| 文件 | 职责 |
|------|------|
| `build.cs` | 插件构建清单，CMakeLists.txt 读取它决定编译哪些 plugins/ 子目录 |

---

## 4. 插件开发流程（新增一个插件）

| 步骤 | 涉及工程 | 操作 |
|------|---------|------|
| 1 | `SandBoxCore` | 实现算法/数据结构，暴露 `IVisualizable` 接口 |
| 2 | `SandBoxEditor/plugins/` | 新建子目录，创建 `*Widget` 和 `*Feature` |
| 3 | `SandBoxEditor` | 在 `build.cs` 添加一行插件名 |
| 4 | `SandBoxEditor/core/` | 在 `PluginLoader.cpp` 加 `#include` + `registerFeature()` |
| 5 | `SandBoxEditor/config/` | 在 `features.json` 加 id 和默认启用状态 |

**不需要修改的文件**：`main.cpp`、`MainWindow`、`FeatureManager`、根 `CMakeLists.txt`。

---

## 5. 物理目录树

```
SandBox/
  │
  ├── CMakeLists.txt                    # (可选) 根构建，同时 build 两个子项目
  │
  ├── SandBoxCore/
  │   ├── CMakeLists.txt                # STATIC 库
  │   ├── include/SandBoxCore/          # 公共头文件
  │   │     ├── SandBoxCore_global.h    # 导出宏（DLL 时用；STATIC 可移除）
  │   │     ├── IVisualizable.h
  │   │     └── Octree.h
  │   └── src/
  │         ├── Octree.cpp
  │         └── sandboxcore.cpp         # Qt 模板生成的空实现，可删除
  │
  └── SandBoxEditor/
        ├── CMakeLists.txt              # EXE，链接 SandBoxCore + Qt6::Widgets
        ├── build.cs                    # 插件构建清单
        ├── main.cpp
        ├── mainwindow.h/cpp/.ui
        ├── core/                       # UI 框架（始终编译）
        │     ├── CMakeLists.txt        # OBJECT 库
        │     ├── Feature.h
        │     ├── FeatureManager.h/cpp
        │     └── PluginLoader.h/cpp
        ├── plugins/                    # 可视化插件（按 build.cs 选择性编译）
        │     ├── CMakeLists.txt        # 遍历子目录
        │     └── OctreeVisualizer/
        │           ├── CMakeLists.txt  # OBJECT 库
        │           ├── OctreeWidget.h/cpp
        │           └── OctreeFeature.h
        └── config/
              └── features.json
```

---

## 6. 各层依赖明细

| 代码单元 | 编译方式 | 可被复用 | 是否有 UI |
|---------|---------|---------|----------|
| `SandBoxCore` | STATIC 库 | 任何 C++ 项目 | 否 |
| `SandBoxEditor::core` | OBJECT 库 | 仅 SandBoxEditor 内部 | 否（纯逻辑） |
| `SandBoxEditor::plugins::*` | OBJECT 库 | 仅 SandBoxEditor 内部 | 是 |
| `SandBoxEditor` (exe) | 可执行文件 | — | 是 |
