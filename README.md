# Nordic nRF-51(422) project: Central client to connect to SensorTag 2650

This project targets the Nordic nRF-51 DK development board. It is demonstration
project simply to show a SensorTag CC2650STK being connected via BLE to a NRF51, and
its ability to gather data from the SensorTag.

This is a central project that will connect to an 'out of box' Texas Instruments
SensorTag 2560, when it is detected via its default advertising.

[A future extension will be to add a simultaneous peripheral interface to connect to a Linux client 
to allow graphic visualisation of the data.]

## License

Small amounts of boilerplate code for initialisation, scanning, and sleep control have
been used from the Nordic SDK example code. 

Files which contain a Nordic Semiconductor copyright notice must only be redistributed 
with that copyright notice intact, and projects containing that source code must only
be used with a Nordic Semiconductor ASA device. All other code is MIT license.

## Requirements

This assumes that you want to compile the project and download it to the target Nordic hardware,
as a demonstration project this assumes that you have access to the following:

  - A Nordic nRF-51 DK development board
  - A Texas Instruments SensorTag 2650
  - arm-gcc-none-eabi GCC toolchain / a Linux build environment
  - Nordic nRF5 SDK version 12.3.0

### Toolchain Installation Notes

#### nRF-51
The nRF-51 DK is actually discontinued, but is still available for purchase new and retail. This
project may be extended to include the nRF-52 DK at a later date.

#### CC2650STK
The Texas Instruments SensorTag should be model CC2650STK; this project has been tested with
Rev 1.2.1 / Rev 1.3.2 FW. This is 'out-of-box' with no modifications whatsoever - it is purely
here to provide I/O and data.

#### GNU GCC Toolchain
The latest arm-gcc-none-eabi toolchain can be obtained from: 
    https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads

Extract the archive e.g. 'arm-gnu-toolchain-14.2.rel1-x86_64-arm-none-eabi.tar.xz'
to a location of your choice, e.g. /opt and ensure that the toolchain /bin folder is added to
the compile environment PATH with, e.g.

`export PATH="$PATH:/opt/gcc-arm-none-eabi-14.2/bin"`

or, for Ubuntu, a slightly older version can be used with

`sudo apt install gcc-arm-none-eabi`

#### Nordic nRF5 SDK 12.3

This is currently available from:
    https://www.nordicsemi.com/Products/Development-software/nRF5-SDK/Download#infotabs

After selecting version 12.3, select nRF5_SDK_12.3.0, and download the files. The .zip file will
contain one or more .zip files, depending on the components selected; extract the SDK .zip file
to a suitable location, e.g. /opt/ or /home/$USER/{some_folder}

This folder will be referenced from the project makefile, either hard-coded or by an enviroment
variable.

#### Picocom (optional)

In order to visualize the data from the device's UART, you will need a serial terminal program; Picocom
can be installed by 

`sudo install picocom`

## Build the software and flash the Nordic DK

First, clone this repository, and 'cd' into the folder.

Inform the make file of the location of your Nordic SDK with the environment variable NRF_SDK_ROOT, e.g:

`export NRF_SDK_ROOT=/opt/{MY_SDK_FOLDER_NAME}`

Plug in the Nordic NRF51 development board, make the project, flash the softdevice and flash the the project
to the dev board. (The first make step is not strictly necessary, the flash target will make this).

`make
make flash_softdevice
make flash`

## Usage

This assumes the Nordic DK has been flashed with this software (see above). 

Monitor the connected UART of the dev board by opening a terminal; for example use picocom

`picocom -b115200 /dev/ttyACM0 --imap lfcrlf`

(The baud rate is 115200, and the NRF51DK normally mounts at ttyACM0. The imap switch 
converts LF (\n) characters into CRLF (\r\n) for easier viewing.)

Reset the board to see the diagnostic messages from the beginning. The device will report that
it is scanning, and LED1 will flash periodically.

Bring the SensorTag CC2650STK online; it will immediately begin to advertise.

The Nordic device will detect the SensorTag and report its BLE address upon connection. Also,
LED1 will be be lit 'solidly' and no longer flashing. 

It will automatically configure the Luxometer and Temperature services of the SensorTag. 

Observe the reported data from the Luxometer and Temperature readings; 

    - the luxometer reads 0-65536 from darkness to very bright light.
    - the ambient temperature gives the temperature of the device, in Celsius

Now either modify the code to your specifications to use the default SensorTag for your use case,
or use this project as an example to connect the NRF51 to other devices.

### Notes

If you are powering the SensorTag CC2650STK using a 'Debugger DevPack' it actually gets quite
warm and so will heat up the SensorTag giving an incorrect reading. It is much better when
powered by a coin cell.

