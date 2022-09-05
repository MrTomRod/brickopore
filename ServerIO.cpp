#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <exception>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
#include <cstring>

#include "ServerIO.h"

namespace {
bool startsWith(const std::string& full, const std::string& start) {
  return full.rfind(start, 0) == 0;
}
} // namespace

ServerIO::ServerIO(const std::string& serverName, const int portNumber) {
  sockfd_ = socket(AF_INET, SOCK_STREAM, 0);

  std::cout << "Opening connection to " << serverName << " port " << portNumber << std::endl;

  if (sockfd_ < 0) {
    throw std::runtime_error("Cannot opening socket");
  }

  const struct hostent* server = gethostbyname(serverName.c_str());

  if (server == nullptr) {
    throw std::runtime_error("error, no such host");
  }

  struct sockaddr_in serverAddress{};
  const socklen_t addressLength = sizeof serverAddress;
  serverAddress.sin_family = AF_INET;

  std::memcpy(&serverAddress.sin_addr.s_addr, server->h_addr, server->h_length);
  serverAddress.sin_port = htons(portNumber);

  if (connect(sockfd_, std::reinterpret_cast<struct sockaddr*>(&serverAddress), addressLength) < 0) {
    throw std::runtime_error("error during connection");
  }
}

ServerIO::~ServerIO() {
  close(sockfd_);
}

bool ServerIO::readNextCommand() {
  constexpr int kMaxCommandLength = 5;
  std::string commandString(kMaxCommandLength, 0);
  if (read(sockfd_, &commandString[0], kMaxCommandLength - 1) < 0) {
    throw std::runtime_error("Failed to read data from socket.");
  }

  std::cout << "Received command: " << commandString << std::endl;

  constexpr int kNudgeScaleFactor = 10;
  nudgeFactor_ = 0;
  if (startsWith(commandString, "!SQ")) {
    currentCommand_ = Command::Sequence;
  } else if (startsWith(commandString, "!FW")) {
    currentCommand_ = Command::FindWhite;
  } else if (startsWith(commandString, "!PG")) {
    currentCommand_ = Command::Ping;
  } else if (startsWith(commandString, "!NB")) {
    currentCommand_ = Command::Nudge;
    nudgeFactor_ = -kNudgeScaleFactor * commandString[3];
  } else if (startsWith(commandString, "!NF")) {
    currentCommand_ = Command::Nudge;
    nudgeFactor_ = kNudgeScaleFactor * commandString[3];
  } else if (startsWith(commandString, "!EX")) {
    // should exit
    return false;
  } else {
    throw std::runtime_error("Invalid command");
  }

  if (currentCommand_ == Command::Nudge) {
    std::cout << "Set nudge factor to " << nudgeFactor_ << std::endl;
  }

  return true;
}

Command ServerIO::getCurrentCommand() const {
  return currentCommand_;
}

int ServerIO::getNudgeDistance() const {
  return nudgeFactor_;
}

void ServerIO::sendSequenceStart() const {
  sendColor(Color::White, 3);
}

void ServerIO::sendColor(const Color& color, const int timesToSend) const {
  constexpr int kTimesEachColorMustBeSent = 20;

  std::vector<char> data;
  data.reserve(4 * kTimesEachColorMustBeSent * timesToSend);
  for (int i = 0; i < kTimesEachColorMustBeSent * timesToSend; ++i) {
    data.push_back('!');
    data.push_back('D');
    data.push_back(color);
    data.push_back(0);
  }
  if (write(sockfd_, data.data(), data.size()) < 0) {
    throw std::runtime_error("Failed to write data to socket.");
  }
}

void ServerIO::sendSequenceStop() const {
  const std::vector<char> stopData = {'!', 'S', 'T', 0};
  if (write(sockfd_, stopData.data(), stopData.size()) < 0) {
    throw std::runtime_error("Failed to write data to socket.");
  }
}
