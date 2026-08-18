#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QObject>
#include <QMap>
#include <QStringList>
#include "Plugin.h"

// 插件描述：仅注册元信息，不实例化（惰性加载）
// 功能包（插件 DLL）在用户首次勾选启用时才真正加载
struct PluginInfo
{
    QString id;                 // 插件唯一标识（与 plugins.json 一致）
    QString displayName;        // 显示名（列表展示用）
    QString dllPath;            // DLL 完整路径（空表示未定位到）
    Plugin* instance = nullptr; // 惰性实例（首次启用时加载，未启用为 nullptr）
};

class PluginManager : public QObject
{
    Q_OBJECT
public:
    explicit PluginManager(const QString& configPath, QObject* parent = nullptr);
    ~PluginManager() override;

    // 注册可用插件（仅元信息，不加载 DLL）
    void registerAvailable(const QString& id, const QString& displayName,
                           const QString& dllPath);

    // 惰性加载并返回插件实例；已加载直接返回，失败返回 nullptr
    Plugin* loadPlugin(const QString& id);

    bool loadConfig();
    void setPluginEnabled(const QString& id, bool enabled);
    bool isPluginEnabled(const QString& id) const;

    QStringList availablePluginIds() const;
    QString displayName(const QString& id) const;
    Plugin* getPlugin(const QString& id) const;   // 仅返回已加载实例（未加载返回 nullptr）
    QList<Plugin*> enabledPlugins() const;

signals:
    void pluginStateChanged(const QString& id, bool enabled);

private:
    bool saveConfig();
    QString configPath;
    QMap<QString, PluginInfo> pluginsMap;   // id -> 插件描述
    QMap<QString, bool> enabledStatesMap;
};

#endif // PLUGINMANAGER_H
