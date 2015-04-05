#pragma once

#include <QWidget>
#include <QSerialPort>

#include "ui_propterm.h"

class Console;

class PropTerm : public QWidget
{
    Q_OBJECT

private:
    Ui::PropTerm ui;
    Console console;
    QSerialPort serial;

public:
    explicit PropTerm(QWidget *parent = 0);
    ~PropTerm();

private slots:
    void openSerialPort();
    void closeSerialPort();
    void writeData(const QByteArray &data);
    void readData();

    void handleError(QSerialPort::SerialPortError error);
    void handleToggle(bool checked);
    void portChanged(const QString & text);
    void baudRateChanged(const QString & text);

private:
};
