#include "propellerimage.h"

#include <QDebug>
#include <QFile>
#include <stdio.h>

int main()
{
    QString filename = "../../test/images/ls/FrappyBard.binary";
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
        return -1;

    PropellerImage image = PropellerImage(file.readAll(),filename);

    printf("           Image: %s\n",qPrintable(image.fileName()));
    printf("            Type: %s\n",qPrintable(image.imageTypeText()));
    printf("            Size: %u\n",image.imageSize());
    printf("        Checksum: %u (%s)\n",image.checksum(), image.checksumIsValid() ? "VALID" : "INVALID");
    printf("    Program size: %u\n",image.programSize());
    printf("   Variable size: %u\n",image.variableSize());
    printf(" Free/stack size: %u\n",image.stackSize());
    printf("      Clock mode: %s\n",qPrintable(image.clockModeText()));

    if (image.clockMode() != 0x00 && image.clockMode() != 0x01)
        printf(" Clock frequency: %i\n",image.clockFrequency());

    return 0;
}
