#ifndef PAGE_FORCEEVENT_H
#define PAGE_FORCEEVENT_H

#include <SD.h>

//
//   The HTML PAGE
//
const char PAGE_ForceEvent[] PROGMEM = R"=====(
<meta name="viewport" content="width=device-width, initial-scale=1" />
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<a href="admin.html"  class="btn btn--s"><</a>&nbsp;&nbsp;<strong>Force Event</strong>
<hr>
<form action="" method="post">
<table border="0"  cellspacing="0" cellpadding="3" style="width:310px" >

<tr><td colspan="2" align="center"><a href="javascript:GetState()" class="btn btn--m btn--blue">Force Event</a></td></tr>
<tr><td colspan="2" align="center"><a href="javascript:GetState2()" class="btn btn--m btn--blue">Reconnect MQTT</a></td></tr>
<tr>
	<td align="right">Topic</td>
	<td><input type="text" id="topic" name="topic" value="channels/772321/publish/1L8YUJBDUEJ8OFMR"></td>
</tr>
<tr>
	<td align="right">Value</td>
	<td><input type="text" id="pubstr" name="pubstr" value="field1=2"></td>
</tr>
<tr><td colspan="2" align="center"><input type="submit" style="width:150px" class="btn btn--m btn--blue" value="Publish"></td></tr>
</table>
</form>
<script>

function GetState()
{
	setValues("/force/opendoor");
}
function GetState2()
{
	setValues("/force/mqttconnect");
}

window.onload = function ()
{
	load("style.css","css", function() 
	{
		load("microajax.js","js", function() 
		{
				
		});
	});
}
function load(e,t,n){if("js"==t){var a=document.createElement("script");a.src=e,a.type="text/javascript",a.async=!1,a.onload=function(){n()},document.getElementsByTagName("head")[0].appendChild(a)}else if("css"==t){var a=document.createElement("link");a.href=e,a.rel="stylesheet",a.type="text/css",a.async=!1,a.onload=function(){n()},document.getElementsByTagName("head")[0].appendChild(a)}}



</script>
)=====" ;


//
// 
// 
void send_force_event_html (AsyncWebServerRequest *server)
{
	if (server->params() > 0 ) {
		String pubstr, topic;
		for ( uint8_t i = 0; i < server->params(); i++ ) {
			if (server->getParam(i)->name() == "topic"){
				topic = server->getParam(i)->value();
				continue; 
			}else if (server->getParam(i)->name() == "pubstr"){
				pubstr = server->getParam(i)->value();
				continue; 
			}
		}
		bool result = mqttClient.publish(topic.c_str(), 0, false, pubstr.c_str());
		wserial.println("topic:"+topic+" -- val:"+pubstr);
		wserial.println("res:"+String(result));		
	}
	server->send ( 200, "text/html", PAGE_ForceEvent ); 
	Serial.println(__FUNCTION__); 
}
void send_force_open_html (AsyncWebServerRequest *server)
{	
	handleInterruptQ(0);
	server->send ( 200);
	Serial.println(__FUNCTION__); 
}
void send_force_mqttconnect_html (AsyncWebServerRequest *server)
{

	mqttClient.disconnect();
	connectToMqtt();
	server->send ( 200);
	Serial.println(__FUNCTION__); 
}

void sdCheck() {
	Sd2Card card;
	SdVolume volume;
	SdFile root;
	int chipSelect = D8;

	
  wserial.print("\nInitializing SD card...");

  // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  if (!card.init(SPI_HALF_SPEED, chipSelect)) {
    wserial.println("initialization failed. Things to check:");
    wserial.println("* is a card inserted?");
    wserial.println("* is your wiring correct?");
    wserial.println("* did you change the chipSelect pin to match your shield or module?");
    while (1);
  } else {
    wserial.println("Wiring is correct and a card is present.");
  }

  // print the type of card
  wserial.println("");
  wserial.print("Card type:         ");
  switch (card.type()) {
    case SD_CARD_TYPE_SD1:
      wserial.println("SD1");
      break;
    case SD_CARD_TYPE_SD2:
      wserial.println("SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      wserial.println("SDHC");
      break;
    default:
      wserial.println("Unknown");
  }

  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) {
    wserial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    while (1);
  }

  wserial.print("Clusters:          ");
  wserial.println(String(volume.clusterCount()));
  wserial.print("Blocks x Cluster:  ");
  wserial.println(String(volume.blocksPerCluster()));

  wserial.print("Total Blocks:      ");
  wserial.println(String(volume.blocksPerCluster() * volume.clusterCount()));
  wserial.println("");

  // print the type and size of the first FAT-type volume
  uint32_t volumesize;
  wserial.print("Volume type is:    FAT");
  wserial.println(String(volume.fatType()));

  volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= volume.clusterCount();       // we'll have a lot of clusters
  volumesize /= 2;                           // SD card blocks are always 512 bytes (2 blocks are 1KB)
  wserial.print("Volume size (Kb):  ");
  wserial.println(String(volumesize));
  wserial.print("Volume size (Mb):  ");
  volumesize /= 1024;
  wserial.println(String(volumesize));
  wserial.print("Volume size (Gb):  ");
  wserial.println(String((float)volumesize / 1024.0));

  wserial.println("\nFiles found on the card (name, date and size in bytes): ");
  root.openRoot(volume);

  // list all files in the card with date and size
  root.ls(LS_R | LS_DATE | LS_SIZE);
}

void sendLogFile(AsyncWebServerRequest *request){
  	AsyncWebServerResponse *response = request->beginChunkedResponse("text/plain", [](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
		File logFile = SD.open("log.txt", FILE_READ);
		if (!logFile)
        {
            wserial.println("can't open SD file for read.");
			//File root = SD.open("/");
  			//printDirectory(root, 0);
			  sdCheck();
			return 0;
        }
		int next = logFile.read(buffer, maxLen);
		if(next == -1) {
			return 0;
		}
		return next;
	});
	request->send(response);
}
#endif
