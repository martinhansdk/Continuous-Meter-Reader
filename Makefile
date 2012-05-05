BOARD = diecimila
PORT = /dev/ttyUSB0
LIB_DIRS = 
include Makefile.ubuntu
include Makefile.master

unittest:
	cd test ; $(MAKE) ; cd ..
	