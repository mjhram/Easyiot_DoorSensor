#include "wSerial.h"

void wSerial::begin(long baud) {
    Serial.begin(baud);
}

void wSerial::print(String tmp) {
    Serial.print(tmp);
    DEBUG_V(tmp.c_str());
}
void wSerial::println(String tmp) {
    Serial.println(tmp);
    String tmp2 = tmp+"\n";
    DEBUG_V(tmp2.c_str());
}
void wSerial::write(size_t tmp) {
    Serial.write(tmp);    
}
void wSerial::printf(char* fmt, ...) {
    va_list args;
    va_start( args, fmt );

    Serial.printf(fmt, args);    
    va_end(args);
    DEBUG_V(fmt);
}
wSerial::wSerial(RemoteDebug& rd) : Debug(rd) {
        
}
