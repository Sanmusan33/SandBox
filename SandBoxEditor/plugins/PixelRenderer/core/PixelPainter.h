#ifndef PIXELPAINTER_H
#define PIXELPAINTER_H

#include "core/PixelTypes.h"
#include <QVector>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

// 像素绘制器（PixelPainter）：封装"绘制像素图元"所需的全部 OpenGL 样板
// 归属层：core/（绘制模块，与管线同层）
//
// 能力分层（核心思路：先画点，再在点上画线）：
//   draw(DrawCommand)    统一入口：一条命令 = 一次绘制（点/线，未来三角形）
//   drawFragment(f,w,h)  便捷：接受管线产出的模拟片元（包装成 Point 命令）
//   upload() + draw()    一次上传 + 批量绘制
//
// 点 + 线 组合即可绘制任意复杂图形（多边形网格 / 线框 / 文字等）。
class PixelPainter
{
public:
    PixelPainter();                       // 构造：仅初始化非 GL 状态
    ~PixelPainter();                      // 析构：释放 GL 资源

    void initialize();                    // 创建 VAO/VBO + 属性布局 + 开启混合（initializeGL 中）

    // ---- 统一绘制入口 ----
    void draw(const DrawCommand& cmd);    // 按命令绘制（点/线，未来三角形）
    void drawFragment(const PixelFragment& f, int targetW, int targetH);  // 便捷：接受模拟片元

    // ---- 缓冲与绘制 ----
    void clear();                         // 清空全部像素
    void upload();                        // CPU 像素数据 -> VBO（GL_DYNAMIC_DRAW）
    void draw();                          // glDrawArrays(GL_POINTS, 0, count)
    int count() const;                    // 当前像素点数

private:
    void appendPixel(int x, int y, const PixelColor& c, int targetW, int targetH);  // 写 1 像素到缓冲
    void plotPoint(const DrawCommand& cmd);   // 点：直接写 1 像素
    void plotLine(const DrawCommand& cmd);    // 线：Bresenham 逐点调 appendPixel

    QVector<float> m_data;          // 交错顶点数据 {ndcX, ndcY, r, g, b, a}，每点 6 个 float
    QOpenGLBuffer m_vbo;            // 顶点缓冲对象
    QOpenGLVertexArrayObject m_vao; // 顶点数组对象
};

#endif // PIXELPAINTER_H
