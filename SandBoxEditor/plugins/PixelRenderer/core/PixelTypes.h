#ifndef PIXELTYPES_H
#define PIXELTYPES_H

#include <QVector>
#include <QtGlobal>

// =====================================================================
// PixelRenderer 模块类型定义（core/）
// 模块内全部数据契约：管线层类型（PixelPrimitiveType/PixelVertex/...）
// + 绘制层类型（PixelColor/DrawCommand）
// 命名约定：模块类型统一以 Pixel 前缀标识归属
// 设计来源：PixelRendererDesign.md 2.3 节
// =====================================================================

// ---------------------------------------------------------------------
// 一、管线层类型（CPU 模拟光栅化管线的数据契约）
// ---------------------------------------------------------------------

// 图元类型（对应 GPU Primitive Topology：点 / 线 / 三角形）
enum class PixelPrimitiveType
{
    Point,     // 点：直接写 1 像素
    Line,      // 线段：Bresenham 整数算法
    Triangle   // 三角形：边界函数 + 重心坐标
};

// 顶点：位置（齐次坐标） + 颜色
struct PixelVertex
{
    float pos[4];     // 位置 x, y, z, w
    float color[4];   // 颜色 r, g, b, a（0..1）
};

// 图元：类型 + 顶点索引（索引指向顶点池）
struct PixelPrimitive
{
    PixelPrimitiveType type;    // 图元类型
    QVector<quint32> indices;   // 顶点索引列表
};

// 模拟片元：CPU 模拟管线产出的像素级结果
// 这是模拟层与呈现层之间唯一的像素数据载体
struct PixelFragment
{
    int x, y;              // 帧缓冲像素坐标（左上角原点，y 向下）
    float color[4];        // RGBA 颜色（各分量 0..1）
    float depth;           // 模拟深度 [0,1]（CPU z-buffer 使用）
    quint32 primitiveId;   // 归属图元索引（教学高亮用）
    quint64 order;         // 全局产出序号（回放/单步用）
};

// ---------------------------------------------------------------------
// 二、绘制层类型（像素图元绘制的数据契约）
// ---------------------------------------------------------------------

// 像素颜色/属性：颜色 + 透明度 + 亮度
struct PixelColor
{
    float r = 1.f, g = 1.f, b = 1.f;   // RGB 颜色（0..1）
    float a = 1.f;                     // 透明度 alpha（0..1，需 GL_BLEND 才可见）
    float brightness = 1.f;            // 亮度（0..1，乘入 RGB；1=原色，0=黑色）

    PixelColor() = default;
    PixelColor(float r_, float g_, float b_, float a_ = 1.f, float brightness_ = 1.f)
        : r(r_), g(g_), b(b_), a(a_), brightness(brightness_) {}
};

// 一次绘制命令：自包含"画什么 + 画到哪 + 画多大"的全部信息
// 参考渲染管线"DrawCall/命令"概念：每条命令独立描述一次绘制
// 注意：这是绘制层原语命令，不是管线提交命令（见 PixelRendererDesign.md 2.3 节）
struct DrawCommand
{
    PixelPrimitiveType type = PixelPrimitiveType::Point;  // 图元类型（点/线，未来三角形）
    int x0 = 0, y0 = 0;      // 起点坐标（点只用起点）
    int x1 = 0, y1 = 0;      // 终点坐标（线用）
    PixelColor color;        // 颜色 / 透明度 / 亮度

    int targetW = 1;         // 渲染目标宽（窗口 / 像素网格尺寸）
    int targetH = 1;         // 渲染目标高

    // 便捷构造
    static DrawCommand makePoint(int x, int y, const PixelColor& c, int w, int h)
    {
        DrawCommand cmd;
        cmd.type = PixelPrimitiveType::Point;
        cmd.x0 = x; cmd.y0 = y;
        cmd.color = c;
        cmd.targetW = w; cmd.targetH = h;
        return cmd;
    }

    static DrawCommand makeLine(int x0, int y0, int x1, int y1,
                                const PixelColor& c, int w, int h)
    {
        DrawCommand cmd;
        cmd.type = PixelPrimitiveType::Line;
        cmd.x0 = x0; cmd.y0 = y0;
        cmd.x1 = x1; cmd.y1 = y1;
        cmd.color = c;
        cmd.targetW = w; cmd.targetH = h;
        return cmd;
    }
};

#endif // PIXELTYPES_H
