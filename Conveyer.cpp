#include <exception>
#include <iostream>

#include "ev3.h"
#include "ev3_tacho.h"

#include "Conveyer.h"

namespace {
void checkReturn(const bool valid) {
  if (!valid) {
    throw std::runtime_error("Read/Write to conveyer failed");
  }
}
} // namespace

Conveyer::Conveyer() {
  std::cout << "Setting up conveyer" << std::endl;
  checkReturn(ev3_search_tacho(LEGO_EV3_L_MOTOR, &tachoAddress_, 0));
  std::cout << "Found tacho at: " << static_cast<int>(tachoAddress_) << std::endl;
  checkReturn(set_tacho_stop_action_inx(tachoAddress_, TACHO_BRAKE));
  checkReturn(set_tacho_ramp_up_sp(tachoAddress_, 0));
  checkReturn(set_tacho_ramp_down_sp(tachoAddress_, 0));
  setSlow();
}

void Conveyer::setSlow() const {
  int maxSpeed;
  checkReturn(get_tacho_max_speed(tachoAddress_, &maxSpeed));
  checkReturn(set_tacho_speed_sp(tachoAddress_, maxSpeed / 8));
}

void Conveyer::setFast() const {
  int maxSpeed;
  checkReturn(get_tacho_max_speed(tachoAddress_, &maxSpeed));
  checkReturn(set_tacho_speed_sp(tachoAddress_, maxSpeed / 4));
}

void Conveyer::stop() const {
  checkReturn(set_tacho_command_inx(tachoAddress_, TACHO_STOP));
}

bool Conveyer::isStopped() const {
  FLAGS_T state;
  checkReturn(get_tacho_state_flags(tachoAddress_, &state));
  return state == 0;
}

int Conveyer::getPosition() const {
  int position;
  checkReturn(get_tacho_position(tachoAddress_, &position));
  return position;
}

void Conveyer::moveBy(const int distanceToMove) const {
  checkReturn(set_tacho_position_sp(tachoAddress_, distanceToMove));
  checkReturn(set_tacho_command_inx(tachoAddress_, TACHO_RUN_TO_REL_POS));
}
