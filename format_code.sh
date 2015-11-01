#!/bin/bash

astyle --mode=c --style=java --indent=spaces=2 --indent-classes --pad-oper lib/*/*.h lib/*/*.cpp src/*/*.h src/*/*.cpp src/*/*.ino
