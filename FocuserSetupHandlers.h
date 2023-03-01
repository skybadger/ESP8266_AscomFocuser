/*
File to be included into relevant device REST setup 
*/
//Assumes Use of ARDUINO ESP8266WebServer for entry handlers
#if !defined _FOCUSERsetupHandlers_H_
#define _FOCUSERsetupHandlers_H_
#include "JSONHelperFunctions.h"

//Setup page Function definitions
bool getUriField( char* inString, int searchIndex, String& outRef );
String& setupFormBuilderHeader( String& htmlForm );
String& setupFormBuilderDeviceHeader( String& htmlForm, String& errMsg );
String& setupFormBuilderDeviceStrings( String& htmlForm );
String& setupFormBuilderDriver0Header( String& htmlForm, String& errMsg );
String& setupFormBuilderDriver0Resolution( String& htmlForm );
String& setupFormBuilderDriver0Limits( String& htmlForm );
//String& setupFormBuilderDriver0Brightness( String& htmlForm );
String& setupFormBuilderDriver0Positions( String& htmlForm );
String& setupFormBuilderFooter( String& htmlForm );
String& setupFormBuilder( String& htmlForm, String& errMsg );


//Handlers used for setup webpage.
//Device specific 
void handlerDeviceSetup(void);
void handleSetup             ( void );
void handlerDeviceHostname(void) ;
void handlerDeviceLocation(void) ;
void handlerDeviceUdpPort(void) ;
void handlerSetupHostnamePut  ( void );

//Driver specific - there may be multiple drivers on this device. 
void handlerDriver0Setup(void);
void handlerDriver0TemperatureSource(void);
void handlerDriver0Limits(void) ;
void handleSetupPositionPut ( void );
void handlerDriver0Setup( void);
void handlerDriver0(void);
void handlerDriver0Motor(void);
void handlerDriver0Rotation(void);
void handlerDriver0Position(void);
void handlerDriver0Backlash(void);
void handlerDriver0Limits(void);

/* Handlers to do custom setup that can't be done without a windows ascom driver setup form. 
 */
 void handlerDeviceSetup(void)
 {
    String message, timeString, err= "";
    int returnCode = 400;

    if ( server.method() == HTTP_GET )
    {
        returnCode = 200;
        err = "";
    }
      
    //Send large pages in chunks
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    setupFormBuilderHeader( message ); 
    server.send( returnCode, "text/html", message );

    setupFormBuilderDeviceHeader( message, err );      
    server.sendContent( message );
    
    setupFormBuilderDeviceStrings( message );      
    server.sendContent( message );

    setupFormBuilderFooter( message );
    server.sendContent( message );
 }

void handlerDriver0Setup(void)
 {
    String message, timeString, err= "";
    int returnCode = 400;

    if ( server.method() == HTTP_GET )
    {
        returnCode = 200;
        err = "";
    }
    
    //Send large pages in chunks
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    setupFormBuilderHeader( message );      
    server.send( returnCode, "text/html", message );
   
    setupFormBuilderDriver0Header( message, err );
    server.sendContent( message );
    
    setupFormBuilderDriver0Limits( message );            
    server.sendContent( message );

    setupFormBuilderDriver0Positions( message );
    server.sendContent( message );
    
    setupFormBuilderDriver0Resolution( message );            
    server.sendContent( message );
    
    setupFormBuilderFooter( message );
    server.sendContent( message );
 }
  
 /* 
  *  Handler to update the hostname from the form.
  */
 void handlerDeviceHostname(void) 
 {
    String message, timeString, err = "";
   
    int returnCode = 400;
    String argToSearchFor[] = { "hostname", "numSwitches"};
     
    if ( server.method() == HTTP_POST || server.method() == HTTP_PUT || server.method() == HTTP_GET)
    {
        if( hasArgIC( argToSearchFor[0], server, false )  )
        {
          String newHostname = server.arg(argToSearchFor[0]) ;
          //process form variables.
          if( newHostname.length() > 0 && newHostname.length() < MAX_NAME_LENGTH-1 )
          {
            //process new hostname
            strncpy( myHostname, newHostname.c_str(), MAX_NAME_LENGTH );
            saveToEeprom();
            returnCode = 200;
          }
          else
          {
            err = "New name too long";
            returnCode = 401;
          }
        }
    }
    else
    {
      returnCode = 400;
      err = "Bad HTTP request verb";
    }
    
    if ( returnCode == 200 )
    {
      server.sendHeader( WiFi.hostname().c_str(), String("/status"), true);
      server.send ( 302, F("text/html"), F("<!Doctype html><html>Redirecting for restart</html>") );
      DEBUGSL1("Reboot requested");
      device.restart();
    }
    else
    {
      //Send large pages in chunks
      server.setContentLength(CONTENT_LENGTH_UNKNOWN);
      setupFormBuilderHeader( message );      
      server.send( returnCode, "text/html", message );
  
      setupFormBuilderDeviceHeader( message, err );      
      server.sendContent( message );

      setupFormBuilderDeviceStrings( message );      
      server.sendContent( message );

      setupFormBuilderFooter( message );
      server.sendContent( message );
    }
 }

 /* 
  *  Handler to set the Focuser temperature source.
  */
 void handlerDriver0TemperatureSource(void) 
 {
    String message, timeString, err = "";
   
    int returnCode = 400;
    String argToSearchFor[] = { "mqttPath" };
     
    if ( server.method() == HTTP_POST || server.method() == HTTP_PUT || server.method() == HTTP_GET)
    {
        if( hasArgIC( argToSearchFor[0], server, false )  )
        {
          String newMqttPath = server.arg(argToSearchFor[0]) ;
          //process form variables.
          if( newMqttPath.length() > 0 && newMqttPath.length() < MAX_NAME_LENGTH-1 )
          {
            //process new hostname
            strncpy( mqttTemperatureSource, newMqttPath.c_str(), MAX_NAME_LENGTH );
            saveToEeprom();
            returnCode = 200;
			//unsubscribe from previous data source
			if( mqttTemperatureSource != nullptr )
				client.unsubscribe( mqttTemperatureSource );	
			//subscribe in addition
			client.subscribe( mqttTemperatureSource );			
          }
          else
          {
            err = "New MQTT path too long";
            returnCode = 401;
          }
        }
    }
    else
    {
      returnCode=400;
      err = "Bad HTTP request verb";
    }
    
	  /*
	  server.sendHeader( WiFi.hostname().c_str(), String("/status"), true);
      server.send ( 302, F("text/html"), F("<!Doctype html><html>Redirecting for restart</html>"));
      DEBUGSL1("Reboot requested");
      device.restart();
	  */

  //Send large pages in chunks
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  setupFormBuilderHeader( message );      
  server.send( returnCode, "text/html", message );

  setupFormBuilderDeviceHeader( message, err );      
  server.sendContent( message );

  setupFormBuilderDeviceStrings( message );      
  server.sendContent( message );

  setupFormBuilderFooter( message );
  server.sendContent( message );
 
 }

 /* 
  *  Handler to update the hostname from the form.
  */
 void handlerDeviceLocation(void) 
 {
    String message, timeString, err = "";
   
    int returnCode = 400;
    String argToSearchFor[] = { "location" };
    
    if ( server.method() == HTTP_POST || server.method() == HTTP_PUT || server.method() == HTTP_GET)
    {
        if( hasArgIC( argToSearchFor[0], server, false )  )
        {
          String newLocation = server.arg(argToSearchFor[0]) ;
          debugD( "SetLocation:  %s", newLocation.c_str() );

          //process form variables.
          if( newLocation.length() > 0 && newLocation.length() < MAX_NAME_LENGTH-1 )
          {
            //process new hostname
            strncpy( Location, newLocation.c_str(), min( (int) MAX_NAME_LENGTH, (int) newLocation.length()) );
            saveToEeprom();
            returnCode = 200;
          }
          else
          {
            err = "New name too long";
            returnCode = 401;
          }
        }
    }
    else
    {
      returnCode=400;
      err = "Bad HTTP request verb";
    }
    
      //Send large pages in chunks
      server.setContentLength(CONTENT_LENGTH_UNKNOWN);
      setupFormBuilderHeader( message );      
      server.send( returnCode, "text/html", message );
  
      setupFormBuilderDeviceHeader( message, err );      
      server.sendContent( message );

      setupFormBuilderDeviceStrings( message );      
      server.sendContent( message );

      setupFormBuilderFooter( message );
      server.sendContent( message );
 }



void handlerDeviceUdpPort(void) 
 {
    String message, timeString, err = "";
   
    int returnCode = 400;
    String argToSearchFor[] = { "disfocuseryport"};
     
    if ( server.method() == HTTP_POST || server.method() == HTTP_PUT || server.method() == HTTP_GET)
    {
        if( hasArgIC( argToSearchFor[0], server, false )  )
        {
          int newPort = server.arg(argToSearchFor[0]).toInt() ;
          //process form variables.
          if( newPort > 1024 && newPort < 65536  )
          {
            //process new hostname
            udpPort = newPort;
            saveToEeprom();
            returnCode = 200;
          }
          else
          {
            err = "New focuse discovery port value out of range ";
            returnCode = 401;
          }
        }
    }
    else
    {
      returnCode=400;
      err = "Bad HTTP request verb";
    }
    
    //Send large pages in chunks
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    setupFormBuilderHeader( message );      
    server.send( returnCode, "text/html", message );

    setupFormBuilderDeviceHeader( message, err );      
    server.sendContent( message );
    
    setupFormBuilderDeviceStrings( message );      
    server.sendContent( message );

    setupFormBuilderFooter( message );
    server.sendContent( message );
 }

 /* 
  *  Handler to update the focuser limits from the form.
  */
 void handlerDriver0Limits(void) 
 {
    String message, timeString, err = "";
    int newMinLimit = -1;
    int newMaxLimit = -1;
    int returnCode = 400;
    String argToSearchFor[] = { "minLimit", "maxLimit" };
     
    if ( server.method() == HTTP_POST || server.method() == HTTP_PUT || server.method() == HTTP_GET)
    {
        String minArg = argToSearchFor[0];
        String maxArg = argToSearchFor[1];
        if( hasArgIC( minArg, server, false )  &&  hasArgIC( maxArg, server, false )  )
        {
          newMinLimit = server.arg( minArg ).toInt();
          newMaxLimit = server.arg( maxArg ).toInt();
          debugD( "New focuser limit values: %s: %d %s: %d ", minArg.c_str(), newMinLimit, maxArg.c_str(), newMaxLimit );
           
          if( newMinLimit >= minStepDefault && newMinLimit <= maxStepDefault &&
              newMaxLimit >= minStepDefault && newMaxLimit <= maxStepDefault ) 
          {
            debugI( "new focuser limit values: %d %d ", newMinLimit, newMaxLimit );
            minStepLimit = newMinLimit;
            maxStepLimit = newMaxLimit;
          }
          else /*outside limits*/
          {
            returnCode = 400;
            err = "Invalid values - one or more outside allowed range";
          }          
        }
        else
        {
            returnCode = 402;
            err = "Invalid arguments - one or more not found ";          
        }
      //Save the outputs 
      returnCode = 200;    
      saveToEeprom();
    }
    else
    {
      returnCode = 400;
      err = "Bad HTTP request verb";
    }

      //Send large pages in chunks
      server.setContentLength(CONTENT_LENGTH_UNKNOWN);
      setupFormBuilderHeader( message );      
      server.send( returnCode, "text/html", message );
  
      setupFormBuilderDriver0Header( message, err );      
      server.sendContent( message );

      setupFormBuilderDriver0Limits( message );            
      server.sendContent( message );

      setupFormBuilderDriver0Positions( message );            
      server.sendContent( message );
      
      //setupFormBuilderDriver0Brightness( message );            
      //server.sendContent( message );
      
      setupFormBuilderFooter( message );
      server.sendContent( message );
    return;
 }


/* 
  *  Handler to update the focuser current position from the form.
  */
 void handlerDriver0Position(void) 
 {
    String message, timeString, err = "";
    int newPosition = minStep;
    int tempPosition = 0;
     
    int returnCode = 400;
    String argToSearchFor[] = { "focuserposition", };
     
   if ( server.method() == HTTP_POST || server.method() == HTTP_PUT || server.method() == HTTP_GET)
    {
        String newPositionArg = argToSearchFor[0];
        if( hasArgIC( newPositionArg, server, false ) )
        {        
          newPosition = server.arg( newPositionArg ).toInt();
          debugD( "New focuser position: %d", newPosition );
          if( !absMode ) 
          { 
            tempPosition = newPosition + position + absPosition;
          }
          
          if( tempPosition >= minStepLimit && tempPosition <= maxStepLimit ) 
          {      			
      			if ( focuserState == FocuserStates::FOCUSER_IDLE )
      			{
      			   if( absMode) 
      			      position = newPosition;	
               else 
                  position = position + newPosition;
      			   focuserCmd = FocuserCmds::FOCUSER_MOVE;
               targetFocuserState = FocuserStates::FOCUSER_MOVING;
      			}           
          }
          else /* outside position limits */
          {
            returnCode = 400;
            err = "Invalid values - one or more outside allowed range";
          }                  
        }
        else
        {
            returnCode = 402;
            err = "Invalid arguments - one or more not found ";          
        }
      //Save the ouputs 
      returnCode = 200;    
      saveToEeprom();
    }
    else
    {
      returnCode = 400;
      err = "Bad HTTP request verb";
    }

    //Send large pages in chunks
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    setupFormBuilderHeader( message );      
    server.send( returnCode, "text/html", message ); 
    message = "";
    
    setupFormBuilderDriver0Header( message, err );      
    server.sendContent( message );
    message = "";
    
    setupFormBuilderDriver0Limits( message );            
    server.sendContent( message );
    message = "";
    
    setupFormBuilderDriver0Positions( message );            
    server.sendContent( message );
    message = "";

    //setupFormBuilderDriver0Brightness( message );            
    //server.sendContent( message );
    //message = "";
    
    setupFormBuilderFooter( message );
    server.sendContent( message );
    return;
 }

/* 
  *  Handler to setup the motor parameters used in the focuser. 
  *  resolution- steps or encoder bits per rev - somewhere between 48 and 2400
  *  distance - how far a single rev advances the focuser - typically around 2mm. 
  *  The number of microsteps per step - set to 1 for using encoders, otherwise use the setting for the Focuser driver chip. Typically powers of 2 up to 256. 
  */
 void handlerDriver0Motor(void) 
 {
    String message, timeString, err = "";
    int newPosition = minStep;
    float newDistance = 0.0F;
     
    int returnCode = 400;
    String argToSearchFor[] = { "motorType", "resolution", "distance", "microsteps", "stepsPerRev" };
     
   if ( ( server.method() == HTTP_POST || server.method() == HTTP_PUT || server.method() == HTTP_GET ) && server.args() >= 4 ) 
   {
      String newMotorType;
      int newResolution = 0;
   		int newMicrosteps = 0;
      int newStepsPerRev = 0;
		  int argCount = 0;    
      
      if( hasArgIC( argToSearchFor[0], server, false ) )
      {
        newMotorType = server.arg( argToSearchFor[0] );
        if ( newMotorType.equalsIgnoreCase( "Focuser" ) || newMotorType.equalsIgnoreCase( "DC brushed" ) )
        {
          debugD( "New focuser motor type: %s: valid values are 'DC brushed' , 'Focuser' ", newMotorType.c_str() );
        }
        argCount ++;
      }
      if( hasArgIC( argToSearchFor[1], server, false ) )
      {
        int local = argToSearchFor[1].toInt();
		if( local >0 && local <= 32000 )
		{	
		 newResolution   = local;
         argCount ++;
		}
      }
      if( hasArgIC( argToSearchFor[2], server, false ) )
      {
          //Todo
		  newDistance   = (float) argToSearchFor[2].toDouble();
          argCount ++;
      }
      if( hasArgIC( argToSearchFor[3], server, false ) )
      {
          //todo
		  newMicrosteps = argToSearchFor[3].toInt();
          argCount ++;
      }
      if( hasArgIC( argToSearchFor[4], server, false ) )
      {
          //todo
		  newStepsPerRev = argToSearchFor[4].toInt();
          argCount ++;     
      }

      //Save the ouputs ?
      if ( argCount == 4 ) 
        saveToEeprom();
           
      returnCode = 200;    
      
    }
    else
    {
      returnCode = 400;
      err = "Bad HTTP request verb";
    }

    //Send large pages in chunks
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    setupFormBuilderHeader( message );      
    server.send( returnCode, "text/html", message ); 
    message = "";
    
    setupFormBuilderDriver0Header( message, err );      
    server.sendContent( message );
    message = "";
    
    setupFormBuilderDriver0Limits( message );            
    server.sendContent( message );
    message = "";
    
    setupFormBuilderDriver0Positions( message );            
    server.sendContent( message );
    message = "";

    //setupFormBuilderDriver0Brightness( message );            
    //server.sendContent( message );
    //message = "";
    
    setupFormBuilderFooter( message );
    server.sendContent( message );
    return;
 }

//Non ASCOM functions
/*
 * String& setupFormBuilder( String& htmlForm )
 */
 void handleSetup(void)
 {
  String output = "";
  String err = "";
  output = setupFormBuilder( output, err );
  server.send(200, "text/html", output );
  return;
 }

//Don't forget MQTT ID aligns with hostname too 
void handleSetupHostnamePut( void ) 
{
  String form;
  String errMsg;
  String newName;
  
  debugURI( errMsg );
  DEBUGSL1 (errMsg);
  DEBUGSL1( "Entered handleSetupHostnamePut" );
  
  //throw error message
  if( server.hasArg("hostname"))
  {
    newName = server.arg("hostname");
    DEBUGS1( "new hostname:" );DEBUGSL1( newName );
  }
  if( newName != NULL && newName.length() != 0 &&  newName.length() < MAX_NAME_LENGTH )
  {
    //save new hostname and cause reboot - requires eeprom read at setup to be in place.  
    memcpy( myHostname, newName.c_str(),  min( sizeof(newName.c_str()),(unsigned) MAX_NAME_LENGTH ) );
    memcpy( thisID, newName.c_str(),  min( sizeof(newName.c_str()),(unsigned) MAX_NAME_LENGTH ) );  
    server.send( 200, "text/html", "rebooting!" ); 

    //Write to EEprom
    saveToEeprom();
    device.restart();
  }
  else
  {
    errMsg = "handleSetupHostnamePut: Error handling new hostname";
    DEBUGSL1( errMsg );
    form = setupFormBuilder( form, errMsg );
    server.send( 200, "text/html", form ); 
  }
}

//Don't forget MQTT ID aligns with hostname too 
void handleSetupTempSourcePut( void ) 
{
  String form;
  String errMsg;
  String newName;
  
  debugURI( errMsg );
  DEBUGSL1 (errMsg);
  DEBUGSL1( "Entered handleSetupHostnamePut" );
  
  //throw error message
  if( server.hasArg("hostname"))
  {
    newName = server.arg("hostname");
    DEBUGS1( "new hostname:" );DEBUGSL1( newName );
  }
  if( newName != NULL && newName.length() != 0 &&  newName.length() < MAX_NAME_LENGTH )
  {
    //save new hostname and cause reboot - requires eeprom read at setup to be in place.  
    memcpy( myHostname, newName.c_str(),  min( sizeof(newName.c_str()),(unsigned) MAX_NAME_LENGTH ) );
    memcpy( thisID, newName.c_str(),  min( sizeof(newName.c_str()),(unsigned) MAX_NAME_LENGTH ) );  
    server.send( 200, "text/html", "rebooting!" ); 

    //Write to EEprom
    saveToEeprom();
    device.restart();
  }
  else
  {
    errMsg = "handleSetupHostnamePut: Error handling new hostname";
    DEBUGSL1( errMsg );
    form = setupFormBuilder( form, errMsg );
    server.send( 200, "text/html", form ); 
  }
}

/*
 Handler for setup dialog - issue call to <hostname>/setup and receive a webpage
 Fill in the form and submit and handler for each form button will store the variables and return the same page.
 optimise to something like this:
 https://circuits4you.com/2018/02/04/esp8266-ajax-update-part-of-web-page-without-refreshing/
 Bear in mind HTML standard doesn't support use of PUT in forms and changes it to GET so arguments get sent in plain sight as 
 part of the URL.
 */
String& setupFormBuilderHeader( String& htmlForm )
{
  String hostname = WiFi.hostname();
  
/*
<!DocType html>
<html lang=en >
<head>
<meta charset="utf-8"/>
<meta name="viewport" content="width=device-width, initial-scale=1"/>
<link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/4.3.1/css/bootstrap.min.css">
<script src="https://ajax.googleapis.com/ajax/libs/jquery/3.4.1/jquery.min.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.14.7/umd/popper.min.js"></script>
<script src="https://maxcdn.bootstrapcdn.com/bootstrap/4.3.1/js/bootstrap.min.js"></script>
<script>function setTypes( a ) {   var searchFor = "types"+a;  var x = document.getElementById(searchFor).value;  if( x.indexOf( "PWM" ) > 0 || x.indexOf( "DAC" ) > 0 )  {      document.getElementById("pin"+a).disabled = false;      document.getElementById("min"+a).disabled = false;      document.getElementById("max"+a).disabled = false;      document.getElementById("step"+a).disabled = false;  }  else  {      document.getElementById("pin"+a).disabled = true;      document.getElementById("min"+a).disabled = true;      document.getElementById("max"+a).disabled = true;      document.getElementById("step"+a).disabled = true;  }}</script>
</meta>
<style>
legend { font: 10pt;}
h1 { margin-top: 0; }
form {
    margin: 0 auto;
    width: 500px;
    padding: 1em;
    border: 1px solid #CCC;
    border-radius: 1em;
}
div+div { margin-top: 1em; }
label span {
    display: inline-block;
    width: 150px;
    text-align: right;
}
input, textarea {
    font: 1em sans-serif;
    width: 150px;
    box-sizing: border-box;
    border: 1px solid #999;
}
input[type=checkbox], input[type=radio], input[type=submit] {
    width: auto;
    border: none;
}
input:focus, textarea:focus { border-color: #000; }
textarea {
    vertical-align: top;
    height: 5em;
    resize: vertical;
}
fieldset {
    width: 410px;
    box-sizing: border-box;
    border: 1px solid #999;
}
button { margin: 20px 0 0 124px; }
label {  position: relative; }
label em {
  position: absolute;
  right: 5px;
  top: 20px;
}
</style>
</head>
*/
 
  htmlForm =  F( "<!DocType html><html lang=en ><head><meta charset=\"utf-8\">");
  htmlForm += F("<meta name=\"viewport\" content=\"width=device-width, initial-scale=\"1\">");
  htmlForm += F("<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.4.1/css/bootstrap.min.css\">");
  htmlForm += F("<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.5.1/jquery.min.js\"></script>");
  htmlForm += F("<script src=\"https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.14.7/umd/popper.min.js\"></script>");
  htmlForm += F("<script src=\"https://maxcdn.bootstrapcdn.com/bootstrap/3.4.1/js/bootstrap.min.js\"></script>");
  
//CSS Style settings
  htmlForm += F( "<style>\
legend { font: 10pt;}\
body { bgcolor:'dark gray'; } \
h1 { margin-top: 0; }\
form { margin: 0 auto; width: 500px;padding: 1em;border: 1px solid #CCC;border-radius: 1em;}\
div+div { margin-top: 1em; }\
label span{display: inline-block;width: 150px;text-align: right;}\
input, textarea {font: 1em sans-serif;width: 150px;box-sizing: border-box;border: 1px solid #999;}\
input[type=checkbox], input[type=radio], input[type=submit]{width: auto;border: 1px;}\
input:focus,textarea:focus{border-color:#000;}\
input[type='number']{width: 60px;} \
textarea {vertical-align: top;height: 5em;resize: vertical;}\
fieldset {width: 410px;box-sizing: border-box;border: 1px solid #999;}\
button {margin: 20px 0 0 124px;}\
label {position:relative;}\
label em { position: absolute;right: 5px;top: 20px;}\
</style> \
</head>");

  return htmlForm;
}

String& setupFormBuilderDeviceHeader( String& htmlForm, String& errMsg )  
{
  htmlForm =  F( "<body><div class=\"container\">" );
  htmlForm += F( "<div class=\"row\" id=\"topbar-device\" bgcolor='A02222'> ");
  htmlForm += F( "<p> This is the <b><i>device setup </i></b>page for the <a href=\"http://www.skybadger.net\">Skybadger</a> <a href=\"https://www.ascom-standards.org\">ASCOM</a> device '");
  htmlForm += myHostname;
  htmlForm += F("' which implements the <a href=\"https://www.ascom-standards.org/api\">ALPACA</a> v1.0 API</b></p> </div>");

  if( errMsg != NULL && errMsg.length() > 0 ) 
  {
    htmlForm += F( "<div class=\"row\" id=\"device-errorbar\" bgcolor='lightred'> <b>Error Message</b><pre>" );
    htmlForm += errMsg;
    htmlForm += F( "</pre></div>" );
  }
  return htmlForm;
}

String& setupFormBuilderDriver0Header( String& htmlForm, String& errMsg )  
{
  //
  htmlForm =  F("<body><div class=\"container\">");
  htmlForm += F("<div class=\"row\" id=\"topbar-driver\" bgcolor='A02222'>");
  htmlForm += F("<p> This is the <b><i>driver setup </i></b> page for the <a href=\"http://www.skybadger.net\">Skybadger</a> <a href=\"https://www.ascom-standards.org\">ASCOM</a> focuser Calibrator device hosted on '");
  htmlForm += myHostname;
  htmlForm += F("' which implements the <a href=\"https://www.ascom-standards.org/api\">ALPACA</a> v1.0 API</b></p> </div>");

  if( errMsg != NULL && errMsg.length() > 0 ) 
  {
    htmlForm += F("<div class=\"row\" id=\"driver-errorbar\" bgcolor='lightred'> <b>Error Message</b><pre>");
    htmlForm += errMsg;
    htmlForm += F("</pre></div>");
    //htmlForm += "<hr>";
  }
  return htmlForm;
}

String& setupFormBuilderDeviceStrings( String& htmlForm )
{
  String hostname = WiFi.hostname();
  int i;
    
  //UDP Disfocusery port 
  //Device instance number
  htmlForm = F("<div class=\"row\" id=\"topbar-strings\">");
  htmlForm += F("<div class=\"col-sm-12\">");
  htmlForm += F("<p>This device supports the <a href=\"placeholder\">ALPACA UDP disfocusery API</a> on port: ");
  htmlForm.concat( udpPort);
  
  //Links to setup pages for each implemented driver
  htmlForm += F("</p> <p> This device implements drivers with driver IDs : <ul><li>");
  //TODO - handle multiple GUIDs for multiple devices. 
  htmlForm.concat( GUID );
  htmlForm += F(" <a href=\"/setup/v1/focusercalibrator/0/setup\"> setup focuser 0 </a>");
  htmlForm += F("</li></ul></p></div></div>");
  
  htmlForm += "<div class=\"row\" id=\"discovery-port\" >";
  htmlForm += "<div class=\"col-sm-12\"><h2> Enter new Discovery port number for device</h2>";
  htmlForm += "<form method=\"POST\" action=\"http://";
  htmlForm.concat( myHostname );
  htmlForm += "/setup/udpport\">";
  htmlForm += "<label for=\"udpport\" id=\"udpport\"> Port number to use for Management API discovery </label>";
  htmlForm += "<input type=\"number\" name=\"udpport\" min=\"1024\" max=\"65535\" ";
  htmlForm += "value=\"";
  htmlForm.concat( udpPort );
  htmlForm += "\"/>";
  htmlForm += "<input type=\"submit\" value=\"Set port\" />";
  htmlForm += "</form></div></div>"; 
  
  //Device settings - hostname 
  htmlForm += "<div class=\"row float-left\" id=\"set-hostname\">";
  htmlForm += "<div class=\"col-sm-12\"><h2> Enter new hostname for device</h2>";
  htmlForm += "<p>Changing the hostname will cause the device to reboot and may change the IP address!</p>";
  htmlForm += "<form method=\"POST\" id=\"hostname\" action=\"http://";
  htmlForm.concat( myHostname );
  htmlForm += "/setup/hostname\">";
  htmlForm += "<label for=\"hostname\" > Hostname </label>";
  htmlForm += "<input type=\"text\" name=\"hostname\" maxlength=\"";
  htmlForm.concat( MAX_NAME_LENGTH );
  htmlForm += "\" value=\"";
  htmlForm.concat( myHostname );
  htmlForm += "\"/>";
  htmlForm += "<input type=\"submit\" value=\"Set hostname\" />";
  htmlForm += "</form></div></div>";

  //Device settings - Location 
  htmlForm += "<div class=\"row float-left\">";
  htmlForm += "<div class=\"col-sm-12\"><h2> Enter new descriptive location for device</h2>";
  htmlForm += "<form method=\"POST\" id=\"location\" action=\"http://";
  htmlForm.concat( myHostname );
  htmlForm += "/setup/location\">";
  htmlForm += "<label for=\"location\" >Location description </label>";
  htmlForm += "<input type=\"text\" class=\"form-control\" name=\"location\" maxlength=\"";
  htmlForm += MAX_NAME_LENGTH;
  htmlForm += "\" value=\"";
  htmlForm.concat( Location );
  htmlForm += "\"/>";
  htmlForm += "<input type=\"submit\" value=\"Set location\" />";
  htmlForm += "</form></div></div>";

  return htmlForm;
}

String& setupFormBuilderDriver0Limits( String& htmlForm )
{
  String hostname = WiFi.hostname();
  int i = 0;
/*  
  htmlForm = "<div class=\"row float-left\" id=\"placeholder-flapcount\" >";
  htmlForm += "<div class=\"col-sm-12\" ><h2>Placeholder for flapcount </h2></div>";
  htmlForm += "</div>";
DEBUGSL1( htmlForm .c_str() );
*/

  //Device settings - opening minlimit and maxlimit 
  htmlForm = F("<div class=\"row float-left\" id=\"focuserLimits\">");
  htmlForm += F("<div class=\"col-sm-12\"><h2> Enter new focuser limits for device</h2>");
  htmlForm += F("<form class=\"form-inline\" method=\"POST\" id=\"limits\" action=\"http://");
  htmlForm.concat( myHostname );
  htmlForm += F("/setup/v1/focuser/0/setup/limits\"> ");

  String local = "";
/*
#define HTMLLIMITS  
#ifdef  HTMLLIMITS
  for (i = 0; i< flapCount; i++ )
  {    
    local += "<div class=\"form-group\">";
    //Update the min swing limit ( closed) 
    local += "<label for=\"minLimit";
    local.concat( i );
    local += "\">Switch [";
    local.concat( i );  
    local.concat( "]: Closed limit: &nbsp; </label>");
    local += "<input type=\"number\" size=\"6\" maxlength=\"6\"  name=\"minLimit";
    local.concat( i ); 
    local += "\" min=\"";
    local.concat( rcMinLimit );
    local += "\" max=\"";
    local.concat( rcMaxLimit );
    local += "\" value=\"";
    local.concat( flapMinLimit[i] );
    local += "\">";
    
    //Update the max limit (out)
    local += "<label for=\"maxLimit";
    local.concat( i ); 
    local += "\" >&nbsp; Opening limit: </label>"; 

    local += "<input type=\"number\" size=\"6\" name=\"maxLimit";
    local.concat( i ); 
    local += "\" min=";
    local.concat ( rcMinLimit );
    local += "\" max=\"";
    local.concat( rcMaxLimit );
    local += "\" value=\"";
    local.concat( flapMaxLimit[i] );
    local += "\"><br>";
    local += "</div>"; //form-group
  }
#endif   
  local += F("<input type=\"submit\" class=\"btn btn-default\" value=\"Update limits\" /> ");
  */
  htmlForm += F("</form></div></div>");
  DEBUGSL1( htmlForm );
  
  return htmlForm;
}

/* Configure the Focuser resoltuon settings */
String& setupFormBuilderDriver0Resolution( String& htmlForm )
{
  String hostname = WiFi.hostname();
  int i = 0;
/*  
  htmlForm = "<div class=\"row float-left\" id=\"placeholder-flapcount\" >";
  htmlForm += "<div class=\"col-sm-12\" ><h2>Placeholder for flapcount </h2></div>";
  htmlForm += "</div>";
DEBUGSL1( htmlForm .c_str() );
*/

  //Device settings - Number of flaps to open - up to 16 handled by Servo board. 
  htmlForm = "<div class=\"row float-left\" id=\"FocuserMovement\">";
  htmlForm += "<div class=\"col-sm-12\"><h2> Setup focuser movement sizes</h2>";
  htmlForm += "<form class=\"form-inline\" method=\"POST\" id=\"flapCount\" action=\"http://";
  htmlForm.concat( myHostname );
  htmlForm += "/setup/v1/focuser/0/setup/resolution\">";

  /*
   * 
  htmlForm += "<div class=\"form-group\">";
  htmlForm += "<label for=\"flapCount\" > Number of flaps in focuser: </label>";
  htmlForm += "<input type=\"number\" class=\"form-control\" size=\"6\" name=\"flapCount\" min=\"1\" max=\"";
  htmlForm.concat( MAX_SERVOS );
  htmlForm += "\" value=\"";
  htmlForm.concat( flapCount );
  htmlForm += "\"/>";
  htmlForm += "<input type=\"submit\" class=\"btn btn-default\" value=\"Set flap count\" />";
  */
  htmlForm += "</div></form>";
  htmlForm += "</div></div>";

  DEBUGSL1( htmlForm.c_str() );

  return htmlForm;
}

//Function to setup the distance setting
String& setupFormBuilderDriver0Position( String& htmlForm) 
{
  /*
   *<div class="row float-left" id="positions"> 
      <div class="col-sm-12"> <h2> Enter new flap positions</h2><br>
        <form class="form-inline" method="POST"  action="http://myHostname/setup/v1/focusercalibrator/0/setup/positions" >
            <div class="form-group">
              <label for="position0"> Position[0]: </label>
              <input type="number" class="form-control" name="brightness" min="0" max="180" value="5" size="6">
              <label for="position1"> Position[1]: </label>
              <input type="number" class="form-control" name="brightness" min="0" max="180" value="5" size="6">
              <label for="position2"> Position[2]: </label>
              <input type="number" class="form-control" name="brightness" min="0" max="180" value="5" size="6">
              <label for="position3"> Position[3]: </label>
              <input type="number" class="form-control" name="position" min="0" max="180" value=".." size="6">
              etc
              <input type="submit" class="btn btn-default" value="Set positions" />
            </div>
        </form>
      </div>
    </div>
   */

  htmlForm =  F("<div class=\"row float-left\" id=\"positions\">");
  htmlForm += F("<div class=\"col-sm-12\"><h2>Set focuser position</h2>");
  htmlForm += F("<form class=\"form-inline\" method=\"POST\"  action=\"http://");
  htmlForm.concat( myHostname );
  htmlForm += F("/setup/v1/focuser/0/setup/position\">");

  String local = "";
  int i = 0;

    local = "<div class=\"form-group\">";
    //set the default selected button to be the opposite of current state. 
    if( focuser.getabsMode() == true ) 
    {
      local += "<input type=\"number\" id=\"position\" name=\"focuserposition\" value=\""  ;
      local + position + absPosition;
      local += "\"";
    }
    else 
    {
        local += "<input type=\"number\" id=\"position\" name=\"focuserposition\" value=\""  ;
        local +=  position;
        local += "\"";
    }
    local += "<label for=\"position\"> Focuser position </label><br>";
    local += "</div>"; //form-group

  htmlForm += local;   
  htmlForm += F("<input type=\"submit\" class=\"btn btn-default\" value=\"Set focuser position\" />");
  htmlForm += F("</form></div></div>");
  return htmlForm;
}
 
//Function to query and receive the central temperature reading source used for managing the active temerature compensation . 
String& setupFormBuilderDriver0Temperature( String& htmlForm )
{
  int i; 
  /*
   *<div class="row float-left" id="brightnessfield"> 
      <div class="col-sm-12"> <h2> Enter new brightness</h2><br>
        <form class="form-inline" method="POST"  action="http://myHostname/setup/v1/focusercalibrator/0/setup/brightness" >
            <div class="form-group">
              <label for="brightness"> Brightness: </label>
              <input type="number" class="form-control" name="brightness" min="0" max="16" value="5" >
              <input type="submit" class="btn btn-default" value="Set brightness" />
            </div>
        </form>
      </div>
    </div>
   */

  if ( tempCompEnabled )
  {
    htmlForm =  F("<div class=\"row float-left\" id=\"temperaturefield\" >");
    htmlForm += F("<div class=\"col-sm-12\"><h2> Edit temperature set points</h2>");
    htmlForm += F("<form class=\"form-inline\" method=\"POST\"  action=\"http://");
    htmlForm.concat( myHostname );
    htmlForm += F("/setup/v1/focuser/0/setup/temperaturesource\">");
  
    //Draw a table of position vs temperatures radio boxes for delete/update and edit boxes for focuser position against a listed termperature. 
    //get a list of temperatures and positions. 
    htmlForm += F("<div class=\"form-group\">");
    htmlForm += F("<label for=\"brightness\" > Positions: </label>");
    htmlForm += F("<input type=\"number\" class=\"form-control\" name=\"brightness\" size=\"6\" min=\"");
    htmlForm.concat ( 0 );
    htmlForm += F("\" max=\"");
    htmlForm.concat( 10024 );
    htmlForm += F("\" value=\"");
    htmlForm.concat( String( position ) );
    htmlForm += F("\"/><br> ");
  }

  //Turn on or off the temp compensation. 
  htmlForm += "<input type=\"radio\" id=\"compensationEnabler\" name=\"compensationEnabler\" value=\"";
  htmlForm += ( tempCompEnabled )? "On\">" : "Off\">" ;
  if( tempCompEnabled ) 
    htmlForm += "<label for=\"compensationEnabler\">&nbsp; Turn on Compensation</label><br>";
  else 
    htmlForm += "<label for=\"compensationEnabler\">&nbsp; Turn off Compensation</label><br>";
  htmlForm += "</div>"; //form-group
  
  htmlForm += F("<input type=\"submit\" class=\"btn btn-default\" value=\"Set Calibrator state\" />" );
  htmlForm += F("</form>");
  htmlForm += F("</div></div>");

  return htmlForm;
}

//footer
String& setupFormBuilderFooter( String& htmlForm )
{
  //restart button
  /*
  htmlForm =  F("<div class=\"row float-left\" id=\"restartField\" >");
  htmlForm += F("<div class=\"col-sm-12\"><h2> Restart device</h2>");
  htmlForm += F("<form class=\"form-inline\" method=\"POST\"  action=\"http://");
  htmlForm.concat( myHostname );
  htmlForm += F("/restart\">");
  htmlForm += F("<input type=\"submit\" class=\"btn btn-default\" value=\"Restart device\" />");
  htmlForm += F("</form>");
  htmlForm += F("</div></div>");
  
  //Update button
  htmlForm += F("<div class=\"row float-left\" id=\"updateField\" >");
  htmlForm += F("<div class=\"col-sm-12\"><h2> Update firmware</h2>");
  htmlForm += F("<form class=\"form-inline\" method=\"GET\"  action=\"http://");
  htmlForm.concat( myHostname );
  htmlForm += F("/update\">");
  htmlForm += F("<input type=\"submit\" class=\"btn btn-default\" value=\"Update firmware\" />");
  htmlForm += F("</form>");
  htmlForm += F("</div></div>");

  //Close page
  htmlForm += F("<div class=\"row float-left\" id=\"positions\">");
  htmlForm += F("<div class=\"col-sm-12\"><br> </div></div>");
  htmlForm += F("</div></body></html>");
  */
  return htmlForm;
}
#endif
