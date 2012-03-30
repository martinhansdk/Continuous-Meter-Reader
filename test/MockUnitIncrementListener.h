
#ifndef _MOCKUNITINCREMENTLISTENER_H
#define _MOCKUNITINCREMENTLISTENER_H

#include "gmock/gmock.h"
#include "UnitIncrementListener.h"

class MockUnitIncrementListener : public UnitIncrementListener {
 public:
  MOCK_METHOD0(increment,
      void());
};

#endif
