#!/bin/bash

astyle --mode=c --style=java --indent=spaces=2 --indent-classes --pad-oper --recursive --exclude='lib/RF24'  '*.cpp' '*.h' '*.ino' 
