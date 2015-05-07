#include "utility.h"

QByteArray Utility::readFile(QString filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) return QByteArray();
    QByteArray blob = file.readAll();
    return blob;
}

void Utility::print(const QString & text)
{
    fprintf(stderr, "%s",qPrintable(text));
    fflush(stderr);
}

void Utility::print_task(const QString & text)
{
    fprintf(stderr, "%-30s",qPrintable(text));
    fflush(stderr);
}

void Utility::print_status(const QString & text)
{
    print("[ ");
    print(text);
    print(" ]\n");
}

void Utility::print_error(int code, const QString & message)
{
    qDebug("[ ERROR %2i ]: %s",code,qPrintable(message));
}
