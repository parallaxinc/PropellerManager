/**
@page buildingspin Building Spin Projects With `qmake`

A common use of PropellerManager is to load special firmware to a board. The 
main question that arises is how to make the code available to your application.
You could just stick it in a directory and update the firmware when you need 
to, but that gets annoying to maintain very fast.

A more elegant approach is to embed your firmware directly into your application.
That way it's always up-to-date and always available when you need it.

In this tutorial, we will show you how to do just that, using code taken from 
BadgeHacker, a tool that downloads and configures firmware for the Hackable 
Electronic Badge using PropellerManager.

http://developer.parallax.com/badgehacker/

### Building the binary

To begin, we need to create a `qmake` project file that can build Spin 
projects. This is fairly easy to make.

[spin/spin.pro](https://github.com/parallaxinc/BadgeHacker/blob/master/spin/spin.pro)

    CONFIG -= qt
    TEMPLATE = aux
    
    spin.target = jm_hackable_ebadge.binary
    spin.commands = openspin jm_hackable_ebadge.spin
    spin.depends = $$files(*.spin)
    
    PRE_TARGETDEPS = jm_hackable_ebadge.binary
    QMAKE_EXTRA_TARGETS = spin

Then, you can include your binary in your main program with an include file: 

[spin/include.pri](https://github.com/parallaxinc/BadgeHacker/blob/master/spin/include.pri)

    PRE_TARGETDEPS += $$PWD/jm_hackable_ebadge.binary
    RESOURCES += $$PWD/spin.qrc

The `PRE_TARGETDEPS` command tells qmake that your program depends on this 
other binary, so it will recompile if the binary is changed.

The `RESOURCES` part compiles the helper binary into the program (it also 
included the Spin file for another reason).

Here are the contents of the resource file: 

[spin/spin.qrc](https://github.com/parallaxinc/BadgeHacker/blob/master/spin/spin.qrc)

    <!DOCTYPE RCC><RCC version="1.0">
    <qresource prefix="/spin">
        <file>jm_hackable_ebadge.binary</file>
        <file>jm_hackable_ebadge.spin</file>
    </qresource>
    </RCC>

### Embedding the binary

Here's how to include your dependency and resource in the main program: 

[src/src.pro](https://github.com/parallaxinc/BadgeHacker/blob/master/src/src.pro)

    include(../spin/include.pri)

After you've compiled in the resource, you can use it just like any other file 
in your program. Here's a snippet from BadgeHacker where I am downloading a 
Spin binary that's compiled into the program. It's a part of your executable so 
you never have to worry about that file not being found.

[src/badge.cpp](https://github.com/parallaxinc/BadgeHacker/blob/master/src/badge.cpp)

    QFile file(":/spin/jm_hackable_ebadge.binary");
    file.open(QIODevice::ReadOnly);
    PropellerImage image = PropellerImage(file.readAll());
    return loader->upload(image, true);

As you can see, the Qt resource system is very powerful and allows me to work a 
lot of magic. Coupled with PropellerManager, it's fantastically easy to embed 
any kind of content, including Spin helper programs, into your applications. 

**Woohoo!**

*/

