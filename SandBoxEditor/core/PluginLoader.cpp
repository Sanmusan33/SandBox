#include "PluginLoader.h"
#include "PluginManager.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QPluginLoader>
#include <QCoreApplication>

void PluginLoader::registerAll(PluginManager* manager)
{
    if (!manager) return;

    // 统一使用 plugins.json
    QString configPath = QDir(QCoreApplication::applicationDirPath()).filePath("plugins.json");
    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open plugins.json at" << configPath;
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Invalid plugins.json format";
        return;
    }

    QJsonArray features = doc.object().value("features").toArray();
    qDebug() << "Total features defined in JSON:" << features.size();

    for (const QJsonValue& val : features) {
        QJsonObject obj = val.toObject();
        QString relativePath = obj.value("path").toString();
        QString pluginId = obj.value("id").toString();

        qDebug() << "---------------------------------------";
        qDebug() << "Processing Plugin ID:" << pluginId;

        QDir baseDir(QCoreApplication::applicationDirPath());
        qDebug() << "Application Dir:" << baseDir.absolutePath();

        QStringList searchPaths;
        searchPaths << baseDir.absoluteFilePath(relativePath);
        searchPaths << baseDir.absoluteFilePath("plugins/" + pluginId);
        searchPaths << baseDir.absoluteFilePath("plugins");
        searchPaths << baseDir.absoluteFilePath("plugins/OctreeVisualizer"); // 针对性路径

        bool loaded = false;
        for (const QString& path : searchPaths) {
            QDir dir(path);
            qDebug() << "  Scanning path:" << path << (dir.exists() ? "[EXISTS]" : "[NOT FOUND]");
            if (!dir.exists()) continue;

            QStringList filters;
#ifdef Q_OS_WIN
            filters << "*.dll";
#else
            filters << "*.so" << "*.dylib";
#endif
            QStringList files = dir.entryList(filters, QDir::Files);
            qDebug() << "  Files found in dir:" << files;

            for (const QString& fileName : files) {
                QString fullPath = dir.absoluteFilePath(fileName);
                qDebug() << "  Attempting to load:" << fullPath;

                QPluginLoader loader(fullPath);
                QObject* instance = loader.instance();
                if (instance) {
                    Plugin* plugin = qobject_cast<Plugin*>(instance);
                    if (plugin) {
                        qDebug() << "  SUCCESS: Loaded" << plugin->displayName();
                        manager->registerPlugin(plugin);
                        loaded = true;
                        break;
                    } else {
                        qWarning() << "  FAILED: File is a Qt plugin but doesn't implement Plugin interface. Check IID!";
                    }
                } else {
                    qDebug() << "  FAILED: loader.instance() returned null. Error:" << loader.errorString();
                }
            }
            if (loaded) break;
        }
        
        if (!loaded) {
            qWarning() << "  CRITICAL: Could not find or load plugin:" << pluginId;
        }
    }
}

void PluginLoader::loadDataFromConfig()
{
    // 逻辑已整合到 registerAll 中
}
