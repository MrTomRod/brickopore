#pragma once

#include <memory>

enum class Command { Sequence, FindWhite, Nudge, Ping };
enum Color : int { Unknown = 0, Black, Blue, Green, Yellow, Red, White, Brown };

constexpr float kTicksPerBlock = 47.0f;

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

class Conveyer {
 public:
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

class ColorSensor {
 public:
  ColorSensor();

  Color getColor() const;

 private:
  uint8_t sensorAddress_;
};

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
