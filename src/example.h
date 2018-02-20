#ifndef PAGE_EXAMPLE_H
#define PAGE_EXAMPLE_H
//
//   The EXAMPLE PAGE
//
const char PAGE_example[] PROGMEM = R"=====(
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
  <link rel="stylesheet" href="/style.css" type="text/css" />
  <script src="/microajax.js"></script> <!-- Adding microajax  -->
  <h1>My Example goes here</h1>     
    
   <div id="mydynamicdata">Here comes the Dynamic Data in </div>   <!-- added a DIV, where the dynamic data goes to -->
   <script>                


		//window.onload = function ()
		{
			load("style.css","css", function() 
			{
				load("microajax.js","js", function() 
				{
						setValues("/admin/filldynamicdata");  //-- this function calls the function on the ESP      
				});
			});
		}
		function load(e,t,n){if("js"==t){var a=document.createElement("script");a.src=e,a.type="text/javascript",a.async=!1,a.onload=function(){n()},document.getElementsByTagName("head")[0].appendChild(a)}else if("css"==t){var a=document.createElement("link");a.href=e,a.rel="stylesheet",a.type="text/css",a.async=!1,a.onload=function(){n()},document.getElementsByTagName("head")[0].appendChild(a)}}



   </script>

)=====";
#endif


void filldynamicdata(AsyncWebServerRequest *server)
{        
    String values ="";
    values += "mydynamicdata|" + (String) + "This is filled by AJAX. Millis since start: " + (String) millis() + "|div\n";   // Build a string, like this:  ID|VALUE|TYPE
    server->send ( 200, "text/plain", values);   
}

void processExample(AsyncWebServerRequest *server)
{        
    if (server->params() > 0 )  // Are there any POST/GET Fields ? 
    {
       for ( uint8_t i = 0; i < server->params(); i++ ) {  // Iterate through the fields
          AsyncWebParameter* p = server->getParam(i);
            if (p->name().c_str() == "firstname") 
            {
                 // Your processing for the transmitted form-variable 
                 String fName = p->value().c_str();
            }
        }
    }
    server->send ( 200, "text/html", PAGE_example  ); 
}
