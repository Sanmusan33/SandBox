#include "ViewportWidget.h"   // 引入视口窗口类声明

#include <QPainter>           // 引入 Qt 绘图类（QPainter）：负责线段/文字/填充等绘制
#include <QMouseEvent>        // 引入鼠标事件类（QMouseEvent）：获取鼠标按键与位置
#include <QKeyEvent>          // 引入键盘事件类（QKeyEvent）：获取按键码
#include <QtGlobal>           // 引入 Qt 全局工具（qDegreesToRadians / qMax 等）
#include <qnamespace.h>

ViewportWidget::ViewportWidget(QWidget* parent)   // 构造函数实现
    : QWidget(parent)                             // 先调用基类构造函数（传入父窗口）
{
    setMinimumSize(480, 360);                     // 设置窗口最小尺寸，防止缩得过小
    setFocusPolicy(Qt::StrongFocus);              // 设置强焦点策略：点击后能接收键盘事件

    m_timer.setInterval(16);                      // 定时器间隔 16 毫秒（约 60Hz）
    connect(&m_timer, &QTimer::timeout, this, &ViewportWidget::onTick);  // 定时器超时信号连接到 onTick 槽
    m_timer.start();                              // 启动定时器（开始周期性触发）
}

void ViewportWidget::paintEvent(QPaintEvent* /*event*/)   // 重绘事件处理：绘制视口的全部内容
{
    QPainter painter(this);                       // 创建画师对象并绑定到本窗口
    painter.fillRect(rect(), QColor(28, 28, 32)); // 用深灰色填充整个窗口背景

    const RasterMath::Mat4 vp = viewProjMatrix();// 计算当前相机与窗口尺寸下的 投影×视图 矩阵

    drawGrid(painter, vp);                        // 绘制网格地面（XZ 平面）
    drawAxis(painter, vp);                        // 绘制 RGB 坐标轴
    drawWireframeCube(painter, vp);               // 绘制线框立方体

    // 相机状态信息（验证用）
    const RasterMath::Vec3 pos = m_camera.position();  // 获取相机当前位置
    const RasterMath::Vec3 fwd = m_camera.forward();   // 获取相机前向向量
    painter.setPen(Qt::white);                    // 设置白色画笔
    painter.drawText(8, 16, QStringLiteral("eye: (%1, %2, %3)")
        .arg(pos.x, 0, 'f', 2).arg(pos.y, 0, 'f', 2).arg(pos.z, 0, 'f', 2));  // 左上角第 1 行：以 2 位小数显示相机位置
    painter.drawText(8, 32, QStringLiteral("fwd: (%1, %2, %3)")
        .arg(fwd.x, 0, 'f', 2).arg(fwd.y, 0, 'f', 2).arg(fwd.z, 0, 'f', 2));  // 左上角第 2 行：显示视线方向

    painter.setPen(QColor(150, 150, 160));        // 设置灰色画笔
    painter.drawText(8, height() - 8,
        QStringLiteral("左键拖拽旋转视角 | W/S 前后 | A/D 左右 | Q/E 升降"));  // 窗口底部显示操作提示文字
}

RasterMath::Mat4 ViewportWidget::viewProjMatrix() const   // 计算 投影矩阵 × 视图矩阵
{
    const float aspect = width() / float(qMax(1, height()));  // 宽高比（height 至少取 1 防止除零）
    const RasterMath::Mat4 proj = RasterMath::Mat4::perspective(
        qDegreesToRadians(60.f), aspect, 0.1f, 100.f);        // 透视投影：60 度垂直视场角、近/远平面 0.1/100
    return proj * m_camera.viewMatrix();                      // 返回 投影×视图 复合矩阵
}

QPointF ViewportWidget::projectToScreen(const RasterMath::Mat4& vp,
                                        const RasterMath::Vec3& world) const   // 将世界坐标投影为屏幕像素坐标
{
    float cx, cy, cz, cw;                          // 声明裁剪空间四分量（齐次坐标）
    vp.transform(world, cx, cy, cz, cw);           // 执行 投影×视图 齐次变换

    if (cw <= 1e-6f)                               // 若齐次 w 非正（点在相机后方）
        return QPointF(-1e6f, -1e6f);              // 返回一个远离屏幕的无效点（调用方据此跳过绘制）

    const float ndcX = cx / cw;                    // 透视除法：裁剪坐标除以 w 得 NDC X
    const float ndcY = cy / cw;                    // 透视除法：得 NDC Y
    const float sx = (ndcX * 0.5f + 0.5f) * width();   // NDC [-1,1] 映射到窗口 X 像素坐标
    const float sy = (1.f - (ndcY * 0.5f + 0.5f)) * height();  // NDC Y 翻转（屏幕 Y 向下）后映射到窗口 Y 像素
    return QPointF(sx, sy);                        // 返回计算得到的屏幕坐标
}

void ViewportWidget::drawGrid(QPainter& p, const RasterMath::Mat4& vp)   // 绘制网格地面（世界 XZ 平面，y=0）
{
    p.setPen(QColor(70, 70, 80));                  // 设置深灰色画笔（网格线颜色）
    constexpr int kExtent = 10;                    // 网格半边长：从 -10 到 10
    for (int i = -kExtent; i <= kExtent; ++i)      // 遍历所有网格线位置
    {
        const QPointF a = projectToScreen(vp, {float(i), 0.f, float(-kExtent)});  // X 方向线的起点（固定 x=i）
        const QPointF b = projectToScreen(vp, {float(i), 0.f, float(kExtent)});   // X 方向线的终点
        if (a.x() > -1e5f && b.x() > -1e5f)        // 两端点均有效（不在相机后方）才绘制
            p.drawLine(a, b);                      // 绘制沿 X 方向的网格线

        const QPointF c = projectToScreen(vp, {float(-kExtent), 0.f, float(i)});  // Z 方向线的起点（固定 z=i）
        const QPointF d = projectToScreen(vp, {float(kExtent), 0.f, float(i)});   // Z 方向线的终点
        if (c.x() > -1e5f && d.x() > -1e5f)        // 两端点均有效才绘制
            p.drawLine(c, d);                      // 绘制沿 Z 方向的网格线
    }
}

void ViewportWidget::drawAxis(QPainter& p, const RasterMath::Mat4& vp)   // 在世界原点绘制 RGB 三色坐标轴
{
    constexpr float kLen = 2.f;                    // 坐标轴长度（世界单位）
    const QPointF o = projectToScreen(vp, {0.f, 0.f, 0.f});  // 原点投影到屏幕坐标
    if (o.x() <= -1e5f)                            // 原点不可见（在相机后方）
        return;                                    // 整组坐标轴直接跳过不绘制

    p.setPen(QColor(220, 70, 70));   // X 红：设置红色画笔
    p.drawLine(o, projectToScreen(vp, {kLen, 0.f, 0.f}));   // 从原点到 +X 端点绘制 X 轴
    p.setPen(QColor(70, 200, 90));   // Y 绿：设置绿色画笔
    p.drawLine(o, projectToScreen(vp, {0.f, kLen, 0.f}));   // 从原点到 +Y 端点绘制 Y 轴
    p.setPen(QColor(80, 130, 230));  // Z 蓝：设置蓝色画笔
    p.drawLine(o, projectToScreen(vp, {0.f, 0.f, kLen}));   // 从原点到 +Z 端点绘制 Z 轴
}

void ViewportWidget::drawWireframeCube(QPainter& p, const RasterMath::Mat4& vp)   // 绘制单位线框立方体
{
    static const RasterMath::Vec3 kVerts[8] = {   // 立方体 8 个顶点坐标（各分量取 ±1）
        {-1,-1,-1}, {1,-1,-1}, {1,1,-1}, {-1,1,-1},    // 背面 4 个顶点（z = -1）
        {-1,-1, 1}, {1,-1, 1}, {1,1, 1}, {-1,1, 1}     // 正面 4 个顶点（z = +1）
    };
    static const int kEdges[12][2] = {             // 立方体 12 条边的顶点索引对
        {0,1}, {1,3}, {3,2}, {2,0},                // 背面四边形（z=-1）
        {4,5}, {5,7}, {7,6}, {6,4},                // 正面四边形（z=+1）
        {0,4}, {1,5}, {2,6}, {3,7}                 // 连接前后面 8 条棱
    };

    QPointF proj[8];                               // 缓存 8 个顶点的屏幕坐标
    for (int i = 0; i < 8; ++i)                    // 遍历全部顶点
        proj[i] = projectToScreen(vp, kVerts[i]);  // 依次将世界顶点投影为屏幕坐标

    p.setPen(QColor(230, 210, 120));               // 设置暖黄色画笔（线框颜色）
    for (const auto& e : kEdges)                   // 遍历全部 12 条边
    {
        const QPointF& a = proj[e[0]];             // 边起点屏幕坐标
        const QPointF& b = proj[e[1]];             // 边终点屏幕坐标
        if (a.x() > -1e5f && b.x() > -1e5f)        // 两端点均有效才绘制
            p.drawLine(a, b);                      // 绘制该条边
    }
}

void ViewportWidget::mousePressEvent(QMouseEvent* event)   // 鼠标按下事件处理
{
    if (event->button() == Qt::RightButton)         // 仅响应左键
    {
        m_dragging = true;                         // 置拖拽状态为真
        m_lastMousePos = event->pos();             // 记录按下时的鼠标位置
        event->accept();                           // 接受事件（阻止继续传播）
    }
}

void ViewportWidget::mouseMoveEvent(QMouseEvent* event)   // 鼠标移动事件处理
{
    if (m_dragging)                                // 仅在拖拽状态下处理移动
    {
        const QPoint delta = event->pos() - m_lastMousePos;  // 计算本次移动的像素增量
        m_camera.orbit(delta.x(), delta.y());      // 将增量交给相机做视角旋转
        m_lastMousePos = event->pos();             // 更新上次位置基准
        update();                                  // 请求重绘（触发 paintEvent）
        event->accept();                           // 接受事件
    }
}

void ViewportWidget::mouseReleaseEvent(QMouseEvent* event)  // 鼠标释放事件处理
{
    if (event->button() == Qt::RightButton)         // 仅响应左键
        m_dragging = false;                        // 置拖拽状态为假（结束拖拽）
}

void ViewportWidget::keyPressEvent(QKeyEvent* event)   // 键盘按下事件处理
{
    m_pressedKeys.insert(event->key());            // 将按键码插入按键集合（记录为"按住"）
    event->accept();                               // 接受事件
}

void ViewportWidget::keyReleaseEvent(QKeyEvent* event)  // 键盘释放事件处理
{
    m_pressedKeys.remove(event->key());            // 从按键集合移除该按键码（记为"松开"）
    event->accept();                               // 接受事件
}

void ViewportWidget::onTick()                      // 定时器回调：实现 WASD 连续移动
{
    if (m_pressedKeys.isEmpty())                   // 若没有任何按键被按下
        return;                                    // 提前返回（无需移动）

    const float dt = m_timer.interval() / 1000.0f; // 将定时器间隔毫秒数换算为秒（作为帧时间）
    constexpr float kSpeed = 6.0f;                 // 相机移动速度（世界单位/秒）

    if (m_pressedKeys.contains(Qt::Key_W)) m_camera.moveForward( kSpeed * dt);  // 按下 W：沿视线前进
    if (m_pressedKeys.contains(Qt::Key_S)) m_camera.moveForward(-kSpeed * dt);  // 按下 S：沿视线后退
    if (m_pressedKeys.contains(Qt::Key_A)) m_camera.moveRight  (-kSpeed * dt);  // 按下 A：向左平移
    if (m_pressedKeys.contains(Qt::Key_D)) m_camera.moveRight  ( kSpeed * dt);  // 按下 D：向右平移
    if (m_pressedKeys.contains(Qt::Key_Q)) m_camera.moveUp     (-kSpeed * dt);  // 按下 Q：向下降低
    if (m_pressedKeys.contains(Qt::Key_E)) m_camera.moveUp     ( kSpeed * dt);  // 按下 E：向上抬高

    update();                                      // 请求重绘（显示移动后的画面）
}
