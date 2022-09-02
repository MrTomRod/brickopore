#pragma once

#include <iostream>

#include "ev3.h"
#include "ev3_port.h"
#include "ev3_sensor.h"
#include "ev3_tacho.h"

#include "client.h"

namespace {
void checkSensorReturn(const bool valid) {
  if (!valid) {
    throw std::runtime_error("Read/Write to sensor failed");
  }
}
} // namespace

Ev3::Ev3() {
  std::cout << "Setting up the EV3" << std::endl;
  if (ev3_init() < 1) {
    throw std::runtime_error("EV3 initialization failed");
  }

  if (ev3_sensor_init() < 0) {
    throw std::runtime_error("Sensor initialization failed");
  }
}

Ev3::~Ev3() {
  ev3_uninit();
}

void ColorHistogram::addColor(const Color& color) {
  ++histogram_[color];
}

void ColorHistogram::removeColor(const Color& color) {
  --histogram_[color];
}

Color ColorHistogram::getMostCommonColor() const {
  const int indexWithHigestCount =
      std::distance(histogram_.begin(), std::max_element(histogram_.begin(), histogram_.end()));
  return histogram_[indexWithHigestCount] > 0 ? static_cast<Color>(indexWithHigestCount)
                                              : Color::Unknown;
}

ServerIO::ServerIO(const std::string& serverName, const int portNumber) {
  struct sockaddr_in serverAddress;
  struct hostent* server;
  sockfd_ = socket(AF_INET, SOCK_STREAM, 0);

  std::cout << "Opening connection to " << serverName << " port " << portNumber << std::endl;

  if (sockfd_ < 0) {
    throw std::runtime_error("Cannot opening socket");
  }

  server = gethostbyname(serverName.c_str());

  if (server == nullptr) {
    throw std::runtime_error("error, no such host");
  }

  bzero((char*)&serverAddress, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;

  bcopy((char*)server->h_addr, (char*)&serverAddress.sin_addr.s_addr, server->h_length);
  serverAddress.sin_port = htons(portNumber);

  if (connect(sockfd_, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
    throw std::runtime_error("error during connection");
  }
}

ServerIO::~ServerIO() {
  close(sockfd_);
}

bool ServerIO::readNextCommand(int& additionalCommandData) {
  constexpr int kMaxCommandLength = 5;
  std::string commandString(kMaxCommandLength, 0);
  if (read(sockfd_, commandString.data(), kMaxCommandLength - 1) < 0) {
    throw std::runtime_error("Failed to read data from socket.");
  }

  std::cout << "Received command: " << commandString << std::endl;

  constexpr int kNudgeScaleFactor = 10;
  nudgeFactor_ = 0;
  if (commandString == "!SQ") {
    currentCommand_ = Command::Sequence;
  } else if (commandString == "!FW") {
    currentCommand_ = Command::FindWhite;
  } else if (commandString.rfind("!NB", 0) == 0) {
    currentCommand_ = Command::Nudge;
    nudgeFactor_ = -kNudgeScaleFactor * commandString.back();
  } else if (commandString.rfind("!NF", 0) == 0) {
    currentCommand_ = Command::Nudge;
    nudgeFactor_ = kNudgeScaleFactor * commandString.back();
  } else if (commandString == "!EX") {
    // should exit
    return false;
  } else {
    throw std::runtime_error("Invalid command");
  }

  return true;
}

int ServerIO::getNudgeDistance() const {
  return nudgeFactor_;
}

Conveyer::Conveyer() {
  std::cout << "Setting up conveyer" << std::endl;
  checkSensorReturn(ev3_search_tacho(LEGO_EV3_L_MOTOR, &tachoAddress_, 0))
      checkSensorReturn(set_tacho_stop_action_inx(tachoAddress_, TACHO_BRAKE));

  int maxSpeed;
  checkSensorReturn(get_tacho_max_speed(tachoAddress_, &maxSpeed));
  checkSensorReturn(set_tacho_speed_sp(tachoAddress_, maxSpeed / 4));
  checkSensorReturn(set_tacho_ramp_up_sp(tachoAddress_, 0));
  checkSensorReturn(set_tacho_ramp_down_sp(tachoAddress_, 0));
}

bool Conveyer::isStopped() const {
  int state;
  checkSensorReturn(get_tacho_state_flags(tachoAddress_, &state));
  return state == 0;
}

int Conveyer::position() const {
  int position;
  checkSensorReturn(get_tacho_position(tachoAddress_, &position));
  return position;
}

void Conveyer::moveBy(const int distanceToMove) const {
  checkSensorReturn(set_tacho_position_sp(tachoAddress_, distanceToMove));
  checkSensorReturn(set_tacho_command_inx(tachoAddress_, TACHO_RUN_TO_REL_POS));
}

ColorSensor::ColorSensor() {
  printf("Setting up color seensor\n");
  checkSensorReturn(ev3_search_sensor(LEGO_EV3_COLOR, &sensorAddress_, 0))
      checkSensorReturn(set_sensor_mode(sensorAddress_, "COL-COLOR"));
}

Color ColorSensor::getColor() const {
  int color;
  checkSensorReturn(get_sensor_value(0, sensorAddress_, &color));
  return static_cast<Color>(color);
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    std::cout << "Brickpore requires 2 arguments (hostname, port), exiting." << std::endl;
    return 0;
  }

  Ev3 ev3;
  Conveyer conveyer;
  ColorSensor colorSensor;

  ServerIO serverIo(argv[1], atoi(argv[2]));

  while (serverIo.readNextCommand()) {
    switch (command) {
      case Command::Nudge:
        conveyer.moveBy(serverIo.getNudgeDistance());
        break;
      default:
        std::cout << "Not implemented" << std::endl;
    }
  }

  return 0;
}
