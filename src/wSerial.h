#ifndef wSerial_h
#define wSerial_h

#include <Arduino.h>
#include <SD.h>
#include "strDateTime.h"
#include "RemoteDebug.h"
#define CS_Pin D8

class wSerial {
    private:
    boolean newline;
public: 
    strDateTime2 &dTime;
    RemoteDebug& Debug;
    bool log2file;
    File logFile;

    wSerial(RemoteDebug& Db, bool log2file, strDateTime2& dd);
    void print(String tmp);
    void println(String tmp);
    void write(size_t tmp);
    void begin(long baud);
    void printf(char*, ...);
    void flush();
};

#endif

