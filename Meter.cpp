/*
 * Meter.cpp
 *
 *  Created on: May 5, 2012
 *      Author: ubuntu
 */

#include "Meter.h"

Meter::Meter() : count(0) {

}

void Meter::increment() {
  count++;
}

uint32_t Meter::getCurrentValue() {
  return count;
}
