#ifndef OCTREEPLUGIN_H
#define OCTREEPLUGIN_H

#include "../../core/Plugin.h"
#include "OctreeWidget.h"
#include <qtmetamacros.h>

class OctreePlugin : public Plugin
{
    Q_OBJECT
    Q_INTERFACES(Plugin)
    Q_PLUGIN_METADATA(IID Plugin_iid FILE "metadata.json")
public:

    explicit OctreePlugin(QObject* parent = nullptr)
        : Plugin(parent)
        , m_widget(nullptr)
    {}

    QString id() const override { return QStringLiteral("octree_visualizer"); }
    QString displayName() const override { return QStringLiteral("八叉树可视化"); }
    QString description() const override
    {
        return QStringLiteral("2D 俯视八叉树节点分布图，支持拖拽平移、滚轮缩放。");
    }

    QWidget* widget() override
    {
        if (!m_widget)
            m_widget = new OctreeWidget();
        return m_widget;
    }

    void onActivate() override
    {
        if (m_widget) m_widget->show();
    }

    void onDeactivate() override
    {
        if (m_widget) m_widget->hide();
    }

private:
    OctreeWidget* m_widget;
};

#endif // OCTREEPLUGIN_H
