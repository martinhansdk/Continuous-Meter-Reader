
/**
 UnitIncrementListener interface that must be implemented by all
classes that want to subscribe to sample events.

*/

#ifndef _UNITINCREMENTLISTENER_H
#define _UNITINCREMENTLISTENER_H

class UnitIncrementListener {
  virtual ~UnitIncrementListener() {};
  virtual void increment() = 0;
}

#endif
