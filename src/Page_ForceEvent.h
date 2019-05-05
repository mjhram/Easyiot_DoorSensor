#ifndef PAGE_FORCEEVENT_H
#define PAGE_FORCEEVENT_H


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
	handleInterrupt();
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

#endif
