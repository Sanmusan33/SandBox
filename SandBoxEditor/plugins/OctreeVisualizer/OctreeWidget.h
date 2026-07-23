#ifndef OCTREEWIDGET_H
#define OCTREEWIDGET_H

#include <QWidget>

class OctreeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OctreeWidget(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    bool m_dragging = false;
    QPoint m_lastMousePos;
    QPointF m_offset;
    qreal m_scale = 1.0;
};

#endif // OCTREEWIDGET_H
