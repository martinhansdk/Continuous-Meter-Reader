#include "gtest/gtest.h"
#include "arduino-mock/Arduino.h"
#include "RadioMessageSender.h"
#include <cmath>

using namespace std;

namespace testing {

    const int METER_ID=5;
    const int SEQUENCE=67;

    class RadioMessageTest : public testing::Test {
    protected:
        RF24 radio; // this is the fake RF24 class from RF24.h in this directory
        RadioMessageSender sender;

        RadioMessageTest() : sender(METER_ID, SEQUENCE, radio) {};

        void testMessageTransmission(string message) {
            sender.startMessage();
            sender.write((uint8_t*) message.c_str(), message.length(), true);

            EXPECT_EQ(ceil((float)(message.length()) / (float)(32 - HEADER_SIZE)), radio.chunks.size());
            EXPECT_EQ(message, radio.combinedMessage);
        } 
    };

    TEST_F(RadioMessageTest, doesNotTransmitEmptyMessage) {
        testMessageTransmission("");
    }


    TEST_F(RadioMessageTest, transmitsShortMessage) {
        testMessageTransmission("x");
    }

	TEST_F(RadioMessageTest, transmitsLongMessage) {
        testMessageTransmission("There was a fisherman named Fisher\n"
                "who fished for some fish in a fissure.\n"
                "Till a fish with a grin,\n"
                "pulled the fisherman in.\n"
                "Now they're fishing the fissure for Fisher.");
    }

} 