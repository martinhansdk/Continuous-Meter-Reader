#include <vector>
#include "gtest/gtest.h"
#include "CommunicateSerial.h"
#include "MeterReader_pb.h"


namespace testing {

class CommunicateSerialTest : public ::testing::Test {
  protected:
    TestStream stream;

    void sendGarbage(size_t len) {
      while(len--) {
        stream.write('G');
      }
    }
};

TEST_F(CommunicateSerialTest, canSendAndReceiveUpdate) {
  const uint32_t METER_ID = 1234567;
  const uint32_t SERIES_ID = 390123456;
  const uint64_t CURRENT_VALUE = 917253829100362;
  const size_t BEGIN_GARBAGE = 45;
  const size_t END_GARBAGE = 15;
  Receiver<MeterReader_Message, MeterReader_Message_fields> receiver(stream);

  sendGarbage(BEGIN_GARBAGE);
  sendCounterUpdate(stream, METER_ID, SERIES_ID, CURRENT_VALUE);
  sendGarbage(END_GARBAGE);

  EXPECT_GE((size_t)MeterReader_Message_size + BEGIN_GARBAGE + END_GARBAGE, stream.size());

  int tries_left = 500;
  bool received_a_message = false;
  while(tries_left--) {
    if(receiver.process()) {
      if(receiver.message.which_message == MeterReader_Message_log_tag) {
        cout << "Message, type " << receiver.message.message.log.type << " text=" << receiver.message.message.log.text << endl;
      }

      ASSERT_EQ(MeterReader_Message_update_tag, receiver.message.which_message);

      EXPECT_EQ(METER_ID, receiver.message.message.update.meterId);
      EXPECT_EQ(SERIES_ID, receiver.message.message.update.seriesId);
      EXPECT_EQ(CURRENT_VALUE, receiver.message.message.update.currentCounterValue);

      EXPECT_EQ(END_GARBAGE, (size_t)stream.available());
      EXPECT_FALSE(received_a_message) << "should only receive the message once";
      received_a_message = true;
    }
  }

  EXPECT_TRUE(received_a_message);
  EXPECT_NE(0, tries_left);
  EXPECT_EQ(0u, (size_t)stream.available());
}

TEST_F(CommunicateSerialTest, canSendAndReceiveSettings) {
  MeterReader_Settings settings;

  Receiver<MeterReader_Message, MeterReader_Message_fields> receiver(stream);

  sendSettings(stream, settings);

  EXPECT_GE((size_t)MeterReader_Message_size, stream.size());

  int tries_left = 500;
  bool received_a_message = false;
  while(tries_left--) {
    if(receiver.process()) {
      ASSERT_EQ(MeterReader_Message_settings_tag, receiver.message.which_message);
      EXPECT_FALSE(received_a_message) << "should only receive the message once";
      received_a_message = true;
    }
  }

  EXPECT_TRUE(received_a_message);
  EXPECT_NE(0, tries_left);
  EXPECT_EQ(0u, (size_t)stream.available());
}

TEST_F(CommunicateSerialTest, discardsWrongCRC) {
  Receiver<MeterReader_Message, MeterReader_Message_fields> receiver(stream);

  sendCounterUpdate(stream, 0, 0, 0);

  // corrupt the crc
  stream.write(stream.pop_last() ^ 0xFF);

  int tries_left = 500;
  while(tries_left--) {
    EXPECT_FALSE(receiver.process()) << "should not receive a message";
  }

  EXPECT_EQ(0u, (size_t)stream.available());
}

}