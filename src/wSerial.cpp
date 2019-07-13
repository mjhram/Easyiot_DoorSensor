#include "wSerial.h"

void wSerial::begin(long baud)
{
    Serial.begin(baud);
    SD.begin(CS_Pin);
    filenameSuffix ="";
    reopen();
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
    if (logFile)
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
    if (logFile)
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
    if (logFile)
    {
        logFile.printf(tmp.c_str(), args);
    }
    va_end(args);
    Debug.printf(tmp.c_str());
}

void wSerial::close(){
    if(logFile) {
        println("closing log file");
        logFile.close();
        log2file = false;
    } 
}

void wSerial::open() {
    log2file = true;    
    String filename = filenameSuffix+ String(".log");
    println(String("opening log file:")+String(filename));
    logFile = SD.open(filename, FILE_WRITE);
    if (!logFile)
    {
        println("can't open SD file. => no log to file.");
        String tmp(logFile.name());
        println(tmp);
    }
}

void wSerial::reopen(){
    //String dt = (String)dTime.year+ "_" +(String)dTime.month+ "_" +(String)dTime.day;
    if(log2file && dTime.year > 2000){
        char date[10];
        sprintf(date,"%04d%02d%02d",dTime.year,dTime.month,dTime.day);
        String dt = String(date);
        if(!filenameSuffix.equals(dt)) {
            filenameSuffix = dt;
            if(logFile) {
                print("close log file...");
                logFile.close();
                println("closed");
            }
            open();
            return;
        }
    } 
    
    if(logFile) {
        print("flush log file...");
        logFile.flush();
        println("flushed");
    } else if (log2file)
    {
        open();  
    }      
}

/*void wSerial::flush(){
    if(logFile) {
        println("flushed");
        logFile.flush();
    }
}*/
wSerial::wSerial(RemoteDebug &rd, bool write2file, strDateTime2 &dd) : Debug(rd), dTime(dd)
{
    log2file = write2file;
    newline = true;
}

