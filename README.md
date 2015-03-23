# Propeller Manager - *do more with the Propeller*

Propeller Manager is more than a download tool; it is a complete API and toolset for managing Propeller devices, no matter where they are.

## About

### Design Goals

* Full IDE integration
* Persistent sessions for painless cross-platform debugging
* Clean, object-oriented Qt implementation
* High-speed downloading
* Download over Wi-Fi
* C++ API and `propman` command-line interface

#### Stretch Goals

* Spin code bootstrapping for custom download applications

**NOTE: This project is in the early design stages**

## Usage

#### `propman` command-line interface

Propeller Manager includes a small utility to use it from the command-line.

For the most basic use cases, simply pass a binary name to `propman` to download to RAM. `propman` will use the first or only device available in the system.

```
propman Brettris.binary
```

Include the `-w` option to write the program to EEPROM.

For more complex use cases, get a list of available devices with `-l`.

```
$ propman -l
"/dev/ttyUSB0"
"/dev/ttyS0"
```

Use the `-d` option to download to a specific device.

```
propman Brettris.binary -d /dev/ttyUSB0
```

Get your `propman` version with `-v`.

```
propman -v
Propeller Manager CLI 0.0.0
```

Get more help with `-h`.

```
Usage: ./propman [options] FILE

A command-line wrapper to the Propeller Manager API
Copyright 2015 by Parallax Inc.

Options:
  -h, --help          Displays this help.
  -v, --version       Displays version information.
  -l, --list          List available devices
  -w, --write         Write program to EEPROM
  -d, --device <DEV>  Device to program (default: first system device)

Arguments:
  file                Binary file to download
```


## License

Propeller Manager is licensed under the GNU Public License v3.

## Author

Propeller Manager is developed by LameStation LLC in collaboration with Parallax Inc.
