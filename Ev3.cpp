#include <unistd.h>
#include <cmath>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "ev3.h"
#include "ev3_sensor.h"
#include "ev3_tacho.h"
#include "ev3_port.h"

#include "Conveyer.h"
#include "ColorSensor.h"
#include "ColorDetector.h"
#include "Ev3.h"

namespace {
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
  conveyer_->moveBy(Conveyer::kTicksPerBlock * kMaxBlocks);
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
  constexpr int kConsecutiveForColorChange = 10;
  constexpr int kMaxBlocks = 35;

  ColorDetector colorDetector(kConsecutiveForColorChange);

  bool haveSentFirstColor = false;
  const int startPosition = conveyer_->getPosition();
  int prevDetectionPosition = startPosition;

  conveyer_->setSlow();
  conveyer_->moveBy(Conveyer::kTicksPerBlock * kMaxBlocks);

  serverIO.sendSequenceStart();

  while (!conveyer_->isStopped()) {
    const Color detectedColor = colorDetector.getCurrentColor();
    const bool meetsDetectionCriteria = colorDetector.colorAboveDetectionThreshold();
    colorDetector.updateColorReading(colorSensor_->getColor());

    const bool colorChanged = colorDetector.getCurrentColor() != detectedColor;
    if (!meetsDetectionCriteria) {
      continue;
    }

    if (detectedColor == Color::White) {
      prevDetectionPosition = conveyer_->getPosition();

      if (haveSentFirstColor) {
        std::cout << "Sequence end found." << std::endl;
        break;
      }
    }

    if (colorChanged && validDnaColor(detectedColor)) {
      const int currentDetectionPosition = conveyer_->getPosition();
      const float sizeInBlocks =
          (currentDetectionPosition - prevDetectionPosition) / Conveyer::kTicksPerBlock;
      std::cout << "Detected " << sizeInBlocks << " blocks of type " << detectedColor << std::endl;
      const int sizeInWholeBlocks =
          sizeInBlocks + 0.7; // ugly hack, blocks tend to be detected slightly too small
      if (sizeInWholeBlocks > 0) {
        serverIO.sendColor(detectedColor, sizeInWholeBlocks);
        haveSentFirstColor = true;
        prevDetectionPosition = currentDetectionPosition;
      } else {
        std::cout << "Detected a partial block of type " << detectedColor << ", ignoring"
                  << std::endl;
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
