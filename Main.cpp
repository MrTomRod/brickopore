#include <iostream>
#include <string>

#include "ServerIO.h"
#include "Ev3.h"

int main(int argc, char* argv[]) {
  if (argc < 3) {
    std::cout << "Brickpore requires 2 arguments (hostname, port), exiting." << std::endl;
    return 0;
  }

  Ev3 ev3;
  ServerIO serverIO(argv[1], atoi(argv[2]));

  while (serverIO.readNextCommand()) {
    switch (serverIO.getCurrentCommand()) {
      case Command::Sequence:
        std::cout << "Sequencing" << std::endl;
        ev3.sequence(serverIO);
        break;
      case Command::FindWhite:
        std::cout << "Finding White" << std::endl;
        ev3.findWhite();
        break;
      case Command::Nudge:
        std::cout << "Nudging" << std::endl;
        ev3.moveConveyerBy(serverIO.getNudgeDistance());
        break;
      case Command::Ping:
        break;
      default:
        std::cout << "Not implemented" << std::endl;
    }
  }

  return 0;
}
