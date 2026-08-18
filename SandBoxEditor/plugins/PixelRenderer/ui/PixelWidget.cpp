#include "ui/PixelWidget.h"
#include <QDebug>

// =====================================================================
// PixelWidget 实现（骨架，代码待用户填充）
// 填充指引见 doc/WorkPlan.md 3.2 节
// =====================================================================

// 临时测试命令（P2 验证用，验证 GL_POINTS 链路后删除）
static void addTestPixels(PixelPainter& painter, int targetW, int targetH)
{
    // 画点：一条命令 = 一次绘制（坐标 + 颜色/透明度/亮度 + 目标尺寸）
    painter.draw(DrawCommand::makePoint(100, 100, PixelColor(1.f, 0.f, 0.f, 1.f, 1.f),
                                        targetW, targetH));   // 红 全亮
    painter.draw(DrawCommand::makePoint(320, 240, PixelColor(0.f, 1.f, 0.f, 0.5f, 1.f),
                                        targetW, targetH));   // 绿 半透明
    painter.draw(DrawCommand::makePoint(600, 400, PixelColor(0.f, 0.f, 1.f, 1.f, 0.3f),
                                        targetW, targetH));   // 蓝 30% 亮度（暗蓝）

    // 画线：基于画点（内部逐点调用 appendPixel）
    painter.draw(DrawCommand::makeLine(100, 100, 600, 400,
                                       PixelColor(1.f, 1.f, 0.f, 1.f, 1.f),
                                       targetW, targetH));    // 黄斜线
    painter.draw(DrawCommand::makeLine(100, 400, 600, 100,
                                       PixelColor(1.f, 0.f, 1.f, 1.f, 0.7f),
                                       targetW, targetH));    // 品红斜线 70% 亮度
}

// 顶点/片元 shader 源码（设计文档 3.2 节）
static const char* kVertexSrc =
    "#version 330 core\n"
    "layout(location = 0) in vec2 aPos;\n"
    "layout(location = 1) in vec4 aColor;\n"
    "out vec4 vColor;\n"
    "void main() { gl_Position = vec4(aPos, 0.0, 1.0);\n"
    "              gl_PointSize = 1.0; vColor = aColor; }\n";
static const char* kFragSrc =
    "#version 330 core\n"
    "in vec4 vColor;\n"
    "out vec4 fragColor;\n"
    "void main() { fragColor = vColor; }\n";

PixelWidget::PixelWidget(QWidget* parent)
    : QOpenGLWidget(parent)
{
    // TODO(用户): 可选设置最小尺寸 / 焦点策略
    setMinimumSize(1280, 720);
}

PixelWidget::~PixelWidget()
{
    // TODO(用户): 若在析构前需要 makeCurrent 释放 GL 资源，可在此补充
}

void PixelWidget::setPipeline(PixelPipeline* pipeline)
{
    m_pipeline = pipeline;   // 注入管线指针（生命周期由调用方管理）
}

void PixelWidget::stepPixel()
{
    // TODO(用户): 教学单步
    //   1) 若 m_pipeline 为空或已耗尽，直接返回
    //   2) PixelFragment f; m_pipeline->nextFragment(f)
    //   3) m_painter.drawFragment(f, m_targetW, m_targetH)
    //   4) update() 触发重绘
}

void PixelWidget::runPipeline()
{
    // TODO(用户): 一次性运行
    //   循环调用 nextFragment 直至耗尽，逐片元 drawFragment，最后 update()
}

void PixelWidget::replayTo(quint64 order)
{
    // TODO(用户): 回放定位
    //   m_painter 已包含全部像素数据，仅改变绘制数量（零重传）
}

void PixelWidget::clearScreen()
{
    // TODO(用户): m_painter.clear() + update()
}

void PixelWidget::initializeGL()
{
    // 关键：初始化 QOpenGLFunctions 函数指针（否则 glClearColor/glClear 断言崩溃）
    initializeOpenGLFunctions();

    buildShaders();
    m_painter.initialize();
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    addTestPixels(m_painter, m_targetW, m_targetH);   // 临时测试：验证绘制链路（验证后删除）
}

void PixelWidget::paintGL()
{
    // TODO(用户):
    //   1) glClear(GL_COLOR_BUFFER_BIT)
    //   2) m_program.bind()
    //   3) m_painter.upload(); m_painter.draw()
    //   4) m_program.release()
    glClear(GL_COLOR_BUFFER_BIT);
    m_program.bind();
    m_painter.upload();
    m_painter.draw();
    m_program.release();
}

void PixelWidget::resizeGL(int w, int h)
{
    // 记录渲染目标尺寸（供 DrawCommand 携带）
    m_targetW = w;
    m_targetH = h;
    glViewport(0, 0, w, h);
}

void PixelWidget::buildShaders()
{
    // TODO(用户): 编译 GLSL 330 shader
    //   顶点/片元源码见设计文档 3.2 节（vertex.glsl / fragment.glsl）
    //   步骤：
    //     m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, kVertexSrc)
    //     m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, kFragSrc)
    //     m_program.link()

    m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, kVertexSrc);
    m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, kFragSrc);
    if (!m_program.link())
        qWarning() << "Shader link failed:" << m_program.log();   // 黑屏排查关键日志
}
