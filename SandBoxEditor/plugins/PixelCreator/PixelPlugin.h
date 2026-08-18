#ifndef PIXELPLUGIN_H      // 头文件保护宏开始：防止同一头文件被重复包含
#define PIXELPLUGIN_H      // 定义保护宏：首次包含后即为"已定义"状态

#include "../../core/Plugin.h"  // 引入 SandBox 插件体系基类接口（定义 Plugin 抽象类）
#include "ViewportWidget.h"     // 引入视口窗口类（本插件的核心界面）
#include <qtmetamacros.h>       // 引入 Qt 元对象宏（Q_PLUGIN_METADATA / Q_INTERFACES 等）

class PixelPlugin : public Plugin   // 像素创作器插件类：继承 Plugin 基类接口
{
    Q_OBJECT                        // Qt 元对象宏：为插件类启用元对象信息（MOC 必需）
    Q_INTERFACES(Plugin)            // 声明实现 Plugin 接口：供 QPluginLoader 的 qobject_cast 识别
    Q_PLUGIN_METADATA(IID Plugin_iid FILE "metadata.json")  // 声明插件元数据：IID 匹配 Plugin 接口、JSON 文件内嵌
public:

    explicit PixelPlugin(QObject* parent = nullptr)  // 构造函数：可指定父对象
        : Plugin(parent)          // 调用基类构造函数
        , m_widget(nullptr)       // 视口指针初始化为空（延迟到首次访问时创建）
    {}

    QString id() const override { return QStringLiteral("pixel_creator"); }  // 插件唯一标识符（须与 plugins.json 中 id 一致）
    QString displayName() const override { return QStringLiteral("Pixel Creator"); }  // 插件显示名称（出现在左侧列表）
    QString description() const override   // 插件功能描述
    {
        return QStringLiteral("软件光栅化管线教学单元：像素级光栅化管线 + 自由视角相机。");  // 返回描述文本
    }

    QWidget* widget() override    // 返回插件主界面窗口
    {
        if (!m_widget)            // 若尚未创建
            m_widget = new ViewportWidget();  // 惰性创建视口窗口（延迟实例化）
        return m_widget;          // 返回窗口指针
    }

    void onActivate() override    // 插件激活回调（被勾选启用时调用）
    {
        if (m_widget) m_widget->show();   // 窗口已存在则显示
    }

    void onDeactivate() override  // 插件停用回调（取消勾选时调用）
    {
        if (m_widget) m_widget->hide();   // 窗口已存在则隐藏
    }

private:
    ViewportWidget* m_widget;     // 视口窗口指针（惰性初始化，首次 widget() 时创建）
};

#endif // PIXELPLUGIN_H   // 头文件保护宏结束
