#ifndef VIEWPORTWIDGET_H   // 头文件保护宏开始：防止同一头文件被重复包含
#define VIEWPORTWIDGET_H   // 定义保护宏：首次包含后即为"已定义"状态

#include <QWidget>         // 引入 Qt 基础窗口组件（QWidget），作为本类的基类
#include <QTimer>          // 引入 Qt 定时器（QTimer），用于驱动 WASD 连续移动
#include <QSet>            // 引入 Qt 集合容器（QSet），用于记录当前按下的按键
#include "RasterCamera.h"  // 引入相机类，提供视角旋转与移动能力

// 渲染视口：被动 Widget，当前显示占位场景（网格/坐标轴/线框立方体）
// 用于验证相机控制；后续替换为 RasterPipeline 的帧缓冲显示
class ViewportWidget : public QWidget   // 视口窗口类：继承 QWidget 成为可显示的独立组件
{
    Q_OBJECT                // Qt 元对象宏：启用信号槽/元数据机制（MOC 必需，缺它则 connect 失效）
public:
    explicit ViewportWidget(QWidget* parent = nullptr);  // 构造函数：可传入父窗口指针（explicit 防隐式转换）

protected:
    void paintEvent(QPaintEvent* event) override;        // 重绘事件处理：每次需要重绘时绘制场景内容
    void mousePressEvent(QMouseEvent* event) override;   // 鼠标按下事件：左键按下进入拖拽状态
    void mouseMoveEvent(QMouseEvent* event) override;    // 鼠标移动事件：拖拽期间增量旋转视角
    void mouseReleaseEvent(QMouseEvent* event) override; // 鼠标释放事件：左键松开退出拖拽状态
    void keyPressEvent(QKeyEvent* event) override;       // 键盘按下事件：将按键码记录进集合
    void keyReleaseEvent(QKeyEvent* event) override;     // 键盘释放事件：将按键码从集合移除

private slots:
    void onTick(); // WASD 连续移动驱动：定时器每帧回调一次，依据按键集合移动相机

private:
    RasterMath::Mat4 viewProjMatrix() const;   // 计算当前帧的 投影矩阵×视图矩阵 组合（一次调用供多处使用）
    QPointF projectToScreen(const RasterMath::Mat4& vp, const RasterMath::Vec3& world) const;  // 世界坐标投影为屏幕坐标（相机后方返回无效点）

    void drawGrid(QPainter& p, const RasterMath::Mat4& vp);      // 绘制 XZ 平面网格地面
    void drawAxis(QPainter& p, const RasterMath::Mat4& vp);      // 绘制原点处的 RGB 三色坐标轴
    void drawWireframeCube(QPainter& p, const RasterMath::Mat4& vp);  // 绘制单位线框立方体（验证相机）

    RasterCamera m_camera;      // 相机实例：持有相机位置与朝向，供本窗口驱动
    QPoint m_lastMousePos;      // 上次记录的鼠标位置（用于计算拖拽位移增量）
    bool m_dragging = false;    // 拖拽状态标记：左键是否处于按住状态
    QSet<int> m_pressedKeys;    // 当前按下的按键码集合（元素为 Qt::Key 枚举值）
    QTimer m_timer;             // 定时器：以固定频率触发 onTick，实现 WASD 连续移动
};

#endif // VIEWPORTWIDGET_H   // 头文件保护宏结束
