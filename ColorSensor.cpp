#include <iostream>

#include "ev3.h"
#include "ev3_sensor.h"

#include "ColorSensor.h"


namespace {
void checkReturn(const bool valid) {
  if (!valid) {
    throw std::runtime_error("Read/Write to color sensor failed");
  }
}
} // namespace

ColorSensor::ColorSensor() {
  std::cout << "Setting up color sensor" << std::endl;
  checkReturn(ev3_search_sensor(LEGO_EV3_COLOR, &sensorAddress_, 0));
  std::cout << "Found color sensor at: " << static_cast<int>(sensorAddress_) << std::endl;
  char colorMode[] = "COL-COLOR";
  checkReturn(set_sensor_mode(sensorAddress_, colorMode));
}

Color ColorSensor::getColor() const {
  int color;
  checkReturn(get_sensor_value(0, sensorAddress_, &color));
  return static_cast<Color>(color);
}
