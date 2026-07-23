#ifndef PLUGIN_H
#define PLUGIN_H

#include <QWidget>
#include <QString>

class Plugin : public QObject
{
    Q_OBJECT
public:
    explicit Plugin(QObject* parent = nullptr) : QObject(parent) {}
    ~Plugin() override = default;

    virtual QString id() const = 0;
    virtual QString displayName() const = 0;
    virtual QString description() const = 0;
    virtual QWidget* widget() = 0;

    virtual void onActivate() {}
    virtual void onDeactivate() {}
};

QT_BEGIN_NAMESPACE
#define Plugin_iid "org.sandbox.Plugin"
Q_DECLARE_INTERFACE(Plugin, Plugin_iid)
QT_END_NAMESPACE




#endif // PLUGIN_H
