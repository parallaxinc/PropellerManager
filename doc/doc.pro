unix {
    doc.target = doc
    doc.commands = bash process.sh

    QMAKE_EXTRA_TARGETS += doc
} else {
    TEMPLATE=aux
}

