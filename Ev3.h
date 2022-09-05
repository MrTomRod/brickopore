#pragma once

#include <memory>

#include "ServerIO.h"

class ColorSensor;
class Conveyer;

class Ev3 {
 public:
  Ev3();
  ~Ev3();

  void sequence(const ServerIO& serverIO) const;
  bool findWhite() const;
  void moveConveyerBy(const int distanceToMove) const;

 private:
  std::unique_ptr<ColorSensor> colorSensor_;
  std::unique_ptr<Conveyer> conveyer_;
};
