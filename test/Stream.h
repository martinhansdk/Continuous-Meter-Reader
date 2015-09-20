#ifndef __STREAM_H
#define __STREAM_H

#include <cstddef>
#include <stdint.h>
#include "deque"

using namespace std;

class Stream {
public:
	virtual int available() = 0;
	virtual int read() = 0;
	virtual size_t write(uint8_t) = 0;
	virtual size_t write(const char *);
	virtual size_t write(const uint8_t*, int);
    size_t readBytes( char *buffer, size_t length); // read chars from stream into buffer
  // terminates if length characters have been read or timeout (see setTimeout)
  // returns the number of characters placed in the buffer (0 means no valid data found)

};

class TestStream : public Stream {
	deque<uint8_t> q;
public:
	virtual int available();
	virtual int read();
	virtual size_t write(uint8_t);
	uint8_t pop_last();

	size_t size() {
		return q.size();
	}
};


#endif