# MIDI to USB adapter

Simple MIDI (UART) to USB adapter running on AT90USB162 microcontroller

## Features:

- build upon LUFA USB framework
- only channel voice messages are supported

## Dependencies

- LUFA
- midiXparser

## How to build

`git clone --recurse-submodules https://github.com/kwarc93/midi-to-usb.git`

`cd midi-to-usb`

`make all`