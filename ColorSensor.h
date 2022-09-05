#pragma once

#include <stdint.h>

enum Color : int { Unknown = 0, Black, Blue, Green, Yellow, Red, White, Brown };

class ColorSensor {
 public:
  ColorSensor();

  Color getColor() const;

 private:
  uint8_t sensorAddress_;
};
