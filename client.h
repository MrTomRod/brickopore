#pragma once

#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>
#include <array>
#include <exception>
#include <string>

enum class Command { Sequence, FindWhite, Nudge };
enum Color : int { Unknown = 0, Black, Blue, Green, Yellow, Red, White, Brown, Count };

class Ev3 {
 public:
  Ev3();
  ~Ev3();
};

class ColorHistogram {
 public:
  void addColor(const Color& color);
  void removeColor(const Color& color);
  Color getMostCommonColor() const;

 private:
  std::array<int, Color::Count> histogram_ = {};
};

class ServerIO {
  ServerIO(const std::string& serverName, const int portNumber);
  ~ServerIO();

  bool readNextCommand(int& additionalCommandData);
  int getNudgeDistance() const;

 private:
  Command currentCommand_;
  int nudgeFactor_;
  int sockfd_;
};

class Conveyer {
 public:
  Conveyer();

  bool isStopped() const;
  int position() const;
  void moveBy(const int distanceToMove) const;

 private:
  uint8_t tachoAddress_;
};

class ColorSensor {
 public:
  ColorSensor();

  Color getColor() const;

 private:
  uint8_t sensorAddress_;
};
