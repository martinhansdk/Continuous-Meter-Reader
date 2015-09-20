#include "gtest/gtest.h"
#include "settings.h"

namespace testing {

    class TestConfig {
    public:
        int32_t a;
        uint32_t b;
        uint64_t c;
    };

    TEST(SettingsTest, canSaveAndRestore) {
        Settings<TestConfig> settings;
        settings.s.a = -1000;
        settings.s.b = 500000;
        settings.s.c = 34965772976;

        settings.save();

        Settings<TestConfig> settings2;
        settings2.s.a = -3;
        settings2.s.b = 3;
        settings2.s.c = 7;
        settings2.load();

        EXPECT_EQ(settings.s.a, settings2.s.a);
        EXPECT_EQ(settings.s.b, settings2.s.b);
        EXPECT_EQ(settings.s.c, settings2.s.c);
      }


  }