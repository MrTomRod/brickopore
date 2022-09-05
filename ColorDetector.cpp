#include "ColorDetector.h"

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
