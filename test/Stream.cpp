#include <iostream>       // std::cout
#include <stdexcept>
#include "Stream.h"

size_t Stream::write(const char *str) {
  if (str == NULL) return 0;

  int count = 0;
  while(*str != '\0') {
    write(*str++);
    count++;
  }
  return count;
}

size_t Stream::write(const uint8_t* buf, int count) {
  int remaining = count;
  while(remaining--) {
    write(*buf++);
  }
  return count;
}

size_t Stream::readBytes(char *buffer, size_t length) {
  size_t count = 0;
  while (count < length) {
    int c = read();
    if (c < 0) break;
    *buffer++ = (char)c;
    count++;
  }
  return count;
}

int TestStream::available() {
  return q.size();
}

int TestStream::read() {
  if(q.empty()) {
    throw runtime_error("Called read on a stream where nothing is available.");
    return -1;
  }
  int val = q.front();
  q.pop_front();
  return val;
}

size_t TestStream::write(uint8_t val) {
  q.push_back(val);
  return 1;
}

uint8_t TestStream::pop_last() {
  uint8_t val = q.back();
  q.pop_back();
  return val;
}

