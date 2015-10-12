#ifndef _RF24_H
#define _RF24_H

#include <string>
#include <vector>

using namespace std;

/** A test fake for testing code that interacts with the RF24 library

*/
class RF24 {
public:
    vector<string> chunks;
    string combinedMessage;

    bool write(const void *buf, uint8_t len) {
        chunks.push_back(string((const char*)buf, len));

        combinedMessage += string(((const char*) buf) + 8, len - 8);

        return true;
    }
};

#endif