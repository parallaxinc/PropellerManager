#pragma once

#include <QTimer>
#include <QStringList>

class PropellerManager : public QObject
{
    Q_OBJECT

private:
    QTimer portMonitor;
    QStringList _ports;

public:
    PropellerManager(QObject *parent = 0);
    ~PropellerManager();

    const QStringList & listPorts();

//    bool open(const QString & portname);
//    bool close(const QString & portname);
//    bool open(const QString & portname);

    void enablePortMonitor(bool enabled);

//    bool attach(PropellerObject * object, const QString & portname);
//    bool detach(PropellerObject * object, const QString & portname);


public slots:
    void checkPorts();

signals:
    void portListChanged();

};
