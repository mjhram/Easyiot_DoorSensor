#ifndef wSerial_h
#define wSerial_h

#include "Arduino.h"
#include "RemoteDebug.h"

class wSerial {
public: 
    RemoteDebug& Debug;
    wSerial(RemoteDebug& Db);
    void print(String tmp);
    void println(String tmp);
    void write(size_t tmp);
    void begin(long baud);
    void printf(char*, ...);
};



#endif