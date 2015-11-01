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

void Meter::add(uint64_t amount) {
  count += amount;
}

uint64_t Meter::getCurrentValue() {
  return count;
}
