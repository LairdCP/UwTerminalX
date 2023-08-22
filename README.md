# Laird Connectivity UwTerminalX

## About

UwTerminalX is a cross-platform utility for communicating and downloading applications onto Laird's range of wireless modules, and uses Qt 5 or 6. The code uses functionality only supported in Qt 5.7 or greater but with some slight modifications will compile and run fine on earlier versions of Qt 5 (with no loss of functionality). UwTerminalX has been tested on Windows, Mac, Arch Linux and Ubuntu Linux and on the [Raspberry Pi running Raspbian](/docs/images/rpi.png?raw=true).

## Downloading

Pre-compiled builds can be found by clicking the [Releases](https://github.com/LairdCP/UwTerminalX/releases) tab on Github, builds are available for Linux (32-bit and 64-bit builds, and a 32-bit ARM Raspberry Pi build), Windows (32-bit build) and Mac (64-bit build) . Please note that the SSL builds include encryption when using online functionality and should only be used in countries where encryption is legal, non-SSL builds are also available from the release page.

It is recommended that you download the **SSL** build of UwTerminalX for your system as the prevents eavesdropping of the application source code when using the online service to XCompile applications.

## Setup

### Windows:

Download and open the zip file, extract the files to a folder on your computer and double click 'UwTerminalX.exe' to run UwTerminalX.

If using the SSL version of UwTerminalX, then the Visual Studio 2015 runtime files are required which are available on the [Microsoft site](https://www.microsoft.com/en-gb/download/details.aspx?id=48145).

### Mac:

(**Mac OS X version 10.10 or later is required if using the pre-compiled binaries, as Secure Transport is built into OS X there is no non-SSL build available for OS X systems**): Download and open the dmg file, open it to mount it as a drive on your computer, go to the disk drive and copy the file UwTerminalX to folder on your computer. You can run UwTerminalX by double clicking the icon - if you are greeted with a warning that the executable is untrusted then you can run it by right clicking it and selecting the 'run' option. If this does not work then you may need to view the executable security settings on your mac.

### Linux (Including Raspberry Pi):

Download the tar file and extract it's contents to a location on your computer, this can be done using a graphical utility or from the command line using:

	tar xf UwTerminalX_<version>.tar.gz -C ~/

Where '\~/' is the location of where you want it extracted to, '\~/' will extract to the home directory of your local user account). To launch UwTerminalX, either double click on the executable and click the 'run' button (if asked), or execute it from a terminal as so:

	./UwTerminalX

Before running, you may need to install some additional libraries, please see https://github.com/LairdCP/UwTerminalX/wiki/Installing for further details. You may also be required to add a udev rule to grant non-root users access to USB devices, please see https://github.com/LairdCP/UwTerminalX/wiki/Granting-non-root-USB-device-access-(Linux) for details.

## Help and contributing

There are various instructions and help pages available on the [wiki site](https://github.com/LairdCP/UwTerminalX/wiki/).

Laird encourages people to branch/fork UwTerminalX to modify the code and accepts pull requests to merge these changes back into the main repository.

## Mailing list/discussion

There is a mailing list available for dicussion about UwTerminalX which is available on https://groups.io/g/UwTerminalX

## Speed/Throughput testing

There is a quick guide available giving an overview of the speed testing feature of UwTerminalX, https://github.com/LairdCP/UwTerminalX/wiki/Using-the-Speed-Test-feature

## Companian Applications

 * [TermNotify](https://github.com/LairdCP/TermNotify): an application that displays a popup notification when a new serial port is detected, which will open UwTerminalX if clicked.
 * [MultiDeviceLoader](https://github.com/LairdCP/MultiDeviceLoader): an application that can be used to XCompile a file and download it to multiple modules at the same time with various options including running the application or renaming it.

## Compiling

For details on compiling, please refer to [the wiki](https://github.com/LairdCP/UwTerminalX/wiki/Compiling).

## License

UwTerminalX is released under the [GPLv3 license](https://github.com/LairdCP/UwTerminalX/blob/master/LICENSE).
