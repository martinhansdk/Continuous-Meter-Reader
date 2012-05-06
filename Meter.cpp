/*
 * Meter.cpp
 *
 *  Created on: May 5, 2012
 *      Author: ubuntu
 */

#include "Meter.h"

Meter::Meter() : count(0), quantityPerIncrement(1.0) {

}

void Meter::increment() {
	count++;
}

void Meter::setQuantityPerIncrement(float amount){
	quantityPerIncrement = amount;
}

float Meter::getCurrentAmount() {
	return count*quantityPerIncrement;
}
