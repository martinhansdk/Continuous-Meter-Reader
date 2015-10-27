#include "gtest/gtest.h"
#include "arduino-mock/Arduino.h"
#include "RadioMessageSender.h"
#include "RadioMessageReceiver.h"
#include <cmath>

using namespace std;

namespace testing {

    const int METER_ID_1=1;
    const int SEQUENCE_1=67;

    const int METER_ID_1_2=1;
    const int SEQUENCE_1_2=68;

    const int METER_ID_2=2;
    const int SEQUENCE_2=30;

    const int METER_ID_3=3;
    const int SEQUENCE_3=3;

    const int METER_ID_4=4;
    const int SEQUENCE_4=4;

    /** structure to support the various tests.


      sender1 -----+-- radio1 ---- receiver1
                   |
      sender1_2 ---+
                   |
      sender2 -----+
                   |
      sender3 -----+

      sender4 -------- radio4

                       radio5 ----- receiver5

    */

    class RadioMessageTest : public testing::Test {
    protected:
        RF24 radio1, radio4, radio5; // this is the fake RF24 class from RF24.h in this directory
        RadioMessageSender sender1, sender1_2, sender2, sender3, sender4;
        RadioMessageReceiver<2, 255> receiver1, receiver5;

        RadioMessageTest() : sender1(radio1), 
                             sender1_2(radio1), 
                             sender2(radio1), 
                             sender3(radio1), 
                             sender4(radio4), 
                             receiver1(radio1),
                             receiver5(radio5) {
            sender1.begin(METER_ID_1, SEQUENCE_1);
            sender1_2.begin(METER_ID_1_2, SEQUENCE_1_2);
            sender2.begin(METER_ID_2, SEQUENCE_2);
            sender3.begin(METER_ID_3, SEQUENCE_3);
            sender4.begin(METER_ID_4, SEQUENCE_4);

        };

        void testMessageTransmission(RadioMessageSender &sender, RF24 &radio, string message) {
            transmit(sender, message);

            EXPECT_EQ(ceil((float)(message.length()) / (float)(32 - RADIO_HEADER_SIZE)), radio.chunks.size());
            EXPECT_EQ(message, radio.combinedMessage);
            EXPECT_EQ(message.length(), radio.combinedMessage.length());
        }

        void transmit(RadioMessageSender &sender, string message) {
            sender.startMessage();
            sender.write((uint8_t*) message.c_str(), message.length(), true);
        }

        template <typename T>
        void checkMessageReceived(T &receiver, string text) {
            ASSERT_TRUE(receiver.available());
            uint8_t *data;
            const MessageHeader *header;
            uint16_t bytes = receiver.read(data, header);

            EXPECT_EQ(text.length(), bytes);
            EXPECT_EQ(text, string((const char*)data, bytes));
        }

        template <typename T>
        void checkNoMoreMessages(T &receiver) {
            while(receiver.getRadio().available()) {
                ASSERT_FALSE(receiver.available());
            }
        }
    }; 

    TEST_F(RadioMessageTest, transmitsEmptyMessage) {
        sender1.startMessage();
        sender1.write((const uint8_t*)"", 0, true);
        EXPECT_EQ(1u, radio1.chunks.size());
        EXPECT_EQ("", radio1.combinedMessage);
        EXPECT_EQ(0u, radio1.combinedMessage.length());
    }


    TEST_F(RadioMessageTest, transmitsShortMessage) {
        testMessageTransmission(sender1, radio1, "x");
    }
 
    TEST_F(RadioMessageTest, allowsEmptyLastChunk) {
        sender1.startMessage();
        sender1.write((const uint8_t*)"banana", 6, false);
        sender1.write((const uint8_t*)"s", 1, false);
        sender1.write((const uint8_t*)"", 0, true);

        EXPECT_EQ(1u, radio1.chunks.size());
        EXPECT_EQ("bananas", radio1.combinedMessage);
        EXPECT_EQ(7u, radio1.combinedMessage.length());
    }

    TEST_F(RadioMessageTest, sendsOddSizeChunks) {
        sender1.startMessage();
        sender1.write((const uint8_t*)string(10, 'a').c_str(), 10, false);
        sender1.write((const uint8_t*)string(10, 'a').c_str(), 10, false);
        sender1.write((const uint8_t*)string(10, 'a').c_str(), 10, false);
        sender1.write((const uint8_t*)string(10, 'a').c_str(), 10, true);

        EXPECT_EQ(2u, radio1.chunks.size());
        EXPECT_EQ(string(40, 'a'), radio1.combinedMessage);
        EXPECT_EQ(40u, radio1.combinedMessage.length());
    }

	TEST_F(RadioMessageTest, transmitsLongMessage) {
        testMessageTransmission(sender1, radio1, "There was a fisherman named Fisher\n"
                "who fished for some fish in a fissure.\n"
                "Till a fish with a grin,\n"
                "pulled the fisherman in.\n"
                "Now they're fishing the fissure for Fisher.");
    }

    TEST_F(RadioMessageTest, transmitsAndReceivesSingleNode) {
        string text = "Fuzzy Wuzzy was a bear. Fuzzy Wuzzy had no hair. Fuzzy Wuzzy wasn't fuzzy, was he?";

        testMessageTransmission(sender1, radio1, text);

        for(unsigned int i=0 ; i < radio1.chunks.size() - 1 ; i++) {
            ASSERT_FALSE(receiver1.available());
        }

        checkMessageReceived(receiver1, text);

    }
  
    TEST_F(RadioMessageTest, acceptsMaximumLengthMessage) {
        string text = string("B", 255);

        testMessageTransmission(sender1, radio1, text);

        for(unsigned int i=0 ; i < radio1.chunks.size() - 1 ; i++) {
            ASSERT_FALSE(receiver1.available());
        }
        checkMessageReceived(receiver1, text);
    }

    TEST_F(RadioMessageTest, receivesTwoMessages) {
        string textA = string("A", 255);
        transmit(sender1, textA);

        for(unsigned int i=0 ; i < radio1.chunks.size() - 1 ; i++) {
            ASSERT_FALSE(receiver1.available());
        }
        checkMessageReceived(receiver1, textA);

        radio1.clear();
        string textB = string("B", 255);
        transmit(sender1, textB);

        for(unsigned int i=0 ; i < radio1.chunks.size() - 1 ; i++) {
            ASSERT_FALSE(receiver1.available());
        }
        checkMessageReceived(receiver1, textB);
    } 

    TEST_F(RadioMessageTest, handlesRepeatedChunks) {
        string textA = string("A", 255);
        transmit(sender1, textA);

        interleaveRadioTransmissions(radio5, radio1, radio1);

        for(unsigned int i=0 ; i < radio5.chunks.size() - 2 ; i++) {
            ASSERT_FALSE(receiver5.available());
        }
        checkMessageReceived(receiver5, textA);
        checkNoMoreMessages(receiver5);
    }

    TEST_F(RadioMessageTest, handlesNewSequence) {
        string textA = string("A", 5);
        transmit(sender1, textA);

        string textB = string("B", 5);
        transmit(sender1_2, textB);

        checkMessageReceived(receiver1, textA);
        checkMessageReceived(receiver1, textB);
        checkNoMoreMessages(receiver1);
    }

    TEST_F(RadioMessageTest, handlesTwoSenders) {
        string textA = string("A", 5);
        transmit(sender1, textA);

        string textB = string("B", 5);
        transmit(sender2, textB);

        checkMessageReceived(receiver1, textA);
        checkMessageReceived(receiver1, textB);
        checkNoMoreMessages(receiver1);
    }

    TEST_F(RadioMessageTest, ignoresSendersIfNoFreeSlot) {
        string textA = string("A", 5);
        transmit(sender1, textA);

        string textB = string("B", 5);
        transmit(sender2, textB);

        string textC = string("C", 5);
        transmit(sender3, textB);

        string textA2 = string("a", 5);
        transmit(sender1, textA2);

        checkMessageReceived(receiver1, textA);
        checkMessageReceived(receiver1, textB);
        ASSERT_FALSE(receiver1.available());
        checkMessageReceived(receiver1, textA2);
        checkNoMoreMessages(receiver1);
    }

    TEST_F(RadioMessageTest, handlesInterleaveFromDifferentSenders) {
        string textA = string("A", 255);
        transmit(sender1, textA);

        string textB = string("B", 255);
        transmit(sender4, textB);

        interleaveRadioTransmissions(radio5, radio1, radio4);
        for(unsigned int i=0 ; i < radio1.chunks.size() + radio4.chunks.size() - 2 ; i++) {
            ASSERT_FALSE(receiver5.available());
        }
        checkMessageReceived(receiver5, textA);
        checkMessageReceived(receiver5, textB);
        checkNoMoreMessages(receiver5);
    }
}  