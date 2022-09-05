#pragma once

#include "ColorSensor.h"

enum class Command { Sequence, FindWhite, Nudge, Ping };

class ServerIO {
 public:
  ServerIO(const std::string& serverName, const int portNumber);
  ~ServerIO();

  bool readNextCommand();
  Command getCurrentCommand() const;
  int getNudgeDistance() const;

  void sendSequenceStart() const;
  void sendColor(const Color& color, const int timesToSend) const;
  void sendSequenceStop() const;

 private:
  Command currentCommand_;
  int nudgeFactor_;
  int sockfd_;
};
