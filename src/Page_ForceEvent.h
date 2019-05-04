#ifndef PAGE_FORCEEVENT_H
#define PAGE_FORCEEVENT_H


//
//   The HTML PAGE
//
const char PAGE_ForceEvent[] PROGMEM = R"=====(
<meta name="viewport" content="width=device-width, initial-scale=1" />
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<link rel="stylesheet" href="style.css" type="text/css" />
<script src="microajax.js"></script> 
<a href="admin.html"  class="btn btn--s"><</a>&nbsp;&nbsp;<strong>Force Event</strong>
<hr>
<table border="0"  cellspacing="0" cellpadding="3" style="width:310px" >


<tr><td colspan="2" align="center"><a href="javascript:GetState()" class="btn btn--m btn--blue">Force Event</a></td></tr>
</table>
<script>

function GetState()
{
	setValues("/force/opendoor");
}

window.onload = function ()
{
	load("style.css","css", function() 
	{
		load("microajax.js","js", function() 
		{
				GetState();
		});
	});
}
function load(e,t,n){if("js"==t){var a=document.createElement("script");a.src=e,a.type="text/javascript",a.async=!1,a.onload=function(){n()},document.getElementsByTagName("head")[0].appendChild(a)}else if("css"==t){var a=document.createElement("link");a.href=e,a.rel="stylesheet",a.type="text/css",a.async=!1,a.onload=function(){n()},document.getElementsByTagName("head")[0].appendChild(a)}}



</script>
)=====" ;


//
// FILL WITH INFOMATION
// 

void send_force_event_html (AsyncWebServerRequest *server)
{

	handleInterrupt();
	//server->send ( 200, "text/plain", values);
	Serial.println(__FUNCTION__); 
}


#endif
