#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

class PluginManager;

class PluginLoader
{
public:
    static void registerAll(PluginManager* manager);

private:
    /** 从配置文件读取插件信息 */
    static void loadDataFromConfig();
};

#endif // PLUGINLOADER_H
