#ifndef _RF24_H
#define _RF24_H

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

using namespace std;

/** A test fake for testing code that interacts with the RF24 library

*/
class RF24 {
public:
	unsigned int chunksRead;
    vector<string> chunks;
    string combinedMessage;

    RF24() : chunksRead(0) {}

    void clear() {
    	chunksRead = 0;
    	chunks.clear();
    	combinedMessage.clear();
    }

    bool write(const void *buf, uint8_t len) {
        chunks.push_back(string((const char*)buf, len));
        combinedMessage += string(((const char*) buf) + 8, len - 8);

        return true;
    }

    bool available() {
    	return chunks.size() > chunksRead;
    }

    uint8_t getDynamicPayloadSize() {
    	return chunks[chunksRead].size();
    }

    void read(void *buf, uint8_t len) {
    	uint8_t readLen = min(chunks[chunksRead].size(), (unsigned long)len);
    	memcpy(buf, chunks[chunksRead++].c_str(), readLen);
    }
};


// interleave chunks from two source radios onto one receiver radio
void interleaveRadioTransmissions(RF24 &target, RF24 &source1, RF24 &source2) {
	vector<string>::const_iterator s1 = source1.chunks.begin();
	vector<string>::const_iterator s2 = source2.chunks.begin();

	while(s1 != source1.chunks.end() && s2 != source2.chunks.end()) {
		if(s1 != source1.chunks.end()) {
			target.chunks.push_back(*s1);
			++s1;
		}
		if(s2 != source1.chunks.end()) {
			target.chunks.push_back(*s2);
			++s2;
		}
	}	
}

#endif