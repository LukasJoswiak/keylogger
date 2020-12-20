# keylogger

This project contains a macOS keylogger and a program to analyze recorded data.

The keylogger differs from most in that it records which physical keys your fingers hit, not which character is produced on the screen. This allows tracking of keyboard usage data.

*In early development, documentation may be lacking and/or wrong.*

## Build

Run `make` from the project root to generate two binaries:

* `bin/keylogger`: a keylogger which records pressed keys while running in the background.
* `bin/reporter`: a reporting utility which reads recorded keystroke data and formats it according to user specified flags.

The keylogger is macOS specific and will not work correctly on other systems.

## Run

`bin/keylogger`: run with the `--output_dir` flag to specify a directory to save data files to. One file will be generated for each unique keyboard used, and stores keystroke and timestamp data.

`bin/reporter`: run with the `--input` flag to specify a data file to read from. Currently, the reporter reports the counts of each physical key pressed. Key codes map to physical keys based on the USB HID Keyboard usage page (0x07) defined at https://www.usb.org/sites/default/files/hut1_21_0.pdf (page 82).
