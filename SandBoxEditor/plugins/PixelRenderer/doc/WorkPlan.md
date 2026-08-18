# PixelRenderer 工作文档

> 状态: 骨架已创建，代码待填充
> 关联设计文档: [PixelRendererDesign.md](PixelRendererDesign.md)

---

## 1. 决策记录（已确认）

| 决策项 | 结论 |
|---|---|
| 像素网格尺寸 | 跟随窗口（resizeGL 时更新，NDC 按窗口换算） |
| 像素呈现模式 | 仅 GL_POINTS 点绘制（不做纹理对照） |
| 落地形式 | 独立插件 `plugins/PixelRenderer/`（id: pixel_renderer） |
| 深度测试 | 仅 CPU z-buffer（不接 GL depth test） |

---

## 2. 文件骨架清单

| 文件 | 职责 | 填充状态 |
|---|---|---|
| `CMakeLists.txt` | 构建定义（MODULE + OpenGL 模块链接） | 已完成 |
| `metadata.json` | 插件元数据 | 已完成 |
| `core/PixelTypes.h` | 模块类型（管线层 + 绘制层：PixelPrimitiveType / PixelVertex / PixelFragment / PixelColor / DrawCommand） | 已完成（纯数据） |
| `core/PixelPainter.h/.cpp` | 像素绘制器：GL_POINTS 逐点绘制（VBO/VAO） | 已完成 |
| `core/PixelPipeline.h/.cpp` | CPU 模拟光栅化管线（可暂停迭代器） | 骨架，待填充 |
| `ui/PixelWidget.h/.cpp` | QOpenGLWidget 呈现窗口（shader/paintGL） | 骨架，待填充 |
| `PixelRendererPlugin.h` | 插件入口（Plugin 接口） | 骨架，待填充 |

---

## 3. 填充指引（代码待用户完成）

### 3.1 PixelPainter（像素绘制器，已封装完成）

对外接口（目标尺寸随命令携带，无需 setGridSize）：

- [x] `initialize()`：创建 VAO/VBO + 属性布局 + 开启混合
- [x] `draw(DrawCommand)`：统一绘制入口（一条命令 = 一次绘制，按图元类型分发）
- [x] `drawFragment(f, targetW, targetH)`：接受模拟片元（包装成点命令）
- [x] `upload()`：CPU 像素数据上传 VBO（GL_DYNAMIC_DRAW）
- [x] `draw()`：glDrawArrays(GL_POINTS) 批量绘制
- [x] `clear()`：清空像素
- [x] `count()`：当前像素点数

核心数据结构：

```cpp
struct DrawCommand          // 一次绘制所需全部信息（自包含）
{
    PixelPrimitiveType type;  // 点 / 线（未来三角形）
    int x0, y0;             // 起点坐标（点只用起点）
    int x1, y1;             // 终点坐标（线用）
    PixelColor color;       // 颜色 + 透明度 + 亮度
    int targetW, targetH;   // 渲染目标尺寸（窗口/网格）
};
```

NDC 映射（目标尺寸来自命令）：
```cpp
ndcX = (x + 0.5f) / targetW * 2.f - 1.f;
ndcY = 1.f - (y + 0.5f) / targetH * 2.f;   // y 翻转
```

### 3.2 PixelWidget（对应设计文档 2.5 / 3.2 / 3.5 节）

- [x] `buildShaders()`：编译 GLSL 330 shader（源码见设计文档 3.2 节），绑定属性位置 0/1
- [x] `initializeGL()`：调用 `m_painter.initialize()` + `buildShaders()`，设置 clear 颜色
- [x] `paintGL()`：`m_painter.upload()` -> `m_painter.draw()`；绘制前 `glClear`
- [x] `resizeGL(w,h)`：记录渲染目标尺寸（随 DrawCommand 携带，`setGridSize` 已移除）
- [ ] `stepPixel()`：调用 `m_pipeline->nextFragment()` 产出片元 -> `m_painter.drawFragment()` -> `update()`
- [ ] `runPipeline()`：循环 nextFragment 直至耗尽，逐片元 drawFragment
- [ ] `replayTo(order)`：回放定位（仅改变绘制数量，零重传）
- [ ] `clearScreen()`：`m_painter.clear()` + `update()`

### 3.3 PixelPipeline（对应设计文档 2.4 节）

- [ ] 顶点数据结构 + Stage0 顶点变换（需复用 `PixelMath.h`，拷贝自 PixelCreator 数学库；列向量记号 `v_clip = P * V * M * v_local`）
- [ ] S1 裁剪/剔除、S2 NDC/视口
- [ ] S4 可暂停光栅化迭代器：`nextFragment()` 逐像素产出 `PixelFragment`
- [ ] S5/S6 着色与 CPU z-buffer 深度比较（**z-buffer 初始化为远平面最大深度**，近者覆盖）

### 3.4 PixelRendererPlugin（参照 plugins/PixelCreator/PixelPlugin.h 模式）

- [ ] `id()` -> `"pixel_renderer"`，`displayName()` -> `"Pixel Renderer"`
- [ ] `widget()` 惰性创建 `PixelWidget`，`onActivate/onDeactivate` 显示/隐藏

---

## 4. 实施里程碑

| 阶段 | 内容 | 验证 |
|---|---|---|
| P1 | 骨架编译通过（空 GL 窗口） | 插件可加载，窗口出现 |
| P2 | 像素流 + shader + GL_POINTS（手动写点） | 手动塞入的像素点可见 |
| P3 | 模拟管线 S0-S4 点/线 | 线框以像素点呈现 |
| P4 | 三角形光栅化 + 插值 | 单三角形正确 |
| P5 | CPU z-buffer | 旋转立方体遮挡正确 |
| P6 | 教学 UI（单步/慢放/回放） | 交互走查 |

---

## 5. 依赖说明

- `PixelMath.h`（拷贝自 PixelCreator 数学库，命名空间 PixelMath）：PixelPipeline 顶点变换需要，建议拷贝到 core/ 后引用
- Qt 模块：`Qt6::Widgets` + `Qt6::OpenGLWidgets` + `Qt6::OpenGL`（CMake 已声明）
- 编译验证由用户执行

---

## 6. 下一步工作内容（P3：模拟管线 + 呈现接线）

> 本章节自包含，可作为新一轮对话的独立执行依据。目标：跑通「管线 begin -> nextFragment 逐像素 -> drawFragment -> 上屏」主链路。

### 6.0 当前基线（新对话必读）

| 项 | 状态 |
|---|---|
| 功能阶段 | P2 完成（PixelPainter 画点/线 + PixelWidget 上屏）；P3 未开始 |
| 目录结构 | 五大阶段模块文件夹（application/geometry/rasterize/fragment/output，均只有 README）+ core/ + ui/ + doc/ |
| 编译 | 由用户执行；助手只做静态校验（GetDiagnostics） |
| 已确认决策 | 仅 GL_POINTS 呈现；CPU z-buffer（不接 GL depth test）；目标尺寸随 DrawCommand 携带 |

关键文件（均为当前实际代码）：

| 文件 | 职责 | 现状 |
|---|---|---|
| `core/PixelTypes.h` | PixelPrimitiveType / PixelVertex / PixelPrimitive / PixelFragment（管线层）+ PixelColor / DrawCommand（绘制层） | 完成（纯数据） |
| `core/PixelPainter.h/.cpp` | GL_POINTS 绘制：initialize/draw/drawFragment/clear/upload/draw/count | 完成 |
| `core/PixelPipeline.h/.cpp` | 可暂停光栅化迭代器 | 空壳（全部 TODO，nextFragment 返回 false） |
| `ui/PixelWidget.h/.cpp` | QOpenGLWidget：initializeGL/paintGL 已实现；stepPixel/runPipeline/replayTo/clearScreen 是 TODO | 半完成 |
| `PixelRendererPlugin.h` | 插件入口，惰性创建 PixelWidget | 完成 |

数据契约速查：

```cpp
// core/PixelTypes.h —— 一、管线层类型
enum class PixelPrimitiveType { Point, Line, Triangle };
struct PixelVertex   { float pos[4]; float color[4]; };   // pos = x,y,z,w
struct PixelPrimitive { PixelPrimitiveType type; QVector<quint32> indices; };
struct PixelFragment { int x, y; float color[4]; float depth; quint32 primitiveId; quint64 order; };

// core/PixelTypes.h —— 二、绘制层类型
struct PixelColor { float r,g,b; float a; float brightness; };
struct DrawCommand { PixelPrimitiveType type; int x0,y0,x1,y1; PixelColor color; int targetW, targetH; };

// core/PixelPainter.h（对外）
void draw(const DrawCommand&);
void drawFragment(const PixelFragment&, int targetW, int targetH);
void clear(); void upload(); void draw(); int count() const;
```

### 6.1 任务分解（按依赖顺序）

| 编号 | 任务 | 内容 | 依赖 |
|---|---|---|---|
| T1 | 引入 PixelMath | 拷贝 PixelCreator 的数学库头文件（Vec3/Mat4/perspective/lookAt/transform）为 `core/PixelMath.h`，文件内命名空间同步改为 `PixelMath` | 无 |
| T2 | PixelPipeline 状态设计 | 补齐私有成员（顶点/图元副本、图元游标、Bresenham 状态、order 计数） | 无 |
| T3 | `begin()` 实现 | 深拷贝 verts/prims；复位所有游标；（P3a 阶段输入即屏幕像素坐标，不做 MVP） | T2 |
| T4 | `nextFragment()` 迭代器 | 点：直接产出 1 片元；线：Bresenham 状态机逐像素产出 | T3 |
| T5 | 状态管理 | `isFinished()` / `pendingCount()` / `reset()` | T4 |
| T6 | PixelWidget 接线 | `stepPixel`/`runPipeline`/`clearScreen` 填充；`replayTo` 占位 | T4 |
| T7 | 测试场景 + 清理 | 构造测试 verts/prims，删除 `addTestPixels` 临时代码 | T6 |
| T8 | 构建清单 + 静态校验 | CMakeLists 登记新头文件（如需要）；GetDiagnostics 零错误 | T1-T7 |

> 编译验证（用户执行）安排在 T8 之后。

### 6.2 关键实现要点

**T2 成员设计（建议）**

```cpp
// core/PixelPipeline.h 私有区
QVector<PixelVertex>   m_verts;       // 顶点池副本（begin 深拷贝）
QVector<PixelPrimitive> m_prims;      // 图元列表副本
int      m_primIndex = 0;             // 当前图元索引
quint64  m_order = 0;                 // 全局产出序号（回放用）
bool     m_inPrimitive = false;       // 是否正在遍历某图元
float    m_curColor[4];               // 当前图元着色色（Flat：取图元首顶点颜色）

// 线图元 Bresenham 状态（进入线时初始化）
int m_lx0, m_ly0, m_lx1, m_ly1;       // 线段端点（像素坐标）
int m_curX, m_curY;                   // 当前游标
int m_err, m_dx, m_dy, m_sx, m_sy;    // 累积误差参数

// P5 预留（本批不启用）
// QVector<float> m_zbuf;  int m_zbufW, m_zbufH;
```

**T4 nextFragment 语义**

- 点图元：从 `m_verts[m_prims[m_primIndex].indices[0]]` 取 `pos[0],pos[1]` 为像素坐标、`pos[2]` 为深度、`color` 为颜色，产出后 `m_primIndex++`。
- 线图元：进入时初始化 Bresenham（整数算法，参考 PixelPainter::plotLine 的 err 推进法），每次调用产出当前 `(curX,curY)` 后推进一步，到端点后 `m_primIndex++`。
- 每个片元填充：`primitiveId = m_primIndex`（产出时）、`order = m_order++`。
- 耗尽条件：`m_primIndex >= m_prims.size()` 且不在图元内部，返回 false。

**T6 PixelWidget 接线（模板）**

```cpp
void PixelWidget::stepPixel()
{
    if (!m_pipeline) return;
    PixelFragment f;
    if (!m_pipeline->nextFragment(f)) return;
    m_painter.drawFragment(f, m_targetW, m_targetH);
    update();
}

void PixelWidget::runPipeline()
{
    if (!m_pipeline) return;
    PixelFragment f;
    while (m_pipeline->nextFragment(f))
        m_painter.drawFragment(f, m_targetW, m_targetH);
    update();
}

void PixelWidget::clearScreen()
{
    m_painter.clear();
    update();
}
```

**T7 测试场景（P3a 屏幕空间直出）**

```cpp
// PixelWidget::initializeGL 内，替换 addTestPixels
QVector<PixelVertex> verts;
QVector<PixelPrimitive> prims;
// 顶点：pos[0]=x, pos[1]=y, pos[2]=0.5(深度), pos[3]=1；color=RGBA
verts.append({{100,100,0.5f,1.f}, {1,0,0,1}});
// 图元：点 {Point, {0}}；线 {Line, {1,2}} ...
// m_pipeline->begin(verts, prims);
```

### 6.3 必要项（编码注意事项）

| 类别 | 要求 |
|---|---|
| GL 约束 | OpenGL 调用只允许在 initializeGL/paintGL/resizeGL 内；裸 gl 调用必须经 QOpenGLFunctions（PixelPainter 已封装，勿在 Widget 直接裸调） |
| 除零保护 | 目标尺寸换算一律 `qMax(1, w/h)`（PixelPainter::appendPixel 已有） |
| 亮度换算 | `r * brightness` 在 appendPixel 内完成（已实现） |
| 整数算法 | 线光栅化必须用整数 Bresenham，避免 float 累积误差 |
| 颜色访问 | `PixelFragment.color` 为 `float[4]`，按索引 0..3 |
| 构建 | 新增头文件如未被子目录扫描包含，需登记进 CMakeLists 源文件列表；include 根目录为插件根，路径前缀 `core/`、`ui/` |
| 命名 | 成员 `m_` 前缀；模块类型统一 Pixel 前缀（PixelVertex / PixelFragment 等） |
| 静态校验 | 每次改动后 GetDiagnostics 确认零错误；编译由用户执行 |
| 文档 | 禁止绝对路径、禁止 Unicode 制表符/箭头；代码注释中文 |

### 6.4 验证标准（P3 完成判据）

1. 窗口启动后显示的是**管线产出的**点 + 线（不再是 `addTestPixels` 手写数据）
2. 连续点击单步按钮，每次屏幕新增 1 个像素
3. `isFinished()` 在全部片元产出后返回 true
4. GetDiagnostics 零错误；编译（用户执行）通过

### 6.5 待决策点（进入新对话后先确认）

| 决策 | 选项 | 建议 |
|---|---|---|
| P3 分两步走 | a) 先屏幕空间直出，闭环再补 MVP；b) 一步到位含 MVP 变换 | 推荐 a：先看到管线产出像素，MVP 是纯数学封装，后续加不影响迭代器结构 |
| PixelMath 落地 | 拷贝到 `core/PixelMath.h`（命名空间 PixelMath）；或抽公共位置双插件引用 | 推荐拷贝 core/（插件独立，符合方案 A） |
| `replayTo` 实现 | a) 给 `PixelPainter::draw` 增加 count 参数（零重传）；b) 清空重建前 k 像素 | 推荐 a：数据已在 VBO，回放只改绘制数量 |
| 光照归属 | 几何阶段做 Gouraud 顶点光照；片元阶段做 Flat/Lambert | 已按五阶段划分确认（见 GPUPipelineSimulation.md 5.4） |
