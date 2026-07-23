#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QObject>
#include <QMap>
#include <QStringList>
#include "Plugin.h"

class PluginManager : public QObject
{
    Q_OBJECT
public:
    explicit PluginManager(const QString& configPath, QObject* parent = nullptr);
    ~PluginManager() override;

    void registerPlugin(Plugin* plugin);
    bool loadConfig();
    void setPluginEnabled(const QString& id, bool enabled);
    bool isPluginEnabled(const QString& id) const;

    QStringList availablePluginIds() const;
    Plugin* getPlugin(const QString& id) const;
    QList<Plugin*> enabledPlugins() const;

signals:
    void pluginStateChanged(const QString& id, bool enabled);

private:
    bool saveConfig();
    QString configPath;
    QMap<QString, Plugin*> pluginsMap;
    QMap<QString, bool> enabledStatesMap;
};

#endif // PLUGINMANAGER_H
