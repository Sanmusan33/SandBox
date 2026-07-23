#include "PluginManager.h"
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
    qDeleteAll(pluginsMap);
    pluginsMap.clear();
}

void PluginManager::registerPlugin(Plugin* plugin)
{
    if (plugin && !pluginsMap.contains(plugin->id()))
    {
        pluginsMap[plugin->id()] = plugin;
    }
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

    QJsonArray arr = doc.object()["features"].toArray(); // 保留 features 键名兼容
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

    enabledStatesMap[id] = enabled;

    if (enabled)
        pluginsMap[id]->onActivate();
    else
        pluginsMap[id]->onDeactivate();

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

Plugin* PluginManager::getPlugin(const QString& id) const
{
    return pluginsMap.value(id, nullptr);
}

QList<Plugin*> PluginManager::enabledPlugins() const
{
    QList<Plugin*> result;
    for (auto it = pluginsMap.constBegin(); it != pluginsMap.constEnd(); ++it)
    {
        if (enabledStatesMap.value(it.key(), false))
            result.append(it.value());
    }
    return result;
}

bool PluginManager::saveConfig()
{
    QJsonArray arr;
    for (auto it = enabledStatesMap.constBegin(); it != enabledStatesMap.constEnd(); ++it)            
    {
        QJsonObject obj;
        obj["id"] = it.key();
        obj["enabled"] = it.value();
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
