#include <vector>
#include "gtest/gtest.h"
#include "CommunicateWireless.h"
#include "RadioMessageReceiver.h"
#include "MeterReader_pb.h"
#include "pb_decode.h"


namespace testing {

    TEST(CommunicateWirelessTest, canSendAndReceiveUpdate) {
        const uint32_t METER_ID = 1234567;
        const uint32_t SERIES_ID = 390123456;
        const uint64_t CURRENT_VALUE = 917253829100362;

        RF24 radio;
        RadioMessageSender sender(radio);
        RadioMessageReceiver<4, MeterReader_Message_size> receiver(radio);

        sendCounterUpdateByRadio(sender, METER_ID, SERIES_ID, CURRENT_VALUE);

        ASSERT_TRUE(receiver.available());

        uint8_t *data;
        
        const MessageHeader *header;
        uint8_t len = receiver.read(data, header);

        ASSERT_EQ(radio.combinedMessage.length(), len);

        MeterReader_Message message;

        pb_istream_t stream = pb_istream_from_buffer(data, len);
        
        // Now we are ready to decode the message. 
        bool ok = pb_decode(&stream, MeterReader_Message_fields, &message);

		EXPECT_TRUE(ok);

		EXPECT_EQ(MeterReader_Message_update_tag, message.which_message);
        EXPECT_EQ(METER_ID, message.message.update.meterId);
        EXPECT_EQ(SERIES_ID, message.message.update.seriesId);
        EXPECT_EQ(CURRENT_VALUE, message.message.update.currentCounterValue);
    }
}