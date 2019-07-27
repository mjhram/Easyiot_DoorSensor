Notes:
1.Shami Water Level threshold values: 
    A.GND-R10k-out-SHAMI-VCC
        int th_min[]={0,   300, 500, 600, 650, 900};
        int th_max[]= {50, 400, 550, 640, 800, 1024};
    B.GND-SHAMI-out-R10k-VCC
        int th[]={,   , , , 720, };

2.GPIO0, GPIO02 couldn't be used (since they are used at startup): even after using one of them as ground and the other as input.

3. In case of internet is lost (for example reseting the PowerBeam), the MQTTClient keeps reconnecting without any sucess.
    This issue happens due to Null IP (no IP obtained from DNS).
    Solved: by adding 2nd DNS server (Google DNS 8.8.8.8) using WiFi.config

4.extending GPIO by using MCP23017: under process


