#include "wSerial.h"

void wSerial::begin(long baud)
{
    Serial.begin(baud);
    SD.begin(CS_Pin);
    if (log2file)
    {
        logFile = SD.open("log.txt", FILE_WRITE);
        if (!logFile)
        {
            log2file = false;
            println("can't open SD file. => no log to file.");
        }
    }
}

void wSerial::print(String tmp2)
{
    String tmp;
    if(newline) {
        tmp = dTime.toString()+"=>" +tmp2;
    } else {
        tmp = tmp2;
    }
    
    Serial.print(tmp);
    Debug.printf(tmp.c_str());
    if (log2file)
    {
        logFile.print(tmp);
    }
    newline = false;
}
void wSerial::println(String tmp3)
{
    String tmp;
    if(newline) {
        tmp = dTime.toString()+"=>" +tmp3;
    } else {
        tmp = tmp3;
    }
    Serial.println(tmp);
    String tmp2 = tmp + "\n";
    Debug.printf(tmp2.c_str());
    if (log2file)
    {
        String tt;
        
        logFile.print(tt);
        logFile.println(tmp);
    }
    newline = true;
}
void wSerial::write(size_t tmp)
{
    Serial.write(tmp);
    /*if (log2file)
    {
        logFile.write(tmp);
    }*/
}
void wSerial::printf(char *tmp2, ...)
{
    va_list args;
    va_start(args, tmp2);

    String tmp = dTime.toString()+"=>" +String(tmp2);

    Serial.printf(tmp.c_str(), args);
    if (log2file)
    {
        logFile.printf(tmp.c_str(), args);
    }
    va_end(args);
    Debug.printf(tmp.c_str());
}
void wSerial::flush(){
    if(log2file) {
        println("flushed");
        logFile.flush();
    }
}
wSerial::wSerial(RemoteDebug &rd, bool write2file, strDateTime2 &dd) : Debug(rd), dTime(dd)
{
    log2file = write2file;
    newline = true;
}

