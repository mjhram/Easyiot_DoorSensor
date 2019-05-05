//
//  HTML PAGE
//

const char PAGE_AdminGeneralSettings[] PROGMEM =  R"=====(
<meta name="viewport" content="width=device-width, initial-scale=1" />
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<a href="admin.html"  class="btn btn--s"><</a>&nbsp;&nbsp;<strong>General Settings</strong>
<hr>
<form action="" method="post">
<table border="0"  cellspacing="0" cellpadding="3" >
<tr>
	<td align="right">Name of Device</td>
	<td><input type="text" id="devicename" name="devicename" value=""></td>
</tr>

<tr>
	<td align="left" colspan="2"><hr></td>
</tr>

<tr><td align="center"><a id="disarm" name="disarm" href="javascript:Disarm()" style="width:150px" class="btn btn--m btn--blue">Disarm</a></td></tr>

<tr>
	<td align="left" colspan="2"><hr></td>
</tr>

<tr>
	<td align="left" colspan="2">Turn on at</td>
</tr>
<tr>
	<td align="right"> Enabled:</td>
	<td><input type="checkbox" id="tonenabled" name="tonenabled"></td>
</tr>

<tr>
	<td align="right"> Time:</td>
	<td><input type="text" id="tonhour" name="tonhour" size="2" value="00">:<input type="text" id="tonminute" name="tonminute" size="2" value="00"></td>
</tr>

<tr>
	<td align="left" colspan="2">Turn off at:</td>
<tr>
	<td align="right"> Enabled:</td>
	<td><input type="checkbox" id="toffenabled" name="toffenabled"></td>
</tr>
<tr>
	<td align="right"> Time:</td>
	<td><input type="text" id="toffhour" name="toffhour" size="2" value="00">:<input type="text" id="toffminute" name="toffminute" size="2" value="00"></td>
</tr>
<tr><td colspan="2" align="center"><input type="submit" style="width:150px" class="btn btn--m btn--blue" value="Save"></td></tr>
</table>
</form>
<script>

 
function Disarm()
{
	setValues("/admin/disarm");
}
window.onload = function ()
{
	load("style.css","css", function() 
	{
		load("microajax.js","js", function() 
		{
				setValues("/admin/generalvalues");
		});
	});
}
function load(e,t,n){if("js"==t){var a=document.createElement("script");a.src=e,a.type="text/javascript",a.async=!1,a.onload=function(){n()},document.getElementsByTagName("head")[0].appendChild(a)}else if("css"==t){var a=document.createElement("link");a.href=e,a.rel="stylesheet",a.type="text/css",a.async=!1,a.onload=function(){n()},document.getElementsByTagName("head")[0].appendChild(a)}}



</script>
)=====";


// Functions for this Page
void send_devicename_value_html(AsyncWebServerRequest *server)
{
		
	String values ="";
	values += "devicename|" + (String) config.DeviceName + "|div\n";
	server->send ( 200, "text/plain", values);
	Serial.println(__FUNCTION__); 
	
}

void send_general_html(AsyncWebServerRequest *server)
{
	
	if (server->params() > 0 )  // "Save" is clicked
	{
		config.AutoTurnOn = false;
		config.AutoTurnOff = false;
		String temp = "";
		for ( uint8_t i = 0; i < server->params(); i++ ) {
			if (server->getParam(i)->name() == "devicename") config.DeviceName = urldecode(server->getParam(i)->value()); 
			if (server->getParam(i)->name() == "tonenabled") config.AutoTurnOn = true; 
			if (server->getParam(i)->name() == "toffenabled") config.AutoTurnOff = true; 
			if (server->getParam(i)->name() == "tonhour") config.TurnOnHour =  server->getParam(i)->value().toInt(); 
			if (server->getParam(i)->name() == "tonminute") config.TurnOnMinute =  server->getParam(i)->value().toInt(); 
			if (server->getParam(i)->name() == "toffhour") config.TurnOffHour =  server->getParam(i)->value().toInt(); 
			if (server->getParam(i)->name() == "toffminute") config.TurnOffMinute =  server->getParam(i)->value().toInt(); 
		}
		WriteConfig();
		firstStart = true;
	}
	server->send ( 200, "text/html", PAGE_AdminGeneralSettings ); 
	Serial.println(__FUNCTION__); 
	
	
}

void send_general_configuration_values_html(AsyncWebServerRequest *server)
{
	String values ="";
	values += "devicename|" +  (String)  config.DeviceName +  "|input\n";
	values += "disarm|" +  (String)  (disarm?"Arm":"Disarm") +  "|div\n";
	values += "tonhour|" +  (String)  config.TurnOnHour +  "|input\n";
	values += "tonminute|" +   (String) config.TurnOnMinute +  "|input\n";
	values += "toffhour|" +  (String)  config.TurnOffHour +  "|input\n";
	values += "toffminute|" +   (String)  config.TurnOffMinute +  "|input\n";
	values += "toffenabled|" +  (String) (config.AutoTurnOff ? "checked" : "") + "|chk\n";
	values += "tonenabled|" +  (String) (config.AutoTurnOn ? "checked" : "") + "|chk\n";
	server->send ( 200, "text/plain", values);
	Serial.println(__FUNCTION__); 
}

void send_disarm_values_html(AsyncWebServerRequest *server){
	String values ="";
	values += "disarm|" +  (String)  (disarm?"Disarm":"Arm") +  "|div\n";
	server->send ( 200, "text/plain", values);
	Serial.println(__FUNCTION__); 
	disarm = !disarm;
	Serial.print("Disarm=");
	Serial.println(disarm);
}
