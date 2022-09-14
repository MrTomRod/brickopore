## Introduction

This repo combines richardmleggett's two LEGO sequencer repos:

- https://github.com/richardmleggett/brickopore
- https://github.com/richardmleggett/BrickoporeGUI

The blueprint for the robot can be found here: [brickopore.co.uk](http://brickopore.co.uk/technology/)

Unfortunately, many parts are not being produced anymore, so use [bricklink.com](https://www.bricklink.com/) to purchase
them. Instead of manually searching all necessary parts, simply [upload](https://www.bricklink.com/help.asp?helpID=207)
this [part list](parts_list/bricklink-order.xml)!

There are many things one could do with the sequencer. We used it at the Night of Research (Nacht der
Forschung) at the Univesity of Bern. As an inspiration, the tasks we set for our visitors are
outlined [here](nacht_der_forschung/readme.md).

## Improvements over richardmleggett's code

- The C code that runs on the LEGO EV3 was re-written in C++
- A few tweaks to so that the accuracy of the readout no longer depends on the
  serial links speed
- Allows for "reads" of more than 24 nucleotides
- A "Copy Sequence" button was added to the Java GUI
- Doubled screen width, matched letter size

## Notes

- The compiled binary is the file `client` in the root directory
- The compiled GUI code is located in `GUI/target/brickopore-1.0.jar`

## How to run

1) start GUI on PC: `java -jar GUI/target/brickopore-1.0.jar`
2) click on "Start server"
3) on LEGO robot, run the client / connect to GUI: `client <IP of PC> 2424`
4) profit

## Modify GUI

1) install Java and Maven
2) `cd GUI`
3) recompile the Java code using `mvn package`

## Modify C++ code on LEGO robot

1) install the EV3 C library: https://github.com/in4lio/ev3dev-c
2) run `make` to compile the code
