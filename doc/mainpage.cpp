/**
@mainpage Overview

- @subpage tutorials
- @subpage faq

PropellerManager works by acting as a signal router between PropellerSession and PropellerDevice instances,
and prevents these classes from interacting with each other directly.

![](manager_layers.png)

Sessions can communicate with any available device without needing to 
manually open and close ports. Many sessions can be connected to a single
device at a time. However, sessions may also request exclusive access to
a device to complete a long-running or time-critical operation.

The PropellerManager API consists of six main classes:

- PropellerManager
- PropellerSession
- PropellerLoader
- PropellerTerminal
- PropellerDevice
- PropellerImage

PropellerManager handles device and platform issues transparently, allowing you to deploy a Propeller
application in just a few lines of code.

Here is an example of how quickly a loader application can be written.

@include download/main.cpp

### Opening and closing devices

PropellerManager handles device opening and closing by maintaining a count of sessions connected to each
device. As long as the session count is greater than zero, a device will remain open.

When no PropellerSessions are registered in PropellerManager, all devices are released to the operating
system.

![](session_empty.png)

### Use of devices by multiple sessions

Sessions can communicate with any available device without the need to open and close ports or handle
device errors. Many sessions can be connected to a single device at a time.

![](session_multiple.png)

This configuration allows multiple widgets to consume device output and do interesting things
with it, or can be used to simplify the creation of interactive GUI applets.

### Exclusive use

The PropellerSession::reserve() and PropellerSession::release() functions control are responsible
for reserving devices for exclusive use. When a session requests exclusive access, all other sessions are
notified and paused temporarily.

PropellerLoader, being a time-critical application, wants to be the only user when sending an image with
PropellerLoader::upload(), so it will reserve the port and release it at the end of the download.

![](session_loading.png)

Upon releasing the device, all other attached devices will be able to resume sending data.

### Read buffering

Each PropellerSession has its own copy of the device's read buffer for its own personal usage. Because of this,
session applications should be developed as though they are the only 

![](manager_buffers.png)

*/
