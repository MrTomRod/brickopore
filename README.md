# brickopore

This repo combines richardmleggett's two LEGO sequencer repos:

- https://github.com/richardmleggett/brickopore
- https://github.com/richardmleggett/BrickoporeGUI


## Improvements:

- The C code that runs on the LEGO EV3 was updated to C++
- A few tweaks to so that the accuracy of the readout no longer depends on the 
serial links speed
- Allows for "reads" of more than 24 nucleotides
- A "Copy Sequence" button was added to the Java GUI


## Notes:

- The compiled binary is the file `client` in the root directory
- The compiled GUI code is located in `GUI/target/brickopore-1.0.jar`


## How to run:

1) start GUI on PC: `java -jar GUI/target/brickopore-1.0.jar`
2) click on "Start server"
3) on LEGO robot, run the client / connect to GUI: `client <IP of PC> 2424`
4) profit


## Modify GUI

- compile: `cd GUI`, then `mvn package`

## Modify C++ code on LEGO robot

- compile: install the EV3 C library: https://github.com/in4lio/ev3dev-c
- run: `make`
