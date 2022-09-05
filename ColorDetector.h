#pragma once

#include "ColorSensor.h"

class ColorDetector {
 public:
  ColorDetector(const int& consecutiveReadingThreshold);

  void updateColorReading(const Color& color);

  Color getCurrentColor() const;
  bool colorAboveDetectionThreshold() const;

 private:
  const int consecutiveReadingThreshold_;
  Color currentColor_ = Color::Unknown;
  int consecutiveReadings_ = 0;
};
