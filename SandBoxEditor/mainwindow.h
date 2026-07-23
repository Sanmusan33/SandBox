#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QListWidget;
class QStackedWidget;
class PluginManager;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onPluginCheckChanged(const QString& id, bool enabled);

private:
    void setupPluginList();

    Ui::MainWindow *ui;
    PluginManager* pluginManager;
    QListWidget* pluginList;
    QStackedWidget* pluginPages;
};

#endif // MAINWINDOW_H
