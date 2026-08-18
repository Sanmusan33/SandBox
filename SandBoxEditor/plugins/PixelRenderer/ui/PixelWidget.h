#ifndef PIXELWIDGET_H
#define PIXELWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include "core/PixelPainter.h"
#include "core/PixelPipeline.h"
#include "core/PixelTypes.h"

// GL 呈现窗口（ui/）：QOpenGLWidget 子类（被动 Widget）
// 命名为 PixelWidget（而非 Widget）：避免与 Qt 基类 QWidget 重名，含义也更贴合
// "像素呈现窗口"的角色
//
// 职责：组装 管线(core/PixelPipeline) + 绘制器(core/PixelPainter) + shader
//  - initializeGL：编译 shader、初始化绘制器（VBO/VAO）
//  - paintGL    ：上传像素并 glDrawArrays(GL_POINTS)
//  - resizeGL   ：记录渲染目标尺寸（随 DrawCommand 携带）
// 设计来源：PixelRendererDesign.md 2.5 / 3.2 / 3.5 节
//
// 注意（填充时务必遵守）：
//  1. OpenGL 调用只允许在 initializeGL/paintGL/resizeGL 内进行
//  2. paintGL 只做"上传 + 绘制"，不参与管线计算（推拉结合：管线推，此处拉）
class PixelWidget : public QOpenGLWidget , protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit PixelWidget(QWidget* parent = nullptr);   // 构造函数
    ~PixelWidget() override;                           // 析构函数

    void setPipeline(PixelPipeline* pipeline);         // 注入模拟管线（外部拥有，本类不释放）
    void stepPixel();                                  // 单步：管线产出 1 片元并绘制
    void runPipeline();                                // 运行：管线跑完全部像素
    void replayTo(quint64 order);                      // 回放到第 order 个像素（仅改变绘制数量）
    void clearScreen();                                // 清空像素并重绘

protected:
    void initializeGL() override;   // GL 初始化：编译 shader、初始化绘制器
    void paintGL() override;        // 每帧绘制：上传 + 批量点绘制
    void resizeGL(int w, int h) override;  // 窗口尺寸变化：记录渲染目标尺寸

private:
    void buildShaders();            // 编译顶点/片元 shader（GLSL 见设计文档 3.2 节）

    PixelPipeline* m_pipeline = nullptr;   // 注入的模拟管线指针（不拥有）
    PixelPainter m_painter;                // 像素绘制器（core/，封装 GL 样板）
    QOpenGLShaderProgram m_program;        // shader 程序（VAO/VBO 布局见 PixelPainter）
    int m_targetW = 1280;                  // 渲染目标宽（窗口，resizeGL 更新）
    int m_targetH = 720;                   // 渲染目标高（窗口，resizeGL 更新）
};

#endif // PIXELWIDGET_H
