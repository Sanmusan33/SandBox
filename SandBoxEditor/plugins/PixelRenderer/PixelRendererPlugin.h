#ifndef PIXELRENDERERPLUGIN_H
#define PIXELRENDERERPLUGIN_H

#include "../../core/Plugin.h"
#include "ui/PixelWidget.h"
#include <qtmetamacros.h>

// 插件入口：实现 Plugin 接口，对外提供 PixelWidget 呈现窗口
// 参照 plugins/PixelCreator/PixelPlugin.h 的模式填充
class PixelRendererPlugin : public Plugin
{
    Q_OBJECT
    Q_INTERFACES(Plugin)
    Q_PLUGIN_METADATA(IID Plugin_iid FILE "metadata.json")
public:

    explicit PixelRendererPlugin(QObject* parent = nullptr)   // 构造函数
        : Plugin(parent)          // 调用基类构造
        , m_widget(nullptr)       // 窗口指针初始化为空（惰性创建）
    {}

    QString id() const override
    { return QStringLiteral("pixel_renderer"); }   // 插件唯一标识（须与 plugins.json 一致）

    QString displayName() const override
    { return QStringLiteral("Pixel Renderer"); }   // 插件显示名（左侧列表）

    QString description() const override
    { return QStringLiteral("基于 OpenGL 逐像素绘制的模拟光栅化管线教学单元。"); }   // 功能描述

    QWidget* widget() override
    {
        if (!m_widget) m_widget = new PixelWidget();   // 惰性创建
        return m_widget;                               // 返回窗口指针
    }

    void onActivate() override
    {
        // TODO(用户): if (m_widget) m_widget->show();
        if (m_widget) m_widget->show();
    }

    void onDeactivate() override
    {
        // TODO(用户): if (m_widget) m_widget->hide();
        if (m_widget) m_widget->hide();
    }

private:
    PixelWidget* m_widget;   // 呈现窗口指针（惰性初始化）
};

#endif // PIXELRENDERERPLUGIN_H
