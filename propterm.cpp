#include "propterm.h"
#include "console.h"

#include <QMessageBox>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>

PropTerm::PropTerm(QWidget *parent) :
    QWidget(parent)
{
    ui.setupUi(this);
    ui.console->setEnabled(false);

    connect(&serial, SIGNAL(error(QSerialPort::SerialPortError)), this,
            SLOT(handleError(QSerialPort::SerialPortError)));

    connect(&serial, SIGNAL(readyRead()), this, SLOT(readData()));
    connect(ui.console, SIGNAL(getData(QByteArray)), this, SLOT(writeData(QByteArray)));

    connect(ui.port, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(portChanged(const QString &)));
    connect(ui.baudRate, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(baudRateChanged(const QString &)));

    connect(ui.clear, SIGNAL(clicked()), ui.console, SLOT(clear()));
    connect(ui.enable, SIGNAL(toggled(bool)), this, SLOT(handleToggle(bool)));
    connect(ui.enable, SIGNAL(toggled(bool)), ui.console, SLOT(enable(bool)));

    connect(ui.echo, SIGNAL(clicked(bool)), ui.console, SLOT(setEchoEnabled(bool)));

    foreach (QSerialPortInfo i, QSerialPortInfo::availablePorts())
    {
        ui.port->addItem(i.portName(),QVariant(i.systemLocation()));
    }

    ui.console->clear();
}

PropTerm::~PropTerm()
{
}

void PropTerm::openSerialPort()
{
    ui.console->setEnabled(true);
    ui.console->setEchoEnabled(ui.echo->isChecked());

    serial.setPortName(ui.port->currentText());
    serial.setBaudRate(ui.baudRate->currentText().toInt());
    serial.open(QIODevice::ReadWrite);
}

void PropTerm::portChanged(const QString & text)
{
    closeSerialPort();
    qDebug() << text;
    openSerialPort();
}

void PropTerm::baudRateChanged(const QString & text)
{
    qDebug() << serial.setBaudRate(text.toInt());
}

void PropTerm::closeSerialPort()
{
    serial.close();
    ui.console->setEnabled(false);
}

void PropTerm::writeData(const QByteArray &data)
{
    serial.write(data);
}

void PropTerm::readData()
{
    QByteArray data = serial.readAll();
    ui.console->putData(data);
}

void PropTerm::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), serial.errorString());
        closeSerialPort();
    }
}

void PropTerm::handleToggle(bool checked)
{
    if (checked)
        openSerialPort();
    else
        closeSerialPort();
}
