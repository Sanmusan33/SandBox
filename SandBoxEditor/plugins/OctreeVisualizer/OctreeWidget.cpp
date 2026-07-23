#include "OctreeWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>

OctreeWidget::OctreeWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumSize(400, 300);
}

void OctreeWidget::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);

    // 背景
    painter.fillRect(rect(), QColor(30, 30, 30));

    // 占位：显示插件名称
    painter.setPen(Qt::white);
    painter.drawText(rect(), Qt::AlignCenter,
        QStringLiteral("Octree Visualizer\n(实现中...)"));
}

void OctreeWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_dragging = true;
        m_lastMousePos = event->pos();
    }
}

void OctreeWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (m_dragging)
    {
        QPoint delta = event->pos() - m_lastMousePos;
        m_offset += QPointF(delta.x(), delta.y());
        m_lastMousePos = event->pos();
        update();
    }
}

void OctreeWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
        m_dragging = false;
}

void OctreeWidget::wheelEvent(QWheelEvent* event)
{
    qreal factor = (event->angleDelta().y() > 0) ? 1.1 : 0.9;
    m_scale *= factor;
    m_scale = qBound(0.1, m_scale, 10.0);
    update();
}
