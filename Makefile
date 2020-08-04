BOARD := nano328
include /usr/share/arduino/Arduino.mk

tags: *.ino *.cpp *.h
	ctags -R .
