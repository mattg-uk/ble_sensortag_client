# Nordic nRF-51(422) project: Central client to connect to SensorTag 2560

This project targets the Nordic nRF-51 DK development board. 

This is a dual central / peripheral project that will simulataneously connect as a central to an 
'out of box' Texas Instruments SensorTag 2560, and offer a Peripheral connection that can be
connected using the supplied Linux Client, or simply be monitored using a 'sniffer' or the Nordic
Connect mobile application.

## Requirements

This assumes that you want to compile the project and download it to the target Nordic hardware,
as a demonstration project this assumes that you have access to the following:

  - A Nordic nRF-51 DK development board
  - A Texas Instruments SensorTag 2650
  - arm-gcc-none-eabi GCC toolchain / a Linux build environment
  - Nordic nRF5 SDK version 12.3.0

### Installation Notes

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

This folder will be required to be referenced from the project makefile.














