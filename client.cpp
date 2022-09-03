#include <iostream>
#include <cmath>
#include <vector>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <exception>
#include <string>

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

bool startsWith(const std::string& full, const std::string& start) {
  return full.rfind(start, 0) == 0;
}

bool validDnaColor(const Color& color) {
  return color == Color::Red || color == Color::Green || color == Color::Blue ||
      color == Color::Yellow;
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

  std::cout << "Waiting for tacho motor..." << std::endl;
  while (ev3_tacho_init() < 1) {
    usleep(100000);
  }

  colorSensor_ = std::make_unique<ColorSensor>();
  conveyer_ = std::make_unique<Conveyer>();
}

Ev3::~Ev3() {
  ev3_uninit();
}

bool Ev3::findWhite() const {
  constexpr int kNumWhiteToDeclareFound = 10;
  constexpr int kMaxBlocks = 10;
  int numWhite = 0;

  conveyer_->setFast();
  conveyer_->moveBy(kTicksPerBlock * kMaxBlocks);
  while (!conveyer_->isStopped() && numWhite < kNumWhiteToDeclareFound) {
    numWhite += colorSensor_->getColor() == Color::White;
  }

  conveyer_->stop();

  if (numWhite == kNumWhiteToDeclareFound) {
    std::cout << "White found at position: " << conveyer_->getPosition() << std::endl;
    return true;
  } else {
    std::cout << "White was not found" << std::endl;
    return false;
  }
}

void Ev3::sequence(const ServerIO& serverIO) const {
  constexpr int kConsecutiveForColorChange = 25;
  constexpr int kMaxBlocks = 35;

  ColorDetector colorDetector(kConsecutiveForColorChange);

  bool haveSentFirstColor = false;
  const int startPosition = conveyer_->getPosition();
  int prevDetectionPosition = startPosition;

  conveyer_->setSlow();
  conveyer_->moveBy(kTicksPerBlock * kMaxBlocks);

  serverIO.sendSequenceStart();

  while (!conveyer_->isStopped()) {
    const Color detectedColor = colorDetector.getCurrentColor();
    const bool meetsDetectionCriteria = colorDetector.colorAboveDetectionThreshold();
    colorDetector.updateColorReading(colorSensor_->getColor());

    const bool colorChanged = colorDetector.getCurrentColor() != detectedColor;
    if (!meetsDetectionCriteria) {
      continue;
    }

    if(detectedColor == Color::White){
      prevDetectionPosition = conveyer_->getPosition();

      if (haveSentFirstColor) {
        std::cout << "Sequence end found." << std::endl;
        break;
      }
    }

    if (colorChanged && validDnaColor(detectedColor)){
      const int currentDetectionPosition = conveyer_->getPosition();
      const int sizeInBlocks = std::round((currentDetectionPosition - prevDetectionPosition) / kTicksPerBlock);
      if(sizeInBlocks > 0){
        std::cout << "Detected " << sizeInBlocks << " blocks of type " << detectedColor << std::endl;
        serverIO.sendColor(detectedColor, sizeInBlocks);
        haveSentFirstColor = true;
        prevDetectionPosition = currentDetectionPosition;
      } else {
        std::cout << "Detected a partial block of type " << detectedColor << ", ignoring" << std::endl;
      }
    }
  }

  conveyer_->stop();
  conveyer_->setFast();
  conveyer_->moveBy(startPosition - conveyer_->getPosition());

  serverIO.sendSequenceStop();
}

void Ev3::moveConveyerBy(const int distanceToMove) const {
  conveyer_->setFast();
  conveyer_->moveBy(distanceToMove);
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

bool ServerIO::readNextCommand() {
  constexpr int kMaxCommandLength = 5;
  std::string commandString(kMaxCommandLength, 0);
  if (read(sockfd_, &commandString[0], kMaxCommandLength - 1) < 0) {
    throw std::runtime_error("Failed to read data from socket.");
  }

  std::cout << "Received command: " << commandString << std::endl;

  constexpr int kNudgeScaleFactor = 10;
  nudgeFactor_ = 0;
  if (startsWith(commandString, "!SQ")) {
    currentCommand_ = Command::Sequence;
  } else if (startsWith(commandString, "!FW")) {
    currentCommand_ = Command::FindWhite;
  } else if (startsWith(commandString, "!PG")) {
    currentCommand_ = Command::Ping;
  } else if (startsWith(commandString, "!NB")) {
    currentCommand_ = Command::Nudge;
    nudgeFactor_ = -kNudgeScaleFactor * commandString[3];
  } else if (startsWith(commandString, "!NF")) {
    currentCommand_ = Command::Nudge;
    nudgeFactor_ = kNudgeScaleFactor * commandString[3];
  } else if (startsWith(commandString, "!EX")) {
    // should exit
    return false;
  } else {
    throw std::runtime_error("Invalid command");
  }

  if (currentCommand_ == Command::Nudge) {
    std::cout << "Set nudge factor to " << nudgeFactor_ << std::endl;
  }

  return true;
}

Command ServerIO::getCurrentCommand() const {
  return currentCommand_;
}

int ServerIO::getNudgeDistance() const {
  return nudgeFactor_;
}

  void ServerIO::sendSequenceStart() const{
    sendColor(Color::White, 3);
  }

  void ServerIO::sendColor(const Color& color, const int timesToSend) const{
    constexpr int kTimesEachColorMustBeSent = 20;

    std::vector<char> data;
    data.reserve(4 * kTimesEachColorMustBeSent * timesToSend);
    for(int i = 0; i < kTimesEachColorMustBeSent * timesToSend; ++i){
      data.push_back('!');
      data.push_back('D');
      data.push_back(color);
      data.push_back(0);
    }
    if(write(sockfd_, data.data(), data.size()) < 0){
      throw std::runtime_error("Failed to write data to socket.");
    }
  }

  void ServerIO::sendSequenceStop() const{
    const std::vector<char> stopData = {'!','S','T',0};
    if(write(sockfd_, stopData.data(), stopData.size()) < 0){
      throw std::runtime_error("Failed to write data to socket.");
    }
  }

Conveyer::Conveyer() {
  std::cout << "Setting up conveyer" << std::endl;
  checkSensorReturn(ev3_search_tacho(LEGO_EV3_L_MOTOR, &tachoAddress_, 0));
  std::cout << "Found tacho at: " << tachoAddress_ << std::endl;
  checkSensorReturn(set_tacho_stop_action_inx(tachoAddress_, TACHO_BRAKE));
  checkSensorReturn(set_tacho_ramp_up_sp(tachoAddress_, 0));
  checkSensorReturn(set_tacho_ramp_down_sp(tachoAddress_, 0));
  setSlow();
}

void Conveyer::setSlow() const{
  int maxSpeed;
  checkSensorReturn(get_tacho_max_speed(tachoAddress_, &maxSpeed));
  checkSensorReturn(set_tacho_speed_sp(tachoAddress_, maxSpeed / 8));
}

void Conveyer::setFast() const{
  int maxSpeed;
  checkSensorReturn(get_tacho_max_speed(tachoAddress_, &maxSpeed));
  checkSensorReturn(set_tacho_speed_sp(tachoAddress_, maxSpeed / 4));
}

void Conveyer::stop() const {
  checkSensorReturn(set_tacho_command_inx(tachoAddress_, TACHO_STOP));
}

bool Conveyer::isStopped() const {
  FLAGS_T state;
  checkSensorReturn(get_tacho_state_flags(tachoAddress_, &state));
  return state == 0;
}

int Conveyer::getPosition() const {
  int position;
  checkSensorReturn(get_tacho_position(tachoAddress_, &position));
  return position;
}

void Conveyer::moveBy(const int distanceToMove) const {
  checkSensorReturn(set_tacho_position_sp(tachoAddress_, distanceToMove));
  checkSensorReturn(set_tacho_command_inx(tachoAddress_, TACHO_RUN_TO_REL_POS));
}

ColorSensor::ColorSensor() {
  printf("Setting up color sensor\n");
  checkSensorReturn(ev3_search_sensor(LEGO_EV3_COLOR, &sensorAddress_, 0));
  std::cout << "Found color sensor at: " << sensorAddress_ << std::endl;
  char colorMode[] = "COL-COLOR";
  checkSensorReturn(set_sensor_mode(sensorAddress_, colorMode));
}

Color ColorSensor::getColor() const {
  int color;
  checkSensorReturn(get_sensor_value(0, sensorAddress_, &color));
  return static_cast<Color>(color);
}

ColorDetector::ColorDetector(const int& consecutiveReadingThreshold)
    : consecutiveReadingThreshold_(consecutiveReadingThreshold) {}

void ColorDetector::updateColorReading(const Color& color) {
  if (color == currentColor_) {
    ++consecutiveReadings_;
  } else {
    consecutiveReadings_ = 0;
    currentColor_ = color;
  }
}

Color ColorDetector::getCurrentColor() const {
  return currentColor_;
}

bool ColorDetector::colorAboveDetectionThreshold() const {
  return consecutiveReadings_ >= consecutiveReadingThreshold_;
}

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
