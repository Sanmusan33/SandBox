#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QListWidget>
#include <QStackedWidget>
#include <QSplitter>
#include <QApplication>

#include "core/PluginManager.h"
#include "core/PluginLoader.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , pluginManager(nullptr)
    , pluginList(nullptr)
    , pluginPages(nullptr)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("SandBox Editor"));

    // 创建 PluginManager（配置文件在 exe 同级目录下）
    QString configPath = QApplication::applicationDirPath() + "/plugins.json";
    pluginManager = new PluginManager(configPath, this);

    // 注册所有已编译的插件
    PluginLoader::registerAll(pluginManager);
    pluginManager->loadConfig();

    // --- UI 布局：左列表 + 右内容 ---
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(splitter);

    // 左侧：插件列表
    pluginList = new QListWidget();
    pluginList->setMaximumWidth(120);
    splitter->addWidget(pluginList);

    // 右侧：插件页面容器
    pluginPages = new QStackedWidget();
    splitter->addWidget(pluginPages);

    splitter->setStretchFactor(0, 0);  // 左侧不拉伸
    splitter->setStretchFactor(1, 1);  // 右侧拉伸

    // 填充插件列表
    setupPluginList();

    // 连接信号
    connect(pluginManager, &PluginManager::pluginStateChanged,
            this, &MainWindow::onPluginCheckChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupPluginList()
{
    for (const QString& id : pluginManager->availablePluginIds())
    {
        Plugin* plugin = pluginManager->getPlugin(id);

        // 左侧列表项（可勾选）
        QListWidgetItem* item = new QListWidgetItem(plugin->displayName());
        item->setData(Qt::UserRole, id);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(pluginManager->isPluginEnabled(id)
                            ? Qt::Checked : Qt::Unchecked);
        pluginList->addItem(item);

        // 右侧页面
        pluginPages->addWidget(plugin->widget());

        // 默认启用的插件立即显示
        if (pluginManager->isPluginEnabled(id))
        {
            plugin->onActivate();
            pluginPages->setCurrentWidget(plugin->widget());
        }
    }

    // 勾选状态变化时切换插件
    connect(pluginList, &QListWidget::itemChanged, this, [this](QListWidgetItem* item)
    {
        QString id = item->data(Qt::UserRole).toString();
        bool enabled = (item->checkState() == Qt::Checked);
        pluginManager->setPluginEnabled(id, enabled);
    });
}

void MainWindow::onPluginCheckChanged(const QString& id, bool enabled)
{
    Plugin* plugin = pluginManager->getPlugin(id);
    if (!plugin) return;

    if (enabled)
    {
        pluginPages->setCurrentWidget(plugin->widget());
    }

    // 同步列表勾选状态（防止循环触发）
    for (int i = 0; i < pluginList->count(); ++i)
    {
        QListWidgetItem* item = pluginList->item(i);
        if (item->data(Qt::UserRole).toString() == id)
        {
            item->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);
            break;
        }
    }
}
