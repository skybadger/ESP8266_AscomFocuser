#ifndef _ESP8266_FOCUSER_HANDLERS_H_
#define _ESP8266_FOCUSER_HANDLERS_H_

//ASCOM Filterwheel REST API specific functions

#include "Focuser_eeprom.h"
#include <Wire.h>
#include "AlpacaErrorConsts.h"
#include "ASCOMAPIFocuser_rest.h"

/*Methods - as per the FocuserRestApi.h
void handlerAbsoluteGet(void);
void handlerIsMovingGet(void);
void handlerMaxIncrementGet(void);
void handlerMaxStepGet(void);
void handlerPositionGet(void);

void handleAbsolute( void );
void handleIsMoving( void );
void handleMaxIncrement( void);
void handleMaxStep( void );
void handlePosition (void );
void handleStepSize (void );
void handleTempCompensation( void );
void handleTempCompAvailable(void);
void handleTemperature(void);

//Others
void handleRootReset(void);
void handlerNotFound(void);
*/

//Function to chop up the uri into '/'delimited fields to analyse for wild cards and paths
bool getUriField( char* inString, int searchIndex, String& outRef )
{
  char *p = inString;
  char *str;
  char delims1[] = {"//"};
  char delims2[] = {"/:"};
  int chunkCtr = 0;
  bool  status = false;    
  int localIndex = 0;
  
  localIndex = String( inString ).indexOf( delims1 );
  if( localIndex >= 0 )
  {
    while ((str = strtok_r(p, delims2, &p)) != NULL) // delimiter is the semicolon
    {
       if ( chunkCtr == searchIndex && !status )
       {
          outRef = String( str );
          status = true;
       }
       chunkCtr++;
    }
  }
  else 
    status = false;
  
  return status;
}

//GET ​/focuser​/{device_number}​/absolute
void handlerAbsoluteGet(void)
{
    String message;
    uint32_t clientID= -1;
    uint32_t transID = -1;
    String argToSearchFor[] = { "clientID", "ClientTransactionID", "absolute" };
    
    if( hasArgIC( argToSearchFor[0], server, false ) )
      clientID = server.arg(argToSearchFor[0]).toInt();
    
    if ( hasArgIC( argToSearchFor[1], server, false) )
      transID = server.arg(argToSearchFor[1]).toInt();

    debugD( "Getting ( client %d trans %d ) Absolute mode  resulted in value: %d", clientID, transID, absMode );
    DynamicJsonBuffer jsonBuffer(256);
    JsonObject& root = jsonBuffer.createObject();
    jsonResponseBuilder( root, clientID, transID, ++serverTransID, "AbsoluteGet", Success , "" );    
    root["Value"] = absMode;
    
    root.printTo(message);
    server.send(200, F("application/json"), message);
    return ;
}

//GET ​/focuser​/{device_number}​/IsMoving
//Return the limit of the brightness setting available
void handlerFocuserIsMovingGet(void)
{
    String message;
    uint32_t clientID= -1;
    uint32_t transID = -1;
    String argToSearchFor[] = { "clientID", "ClientTransactionID", };
    
    if( hasArgIC( argToSearchFor[0], server, false ) )
      clientID = server.arg(argToSearchFor[0]).toInt();
    
    if ( hasArgIC( argToSearchFor[1], server, false) )
      transID = server.arg(argToSearchFor[1]).toInt();

    debugD( "Getting ( client %d trans %d ) calibrator max brightness resulted in value: %d", clientID, transID, isMoving );
    DynamicJsonBuffer jsonBuffer(256);
    JsonObject& root = jsonBuffer.createObject();
    jsonResponseBuilder( root, clientID, transID, ++serverTransID, "IsMovingGet", Success , "" );    
    root["Value"] = isMoving;       
    root.printTo(message);
    server.send(200, F("application/json"), message);
    return ;
}

//Get the focuser max increment setting
void handlerFocuserModeGet(void)
{
    String message;
    int returnCode = 200;

    uint32_t clientID= -1;
    uint32_t transID = -1;
    String argToSearchFor[] = { "clientID", "ClientTransactionID", };
    DynamicJsonBuffer jsonBuffer(256);
    
    if( hasArgIC( argToSearchFor[0], server, false ) )
      clientID = server.arg(argToSearchFor[0]).toInt();
    
    if ( hasArgIC( argToSearchFor[1], server, false) )
      transID = server.arg(argToSearchFor[1]).toInt();

    debugD( "Getting (client %d trans %d) focuser mode resulted in value: %d", clientID, transID, absMode  );
    JsonObject& root = jsonBuffer.createObject();
    jsonResponseBuilder( root, clientID, transID, ++serverTransID, F("MaxSteIncrementGet"), Success , "" );    
 
    root["Value"] = (int) maxStepIncrement;

    root.printTo(message);
    server.send(returnCode, F("application/json"), message);
    return;
}


//Get the focuser max increment setting
void handlerFocuserMaxIncrementGet(void)
{
    String message;
    int returnCode = 200;

    uint32_t clientID= -1;
    uint32_t transID = -1;
    String argToSearchFor[] = { "clientID", "ClientTransactionID", };
    DynamicJsonBuffer jsonBuffer(256);
    
    if( hasArgIC( argToSearchFor[0], server, false ) )
      clientID = server.arg(argToSearchFor[0]).toInt();
    
    if ( hasArgIC( argToSearchFor[1], server, false) )
      transID = server.arg(argToSearchFor[1]).toInt();

    debugD( "Getting (client %d trans %d) focuser state resulted in value: %d", clientID, transID, maxStepIncrement  );
    JsonObject& root = jsonBuffer.createObject();
    jsonResponseBuilder( root, clientID, transID, ++serverTransID, F("MaxSteIncrementGet"), Success , "" );    
 
    root["Value"] = (int) maxStepIncrement;

    root.printTo(message);
    server.send(returnCode, F("application/json"), message);
    return;
}

//Get the focuser max step setting
void handlerFocuserMaxStepGet(void)
{
    String message;
    int returnCode = 200;

    uint32_t clientID= -1;
    uint32_t transID = -1;
    String argToSearchFor[] = { "clientID", "ClientTransactionID", };
    DynamicJsonBuffer jsonBuffer(256);
    
    if( hasArgIC( argToSearchFor[0], server, false ) )
      clientID = server.arg(argToSearchFor[0]).toInt();
    
    if ( hasArgIC( argToSearchFor[1], server, false) )
      transID = server.arg(argToSearchFor[1]).toInt();

    debugD( "Getting (client %d trans %d) focuser state resulted in value: %d", clientID, transID, maxStep  );
    JsonObject& root = jsonBuffer.createObject();
    jsonResponseBuilder( root, clientID, transID, ++serverTransID, F("MaxStepGet"), Success , "" );    
 
    root["Value"] = (int) maxStep;

    root.printTo(message);
    server.send(returnCode, F("application/json"), message);
    return;
}


//Gets the fpcuser position - TODO need to add consideration for abs vs rel. 
void handlerFocuserPositionGet(void)
{
    String message;
    int returnCode = 200;

    DynamicJsonBuffer jsonBuffer(256);
    uint32_t clientID= -1;
    uint32_t transID = -1;
    String argToSearchFor[] = { F("clientID"), F("ClientTransactionID"), };
    
    if( hasArgIC( argToSearchFor[0], server, false ) )
      clientID = server.arg(argToSearchFor[0]).toInt();
    
    if ( hasArgIC( argToSearchFor[1], server, false) )
      transID = server.arg(argToSearchFor[1]).toInt();
   
    debugD( "Getting (client %d trans %d) position result: %d, ", clientID, transID, position  );
    
    JsonObject& root = jsonBuffer.createObject();
    jsonResponseBuilder( root, clientID, transID, ++serverTransID, F("PositionGet"), Success , "" );    
 
    root["Value"] = (int) position;
    root.printTo(message);
    server.send( returnCode = 200, F("application/json"), message);
    return;
}


//GET
//Get the focuser stepSize - needs to return in microns - so need the steps/rev, microstepping and some physical sizes per rev to work this out. 
void handlerFocuserStepSizeGet(void)
{
    String message;
    String errMsg;
    int errCode = Success;
    int returnCode = 200;

    uint32_t clientID= -1;
    uint32_t transID = -1;
    String argToSearchFor[] = { "ClientID", "ClientTransactionID", };
    
    if( hasArgIC( argToSearchFor[0], server, false ) )
      clientID = server.arg(argToSearchFor[0]).toInt();
      
    if ( hasArgIC( argToSearchFor[1], server, false) )
      transID = server.arg(argToSearchFor[1]).toInt();
     
    DynamicJsonBuffer jsonBuffer(256);
    JsonObject& root = jsonBuffer.createObject();
    jsonResponseBuilder( root, clientID, transID, ++serverTransID, F("handlerStepSizeGet"), errCode, errMsg );    

    //Set targetfocuserState to desired state. 
    if ( connected != clientID && connected != NOT_CONNECTED )
    {
      errCode = notConnected;  
      errMsg = "This is not the connected client";         
    }
    else if ( stepSize == 0.0F )
  	{
        errCode = valueNotSet;  
        errMsg = "The stepSize property is not yet configured";         			
  	}
  	else
    {
	    root["Value"]  = (double) stepSize;
      errCode = Success;
      errMsg = "";   
    }

    debugD( "Getting ( client %d trans %d ) focuser stepsize resulted in value: %f, error: '%s'", clientID, transID, stepSize, errMsg.c_str()  );
    root.printTo(message);

    server.send(returnCode = 200, F("application/json"), message);
    return;
}

void handlerFocuserTempCompModeGet( void )
{
    String message, errMsg;
    int returnCode = 200, errCode = 0;

    uint32_t clientID= -1;
    uint32_t transID = -1;
    String argToSearchFor[] = { "clientID", "ClientTransactionID", };
    DynamicJsonBuffer jsonBuffer(256);
    
    if( hasArgIC( argToSearchFor[0], server, false ) )
      clientID = server.arg(argToSearchFor[0]).toInt();
    
    if ( hasArgIC( argToSearchFor[1], server, false) )
      transID = server.arg(argToSearchFor[1]).toInt();

    debugD( "Getting (client %d trans %d) focuser temperature compensation mode resulted in value: %d", clientID, transID, tempCompEnabled );
    JsonObject& root = jsonBuffer.createObject();
 
    if ( connected != clientID && connected != NOT_CONNECTED )
    {
      errCode = notConnected;  
      errMsg = "This is not the connected client";         
      jsonResponseBuilder( root, clientID, transID, ++serverTransID, F("handleTempCompensation"), errCode , errMsg );          
    }
    else
	  {
      errCode = Success;
      errMsg = "";
      jsonResponseBuilder( root, clientID, transID, ++serverTransID, F("handleTempCompensation"), errCode , errMsg );    
	    root["Value"] = (bool) tempCompEnabled;
    }
    root.printTo(message);
    server.send(returnCode, F("application/json"), message);
    return;
}

//Get the focuser max step setting
void handlerFocuserTempCompAvailableGet(void)
{
    String errMsg, message;
    int returnCode = 200, errCode = 0;

    uint32_t clientID= -1;
    uint32_t transID = -1;
    String argToSearchFor[] = { "clientID", "ClientTransactionID", };
    DynamicJsonBuffer jsonBuffer(256);
    
    if( hasArgIC( argToSearchFor[0], server, false ) )
      clientID = server.arg(argToSearchFor[0]).toInt();
    
    if ( hasArgIC( argToSearchFor[1], server, false) )
      transID = server.arg(argToSearchFor[1]).toInt();

    debugD( "Getting (client %d trans %d) focuser Temp Comp Available resulted in value: %d", clientID, transID, tempCompAvailable  );
    JsonObject& root = jsonBuffer.createObject();

    if ( connected != clientID && connected != NOT_CONNECTED )
    {
      errCode = notConnected;  
      errMsg = "This is not the connected client";         
      jsonResponseBuilder( root, clientID, transID, ++serverTransID, F("handleTempCompAvailableGet"), errCode , errMsg );    
    }
    else
	  {
      errCode = Success;
      errMsg = "";
      jsonResponseBuilder( root, clientID, transID, ++serverTransID, F("handleTempCompAvailableGet"), errCode , errMsg );    
	    root["Value"] = (bool) tempCompAvailable;     
    }

    root.printTo(message);
    server.send(returnCode, F("application/json"), message);
    return;
}

void handlerFocuserTempGet(void)
{
	  String errMsg, message;
    int errCode = Success, returnCode = 200;

    uint32_t clientID= -1;
    uint32_t transID = -1;
    String argToSearchFor[] = { "clientID", "ClientTransactionID", };
    DynamicJsonBuffer jsonBuffer(256);
    
    if( hasArgIC( argToSearchFor[0], server, false ) )
      clientID = server.arg(argToSearchFor[0]).toInt();
    
    if ( hasArgIC( argToSearchFor[1], server, false) )
      transID = server.arg(argToSearchFor[1]).toInt();

    debugD( "Getting (client %d trans %d) focuser temperature resulted in value: %f", clientID, transID, temperature );
    JsonObject& root = jsonBuffer.createObject();
 
    if ( connected != clientID && connected != NOT_CONNECTED)
    {
      errCode = notConnected;  
      errMsg = "This is not the connected client";         
      jsonResponseBuilder( root, clientID, transID, ++serverTransID, F("handleTemperatureGet"), Success , "" );    

    }
    else
	  {
      jsonResponseBuilder( root, clientID, transID, ++serverTransID, F("handleTemperatureGet"), Success , "" );       
      root["Value"] = (double) temperature;      
    }

    root.printTo(message);
    server.send( returnCode, F("application/json"), message);
    return;
}

//Methods
//PUT
//Set the focuser to halt
void handlerFocuserHaltPut(void)
{
    String message;
    String errMsg;
    int errCode = Success;
    int returnCode = 200;

    uint32_t clientID= -1;
    uint32_t transID = -1;
    String argToSearchFor[] = { "ClientID", "ClientTransactionID", };
    
    if( hasArgIC( argToSearchFor[0], server, false ) )
      clientID = server.arg(argToSearchFor[0]).toInt();
      
    if ( hasArgIC( argToSearchFor[1], server, false) )
      transID = server.arg(argToSearchFor[1]).toInt();
 
    DynamicJsonBuffer jsonBuffer(256);
    JsonObject& root = jsonBuffer.createObject();

    //Set targetfocuserState to desired state. 
    if ( connected != clientID && connected != NOT_CONNECTED )
    {
      errCode = notConnected;  
      errMsg = "Not the connected client";         
      jsonResponseBuilder( root, clientID, transID, ++serverTransID, F("handlerHaltfocuserPut"), errCode, errMsg );         
    }
    else
    {
      //We use HALTED to flag the requested change of state clearly and end up with UNKNOWN once handled. 
      focuserCmd = FOCUSER_HALT;
	    targetFocuserState = FocuserStates::FOCUSER_HALTED;
      errCode = Success;
      errMsg = "";   
      jsonResponseBuilder( root, clientID, transID, ++serverTransID, F("handlerHaltfocuserPut"), errCode, errMsg );         
    }

    debugD( "Setting ( client %d trans %d ) focuser state to %s from %s resulted in code: %d, error: '%s'", clientID, transID, focuserStateCh[targetFocuserState], focuserStateCh[focuserState], errCode, errMsg.c_str() );
    root.printTo(message);
    server.send(returnCode = 200, F("application/json"), message);
    return;
}

//PUT
//Set the focuser to Move
void handlerFocuserMovePut(void)
{
    String message;
    String errMsg;
    int localPosition;
	  int errCode = Success;
    int returnCode = 200;

    uint32_t clientID= -1;
    uint32_t transID = -1;
    String argToSearchFor[] = { "clientID", "ClientTransactionID", "position" };
    
    if( hasArgIC( argToSearchFor[0], server, false ) )
      clientID = server.arg(argToSearchFor[0]).toInt();
      
    if ( hasArgIC( argToSearchFor[1], server, false) )
      transID = server.arg(argToSearchFor[1]).toInt();

    if ( hasArgIC( argToSearchFor[2], server, false) )
      localPosition = server.arg(argToSearchFor[2]).toDouble();

    DynamicJsonBuffer jsonBuffer(256);
    JsonObject& root = jsonBuffer.createObject();

    //Set targetfocuserState to desired state. 
    if ( connected != clientID && connected != NOT_CONNECTED )
    {
      errCode = notConnected;  
      errMsg = F("This is not the connected client");         
    }
    else if ( focuserCmd != FOCUSER_RDY ) 
    {
       errCode = invalidOperation; //handle not ready error
       errMsg = "Focuser not ready - try again";
     }
    else   
    {
      if ( absMode ) //absolute mode 
  	  {
    		//check we are aiming to be in a valid range
    		if ( localPosition <= maxStep && localPosition >= minStep )
        {
           targetPosition = localPosition; 
           focuserCmd = FocuserCmds::FOCUSER_MOVE;
           errCode = Success;
           errMsg = "";           
        }
  		  else
  		  {
          errCode = invalidValue;
          errMsg = "Focuser Move Cmd out of available range - try again";
  		  }
   		}
      else //relative mode
      {
        //check we are aiming for a valid absolute range 
        int localAbsPosition = localPosition + position + absPosition; 
        if ( localAbsPosition <= maxStep && localAbsPosition >= minStep )
        {
           targetPosition = localPosition; 
           focuserCmd = FocuserCmds::FOCUSER_MOVE;
           errCode = Success;
           errMsg = "";
        }
        else
        {
          errCode = invalidValue;
          errMsg = "Relative Focuser Move Cmd would move focuser out of available absolute range - try again";
        }
      }
	  }
    jsonResponseBuilder( root, clientID, transID, ++serverTransID, F("handlerFocuserMovePut"), errCode, errMsg );    
	   
    debugD( "Setting ( client %d trans %d ) focuser position to %d from %d resulted in code: %d, error: '%s'", clientID, transID, targetPosition, position, errCode, errMsg.c_str() );

    root.printTo(message);
    server.send( returnCode = 200, F("application/json"), message);
    return;
}

////////////////////////////////////////////////////////////////////////////////////
//Additional non-ASCOM custom setup calls

void handlerNotFound()
{
  String message;
  int responseCode = 400;
  uint32_t clientID= -1;
  uint32_t transID = -1;
  String argToSearchFor[] = { "clientID", "ClientTransactionID", };
  
  if( hasArgIC( argToSearchFor[0], server, false ) )
    clientID = server.arg(argToSearchFor[0]).toInt();
  
  if ( hasArgIC( argToSearchFor[1], server, false) )
    transID = server.arg(argToSearchFor[1]).toInt();

  DynamicJsonBuffer jsonBuffer(250);
  JsonObject& root = jsonBuffer.createObject();
  String missingURL = F("HandlerNotFound");
  missingURL.concat(":");
  missingURL.concat( server.uri() );
  
  jsonResponseBuilder( root, clientID, transID, ++serverTransID, missingURL, invalidOperation , F("No REST handler found for argument - check ASCOM Switch v2 specification") );    
  root["Value"] = 0;
  root.printTo(message);
  server.send( responseCode, "application/json", message);
}

void handlerRestart( void)
{
  //Trying to do a redirect to the rebooted host so we pick up from where we left off. 
  server.sendHeader( WiFi.hostname().c_str(), String("/status"), true);
  server.send ( 302, F("text/html"), F("<!Doctype html><html>Redirecting for restart</html>"));
  debugI("Reboot requested");
  //Set any device specific hardware  controls here
  
  device.restart();
}

void handlerNotImplemented()
{
  String message;
  int responseCode = 400;
  uint32_t clientID= -1;
  uint32_t transID = -1;
  String argToSearchFor[] = { "clientID", "ClientTransactionID", };
  
  if( hasArgIC( argToSearchFor[0], server, false ) )
    clientID = server.arg(argToSearchFor[0]).toInt();
  
  if ( hasArgIC( argToSearchFor[1], server, false) )
    transID = server.arg(argToSearchFor[1]).toInt();

  DynamicJsonBuffer jsonBuffer(250);
  JsonObject& root = jsonBuffer.createObject();
  jsonResponseBuilder( root, clientID, transID, ++serverTransID, F("HandlerNotFound"), notImplemented  , F("No REST handler implemented for argument - check ASCOM Dome v2 specification") );    
  root["Value"] = 0;
  root.printTo(message);
  
  server.send(responseCode, F("application/json"), message);
}

//Get a descriptor of all the components managed by this driver for focusery purposes
void handlerStatus(void)
{
    String message, timeString;
    int returnCode = 200;
    int i;
   
    DynamicJsonBuffer jsonBuffer(250);
    JsonObject& root = jsonBuffer.createObject();
   
    root[F("time")] = getTimeAsString( timeString );
    root[F("host")] = myHostname;
    root[F("connected")] = (connected == NOT_CONNECTED )?"false":"true";
    root[F("clientID")] = (int) connected;
    root[F("focuserState")] = focuserStateCh[focuserState];   
    root[F("position")]  = position;
    
    //Limits
    root[F("maxStep")] = maxStepLimit;
    root[F("minStep")] = minStepLimit;
    root[F("maxStepIncrement")] = maxStepIncrement;
    root[F("minStepIncrement")] = minStepIncrement;
    root[F("absMode")] = absMode; 

    //root[""] = ;
       
    //Calibration settings
    root[F("microsteps per step")]  = microstepsPerStep;
    root[F("stepsPerRev")] = stepsPerRev;
    root[F("distancePerRev")] = distancePerRev;

    //Temperature compensation
    root[F("TempComp source")] = mqttTemperatureSource;
    root[F("tempCompAvailable")] = tempCompAvailable;
    root[F("tempCompEnabled")] = tempCompEnabled;
    if( tempCompAvailable )
    {
      JsonArray& tempEntries = root.createNestedArray( F("TempOffsets") );
      JsonObject& entry = jsonBuffer.createObject();      
      for( i=0; i< tempOffsetsLength; i++ )
      {
        entry[F("temp")]   = (int) tempCompTuples[i].temp;
        entry[F("offset")] = (int) tempCompTuples[i].offset;
        tempEntries.add( entry ); 
      }
    } 
           
    Serial.println( message);
    root.printTo(message);
    
    server.send(returnCode, F("application/json"), message);
    return;
}


#endif
