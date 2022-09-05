#pragma once

#include <stdint.h>

class Conveyer {
 public:
  static constexpr float kTicksPerBlock = 47.0f;

  Conveyer();

  void setSlow() const;
  void setFast() const;

  void stop() const;
  bool isStopped() const;

  int getPosition() const;
  void moveBy(const int distanceToMove) const;

 private:
  uint8_t tachoAddress_;
};
