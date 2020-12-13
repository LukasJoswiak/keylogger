# keylogger

This project contains a macOS keylogger and a program to analyze recorded data.

*In early development, so documentation may be lacking and/or wrong.*

## Build

Run `make` from the project root to generate two binaries:

* `bin/keylogger`: a keylogger which records keystrokes while running in the background. Run with the `--output` flag to specify a file to save recorded keystrokes and timestamp data to.
* `bin/reporter`: a reporting utility which reads recorded keystroke data and formats it according to user specified flags. Run with the `--input` flag to specify a file to read recorded keystroke data from.

The keylogger is macOS specific and will not work correctly on other systems.
