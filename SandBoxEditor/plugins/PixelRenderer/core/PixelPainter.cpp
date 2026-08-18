#include "core/PixelPainter.h"
#include <QOpenGLFunctions>
#include <QOpenGLContext>
#include <QtGlobal>
#include <cmath>

// 静态 GL 函数表辅助（Qt6 中裸 gl 调用须经 QOpenGLFunctions）
static QOpenGLFunctions* gl()
{
    return QOpenGLContext::currentContext()->functions();
}

PixelPainter::PixelPainter()
    : m_vbo(QOpenGLBuffer::VertexBuffer)
{
    // 目标尺寸由 DrawCommand 携带，此处无需初始化网格
}

PixelPainter::~PixelPainter() = default;

void PixelPainter::initialize()
{
    m_vao.create();                 // 申请 VAO 句柄
    m_vao.bind();                   // 设为当前 VAO（属性配置记录进它）
    m_vbo.create();                 // 申请 VBO 句柄
    m_vbo.bind();                   // 设为当前缓冲（属性数据源关联它）
    // 属性0: vec2 位置；属性1: vec4 颜色；stride = 6 float = 24 字节
    gl()->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                                6 * sizeof(float), (void*)0);
    gl()->glEnableVertexAttribArray(0);
    gl()->glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE,
                                6 * sizeof(float), (void*)(2 * sizeof(float)));
    gl()->glEnableVertexAttribArray(1);
    m_vbo.release();                // 配置已存入 VAO，清理当前绑定
    m_vao.release();

    // 开启 alpha 混合，使"透明度"属性在屏幕上可见（src*a + dst*(1-a)）
    gl()->glEnable(GL_BLEND);
    gl()->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void PixelPainter::draw(const DrawCommand& cmd)
{
    // 统一入口：按图元类型分发
    switch (cmd.type)
    {
    case PixelPrimitiveType::Point:    plotPoint(cmd); break;
    case PixelPrimitiveType::Line:     plotLine(cmd);  break;
    case PixelPrimitiveType::Triangle: break;   // 未来扩展：三角形填充
    }
}

void PixelPainter::drawFragment(const PixelFragment& f, int targetW, int targetH)
{
    // 模拟片元 -> 一条点命令（管线产物经此进入绘制层）
    DrawCommand cmd = DrawCommand::makePoint(
        f.x, f.y,
        PixelColor(f.color[0], f.color[1], f.color[2], f.color[3]),
        targetW, targetH);
    draw(cmd);
}

void PixelPainter::appendPixel(int x, int y, const PixelColor& c,
                               int targetW, int targetH)
{
    // 亮度乘入 RGB（0 -> 黑色，1 -> 原色）；透明度独立保留（交给 GL_BLEND）
    const float r = c.r * c.brightness;
    const float g = c.g * c.brightness;
    const float b = c.b * c.brightness;

    // 目标尺寸随命令带入；至少取 1，防除零（NaN）
    const float gw = qMax(1, targetW);
    const float gh = qMax(1, targetH);

    // 像素中心 (x+0.5, y+0.5) -> NDC（含 y 翻转）
    const float ndcX = (x + 0.5f) / gw * 2.f - 1.f;
    const float ndcY = 1.f - (y + 0.5f) / gh * 2.f;

    m_data << ndcX << ndcY << r << g << b << c.a;   // 交错追加 {pos2, rgba4}
}

void PixelPainter::plotPoint(const DrawCommand& cmd)
{
    // 点：直接写 1 像素
    appendPixel(cmd.x0, cmd.y0, cmd.color, cmd.targetW, cmd.targetH);
}

void PixelPainter::plotLine(const DrawCommand& cmd)
{
    // 线：Bresenham 整数算法，逐点调用 appendPixel（画线建立在画点之上）
    int x0 = cmd.x0, y0 = cmd.y0, x1 = cmd.x1, y1 = cmd.y1;
    int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    for (;;)
    {
        appendPixel(x0, y0, cmd.color, cmd.targetW, cmd.targetH);
        if (x0 == x1 && y0 == y1) break;
        const int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void PixelPainter::clear()
{
    m_data.clear();                 // 清空 CPU 侧数据（下次 upload 生效）
}

void PixelPainter::upload()
{
    m_vbo.bind();                   // 上传 CPU 像素数据到 VBO
    m_vbo.allocate(m_data.constData(), m_data.size() * sizeof(float));
    m_vbo.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    m_vbo.release();
}

void PixelPainter::draw()
{
    if (m_data.isEmpty()) return;   // 无像素直接跳过
    m_vao.bind();                   // 绑定 VAO（属性布局已记录）
    gl()->glDrawArrays(GL_POINTS, 0, m_data.size() / 6);   // 批量绘制像素点
    m_vao.release();
}

int PixelPainter::count() const
{
    return m_data.size() / 6;       // 每点 6 个 float
}
