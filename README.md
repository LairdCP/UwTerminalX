# Laird UwTerminalX

## About

UwTerminalX is a cross-platform utility for communicating and downloading applications onto Laird's range of wireless modules, and uses Qt 5. The code uses functionality only supported in Qt 5.5 or greater but with some slight modifications will compile and run fine on earlier versions of Qt 5 (with no loss of functionality). UwTerminalX has been tested on Windows, Mac, Arch Linux and Ubuntu Linux and on the [Raspberry Pi running Raspbian](http://uwterminalx.no-ip.org/Github/rpi.png).

## Downloading

Pre-compiled builds can be found by clicking the [Releases](https://github.com/LairdCP/UwTerminalX/releases) tab on Github, builds are available for Linux (32 bit build), Mac (64 bit build) and Windows (32 bit build). Please note that the SSL builds include encryption when using online functionality and should only be used in countries where encryption is legal, non-SSL builds are also available from the release page.

## Setup

### Windows:

Download and open the zip file, extract the files to a folder on your computer and double click 'UwTerminalX.exe' to run UwTerminalX.

If using the SSL version of UwTerminalX, then the Visual Studio 2015 runtime files are required which are available on the [Microsoft site](https://www.microsoft.com/en-gb/download/details.aspx?id=48145).

### Mac:

Download and open the dmg file, open it to mount it as a drive on your computer, go to the disk drive and copy the file UwTerminalX to folder on your computer. You can run UwTerminalX by double clicking the icon.

### Linux (Including Raspberry Pi):

Download the tar file and extract it's contents to a location on your computer, this can be done using a graphical utility or from the command line using:

	tar xf UwTerminalX.tar ~/

Where '\~/' is the location of where you want it extracted to, '\~/' will extract to the home directory of your local user account). To launch UwTerminalX, either double click on the executable and click the 'run' button (if asked), or execute it from a terminal as so:

	./UwTerminalX

Before running, you may need to install some additional libraries, please see https://github.com/LairdCP/UwTerminalX/wiki/Installing for further details

## Help and contributing

There are various instructions and help pages available on the [wiki site](https://github.com/LairdCP/UwTerminalX/wiki/), due to problems with the service the UwTerminalX mailing list is currently unavailable.

Laird encourages people to branch/fork UwTerminalX to modify the code and accepts pull requests to merge these changes back into the main repository.

## Companian Applications

 * [TermNotify](https://github.com/LairdCP/TermNotify): an application that displays a popup notification when a new serial port is detected, which will open UwTerminalX if clicked.
 * [MultiDeviceLoader](https://github.com/LairdCP/MultiDeviceLoader): an application that can be used to XCompile a file and download it to multiple modules at the same time with various options including running the application or renaming it.

## Compiling

For details on compiling, please refer to [the wiki](https://github.com/LairdCP/UwTerminalX/wiki/Compiling).

## License

UwTerminalX is released under the [GPLv3 license](https://github.com/LairdCP/UwTerminalX/blob/master/LICENSE).