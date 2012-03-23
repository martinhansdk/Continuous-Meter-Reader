BOARD = diecimila
PORT = /dev/ttyUSB0
LIB_DIRS = ../libraries/EDB ../libraries/Time ../libraries/TimeAlarms ../libraries/Messenger ../libraries/DS1621 $(ARD_LIBDIR)/Wire $(ARD_LIBDIR)/Wire/utility
include ../Makefile.ubuntu
include ../Makefile.master