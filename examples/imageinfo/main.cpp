#include <PropellerImage>

#include <QFile>
#include <stdio.h>

int main(int argc, char *argv[])
{
    if (argc < 2) return 1;

    QString filename = argv[1];
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
        return 1;

    PropellerImage image = PropellerImage(file.readAll(),filename);

    printf("             Image: %s\n",          qPrintable(image.fileName()));
    printf("              Type: %s\n",          qPrintable(image.imageTypeText()));

    printf("          Checksum: %u (%s)\n\n",   image.checksum(),
                                                image.checksumIsValid() ? "VALID" : "INVALID");

    printf("        Image size: %d\n",          image.imageSize());
    printf("      Program size: %d\n\n",        image.programSize());

    printf("     Start of code: %04X\n",        image.startOfCode());
    printf("Start of variables: %04X\n",        image.startOfVariables());
    printf("    Start of stack: %04X\n\n",      image.startOfStackSpace());

    printf("         Code size: %04X\n",        image.codeSize());
    printf("     Variable size: %04X\n",        image.variableSize());
    printf("   Free/stack size: %04X\n\n",      image.stackSize());

    printf("        Clock mode: %s\n",          qPrintable(image.clockModeText()));

    if (image.clockMode() != 0x00 && 
        image.clockMode() != 0x01)

        printf(" Clock frequency: %i\n",        image.clockFrequency());

    return 0;
}
