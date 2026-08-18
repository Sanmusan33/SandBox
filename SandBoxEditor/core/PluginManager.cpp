#include "PluginManager.h"

#include <QPluginLoader>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

PluginManager::PluginManager(const QString& configPath, QObject* parent)
    : QObject(parent)
    , configPath(configPath)
{
}

PluginManager::~PluginManager()
{
    saveConfig();
    // 卸载已加载的插件实例（先停用再删除）
    for (auto it = pluginsMap.begin(); it != pluginsMap.end(); ++it)
    {
        if (it->instance)
        {
            it->instance->onDeactivate();
            delete it->instance;
            it->instance = nullptr;
        }
    }
    pluginsMap.clear();
}

void PluginManager::registerAvailable(const QString& id, const QString& displayName,
                                      const QString& dllPath)
{
    if (id.isEmpty() || pluginsMap.contains(id))
        return;

    PluginInfo info;                 // 仅登记元信息，不加载 DLL
    info.id = id;
    info.displayName = displayName;
    info.dllPath = dllPath;
    pluginsMap[id] = info;
}

Plugin* PluginManager::loadPlugin(const QString& id)
{
    if (!pluginsMap.contains(id))
        return nullptr;

    PluginInfo& info = pluginsMap[id];
    if (info.instance)
        return info.instance;                  // 已加载，直接返回

    if (info.dllPath.isEmpty())
    {
        qWarning() << "Plugin" << id << "has no dll path, skip loading.";
        return nullptr;
    }

    QPluginLoader loader(info.dllPath);        // 此刻才真正加载功能包（DLL）
    QObject* obj = loader.instance();
    if (!obj)
    {
        qWarning() << "Failed to load plugin" << id << ":" << loader.errorString();
        return nullptr;
    }

    Plugin* plugin = qobject_cast<Plugin*>(obj);
    if (!plugin)
    {
        qWarning() << "Plugin" << id << "does not implement Plugin interface.";
        loader.unload();
        return nullptr;
    }

    info.instance = plugin;                    // 缓存实例，避免重复加载
    return plugin;
}

bool PluginManager::loadConfig()
{
    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Cannot open config:" << configPath;
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isObject()) return false;

    QJsonArray arr = doc.object()["features"].toArray();
    for (const QJsonValue& val : arr)
    {
        QJsonObject obj = val.toObject();
        QString id = obj["id"].toString();
        bool enabled = obj["enabled"].toBool(false);
        if (!id.isEmpty())
            enabledStatesMap[id] = enabled;
    }

    for (const QString& id : pluginsMap.keys())
    {
        if (!enabledStatesMap.contains(id))
            enabledStatesMap[id] = false;
    }

    return true;
}

void PluginManager::setPluginEnabled(const QString& id, bool enabled)
{
    if (!pluginsMap.contains(id)) return;
    if (enabledStatesMap.value(id) == enabled) return;

    if (enabled)
    {
        // 启用：惰性加载功能包，加载失败则回滚状态
        Plugin* plugin = loadPlugin(id);
        if (!plugin)
        {
            qWarning() << "Enable plugin failed:" << id;
            return;
        }
        enabledStatesMap[id] = true;
        plugin->onActivate();
    }
    else
    {
        enabledStatesMap[id] = false;
        if (pluginsMap[id].instance)
            pluginsMap[id].instance->onDeactivate();
    }

    emit pluginStateChanged(id, enabled);
}

bool PluginManager::isPluginEnabled(const QString& id) const
{
    return enabledStatesMap.value(id, false);
}

QStringList PluginManager::availablePluginIds() const
{
    return pluginsMap.keys();
}

QString PluginManager::displayName(const QString& id) const
{
    if (!pluginsMap.contains(id))
        return id;
    return pluginsMap.value(id).displayName;
}

Plugin* PluginManager::getPlugin(const QString& id) const
{
    if (!pluginsMap.contains(id))
        return nullptr;
    return pluginsMap.value(id).instance;   // 未加载时为 nullptr
}

QList<Plugin*> PluginManager::enabledPlugins() const
{
    QList<Plugin*> result;
    for (auto it = pluginsMap.constBegin(); it != pluginsMap.constEnd(); ++it)
    {
        if (enabledStatesMap.value(it.key(), false) && it.value().instance)
            result.append(it.value().instance);
    }
    return result;
}

bool PluginManager::saveConfig()
{
    QJsonArray arr;
    for (auto it = pluginsMap.constBegin(); it != pluginsMap.constEnd(); ++it)
    {
        QJsonObject obj;
        obj["id"] = it.key();
        obj["name"] = it.value().displayName;
        obj["path"] = "plugins/" + it.key();   // 约定输出目录，保证下次可重新定位
        obj["type"] = "plugin";
        obj["enabled"] = enabledStatesMap.value(it.key(), false);
        arr.append(obj);
    }

    QJsonObject root;
    root["features"] = arr;

    QFile file(configPath);
    if (!file.open(QIODevice::WriteOnly)) return false;
    file.write(QJsonDocument(root).toJson());
    file.close();
    return true;
}
