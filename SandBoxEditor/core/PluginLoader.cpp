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
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Failed to open plugins.json at" << configPath;
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject())
    {
        qWarning() << "Invalid plugins.json format";
        return;
    }

    QJsonArray features = doc.object().value("features").toArray();
    qDebug() << "Total features defined in JSON:" << features.size();

    QDir baseDir(QCoreApplication::applicationDirPath());
    QStringList filters;
#ifdef Q_OS_WIN
    filters << "*.dll";
#else
    filters << "*.so" << "*.dylib";
#endif

    for (const QJsonValue& val : features)
    {
        QJsonObject obj = val.toObject();
        QString pluginId = obj.value("id").toString();
        if (pluginId.isEmpty()) continue;

        QString displayName = obj.value("name").toString();
        QString relativePath = obj.value("path").toString();

        // 搜索路径：优先配置的 path，其次约定目录 plugins/<id>
        QStringList searchPaths;
        if (!relativePath.isEmpty())
            searchPaths << baseDir.absoluteFilePath(relativePath);
        searchPaths << baseDir.absoluteFilePath("plugins/" + pluginId);

        // 定位 DLL（仅查找文件，不实例化——惰性加载由 PluginManager 负责）
        QString dllPath;
        for (const QString& path : searchPaths)
        {
            QDir dir(path);
            if (!dir.exists()) continue;
            QStringList files = dir.entryList(filters, QDir::Files);
            if (!files.isEmpty())
            {
                dllPath = dir.absoluteFilePath(files.first());
                break;
            }
        }

        if (dllPath.isEmpty())
        {
            qWarning() << "CRITICAL: Could not locate plugin:" << pluginId;
            continue;
        }

        // 读取插件内嵌元数据作为显示名（可选，失败时回退到 config 名称）
        QPluginLoader probe(dllPath);
        QJsonObject meta = probe.metaData().value("MetaData").toObject();
        QString metaName = meta.value("Name").toString();
        if (!metaName.isEmpty())
            displayName = metaName;

        qDebug() << "Register available plugin:" << pluginId
                 << "->" << dllPath;
        manager->registerAvailable(pluginId, displayName, dllPath);
    }
}

void PluginLoader::loadDataFromConfig()
{
    // 元数据已由 registerAll 读取；本方法保留签名兼容
}
