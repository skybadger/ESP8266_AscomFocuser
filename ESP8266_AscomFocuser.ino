/*
    This program uses an esp8266 board to host a web server providing a rest API to control a Focuser-motor 
   driven telescope focuser motor as used on a telescope. 
   The Focuser motor controller is controlled direclty through the three ios avilable on the chip.
   This implements the ASCOM REST api for a focuser natively and adds a 
   user setup html page which ASCOM doesn't provide unless you use a native ASCOM driver as initial setup. 
   Specified focuser default ID as zero since we will only ever handle the one filterwheel for each hostname/address. 
   For testing, bear in mind that while browsers can't provide a manual put action, neither can curl 
   run from the windows command-line without wrapping the arguments in quotes due to windows interpreting the "&" character as an additional command. 
   Enable the debug http core setting in arduino instead to get access to all the inline request args as debug output on the serial port.
 Supports web interface on port 80 returning json string
 
Notes: 
 
 To do:
 Implement handler functions
 Test handler functions
 Add encoder/DC brushed motor support 
 Add focuser state monitor
 Add support for MQTT callback for local temperature sensor listening. 
 Add support for local hardware temperature sensor - probably won't do since can do it over MQTT. 
   
 Done: 
 
 
 Pin Layout:
 ESP8266_01e
 GPIO 3 (Rx) Used to control power to servo using PNP poweer mosfet.
 GPIO 2 (SDA) to PWM RC signal 
 GPIO 0 (SCL) Used to control illuminator using PWM via PNP power mosfet. 
 GPIO 1 used as Serial Tx. 
 
 ESP8266_12
 GPIO 4,2 to SDA
 GPIO 5,0 to SCL 
 All 3.3v logic. 
 
To test:
 curl -X PUT -d "name=myFocuserHostname" http://EspFoc01/api/v1/focuser/0/hostname
 curl -X get http://EspFoc01/api/v1/focuser/0/position

telnet espFoc01 32272 (UDP)

Dependencies
JSON parsing for arduino : https://arduinojson.org/v5/api/
Remote debug over telnet : https://github.com/JoaoLopesF/RemoteDebug
MQTT pubsub library : https://pubsubclient.knolleary.net/api.html

Change Log
 */

#include "Focuser_common.h"
#include "AlpacaErrorConsts.h"
#include <esp8266_peri.h> //register map and access
#include <ESP8266WiFi.h>
#include <PubSubClient.h> //https://pubsubclient.knolleary.net/api.html
#include <EEPROM.h>
#include "EEPROMAnything.h"
#include <Wire.h>         //https://playground.arduino.cc/Main/WireLibraryDetailedReference
#include <Time.h>         //Look at https://github.com/PaulStoffregen/Time for a more useful internal timebase library
#include <WiFiUdp.h>      //WiFi UDP discovery responder
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ArduinoJson.h>  //https://arduinojson.org/v5/api/

//Create a remote debug object
#if !defined DEBUG_DISABLED
RemoteDebug Debug;
#endif 
 
extern "C" { 
//Ntp dependencies - available from v2.4
#include <time.h>
#include <sys/time.h>
//#include <coredecls.h>
 
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <String.h> 
#include <user_interface.h>
            }
time_t now; //use as 'gmtime(&now);'

WiFiClient espClient;
PubSubClient client(espClient);
volatile bool callbackFlag = false;

// Create an instance of the server
// specify the port to listen on as an argument
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer updater;
//Setup the management discovery listener
WiFiUDP Udp;

//Hardware device system functions - reset/restart etc
EspClass device;
long int nowTime, startTime, indexTime;
ETSTimer timer, timeoutTimer;
volatile int timeoutFlag = 0;
volatile int newDataFlag = 0;
int focuserStateFlag = 0; //Use this for the Focuser to flag its state. 1 is active

const int EEPROMSIZE = 256;
int focuserPresent = false;

//Declarations - web handlers
//Order sensitive
#include "Skybadger_common_funcs.h"
#include "JSONHelperFunctions.h"
#include "Focuser_eeprom.h"
#include "ASCOMAPICommon_rest.h" //ASCOM common driver web handlers. 
#include "ESP8266_FocuserHandlers.h"
#include "FocuserSetupHandlers.h"
#include <AlpacaManagement.h>
#include "ESP8266_AscomFocuser_Focuser.h"
Focuser focuser( 2, 3, 4, 2000);

//local functions
void onTimer();
void onTimeoutTimer();
void setup(void);

//Timer handler for 'soft' interrupt handler
void onTimer( void* pArg )
{
  newDataFlag++;
}

//Used to complete MQTT timeout actions. 
volatile bool timerSet = false;
void onTimeoutTimer( void* pArg )
{
  //Read command list and apply. 
  timeoutFlag = true;
}

// put your setup code here, to run once:
void setup()
{
  String i2cMsg = "";

  Serial.begin( 115200, SERIAL_8N1, SERIAL_TX_ONLY);
  Serial.println();
  DEBUG_ESP_MH("ESP starting.");
 
  //Setup default data structures
  DEBUGSL1("Setup EEprom variables"); 
  EEPROM.begin( EEPROMSIZE );
  setupFromEeprom();
  DEBUGSL1("Setup eeprom variables complete."); 

  //Setup Wifi
  scanNet();
  setup_wifi();   
  
  delay(2000);
  
  //Start NTP client
  configTime(TZ_SEC, DST_SEC, timeServer1, timeServer2, timeServer3 );
  delay( 2000);
    
#if !defined DEBUG_DISABLED
  //Debugging over telnet setup
  // Initialize the server (telnet or web socket) of RemoteDebug
  Debug.begin( WiFi.hostname().c_str(), Debug.VERBOSE ); 
  Debug.setSerialEnabled(true);//until set false 
  // Options
  // Debug.setResetCmdEnabled(true); // Enable the reset command
  // Debug.showProfiler(true); // To show profiler - time between messages of Debug
  //In practice still need to use serial commands until debugger is up and running.. 
  debugE("Remote debugger enabled and operating");
#endif //remote debug

  //for use in debugging reset - may need to move 
  DEBUG_ESP_MH( "Device reset reason: %s\n", device.getResetReason().c_str() );
  DEBUG_ESP_MH( "device reset info: %s\n",   device.getResetInfo().c_str() );
    
  //Open a connection to MQTT
  DEBUG_ESP_MH( ("Setting up MQTT.")); 
  client.setServer( mqtt_server, 1883 );
  client.connect( thisID, pubsubUserID, pubsubUserPwd ); 
  String lastWillTopic = outHealthTopic; 
  lastWillTopic.concat( myHostname );
  client.connect( thisID, pubsubUserID, pubsubUserPwd, lastWillTopic.c_str(), 1, true, "Disconnected", false ); 
  //Create a heartbeat-based callback that causes this device to read the local i2C bus devices for data to publish.
  //TODO Update callback to replace with another that listens for the temperature data required for temp compensation  - set compEn false if not found. 
  client.setCallback( callback ); 
  client.subscribe( inTopic );
  client.subscribe(mqttTemperatureSource);
  publishHealth();
  DEBUG_ESP_MH( ("MQTT Setup complete."));   

//Focuser hardware setup
  debugI("Setting up Focuser controls\n");  
  focuserPresent = true; 
  focuser = new Focuser();
  focuser.begin();
  focuser.Enable( &focuserStateFlag );
  DEBUG_ESP_MH("Setting up Focuser  controls complete\n");
    
  //Setup webserver handler functions 
  //Common ASCOM handlers
  server.on(F("/api/v1/focuser/0/action"),              HTTP_PUT, handleAction );
  server.on(F("/api/v1/focuser/0/commandblind"),        HTTP_PUT, handleCommandBlind );
  server.on(F("/api/v1/focuser/0/commandbool"),         HTTP_PUT, handleCommandBool );
  server.on(F("/api/v1/focuser/0/commandstring"),       HTTP_PUT, handleCommandString );
  server.on(F("/api/v1/focuser/0/connected"),           handleConnected );
  server.on(F("/api/v1/focuser/0/description"),         HTTP_GET, handleDescriptionGet );
  server.on(F("/api/v1/focuser/0/driverinfo"),          HTTP_GET, handleDriverInfoGet );
  server.on(F("/api/v1/focuser/0/driverversion"),       HTTP_GET, handleDriverVersionGet );
  server.on(F("/api/v1/focuser/0/interfaceversion"),    HTTP_GET, handleInterfaceVersionGet );
  server.on(F("/api/v1/focuser/0/name"),                HTTP_GET, handleNameGet );
  server.on(F("/api/v1/focuser/0/supportedactions"),    HTTP_GET, handleSupportedActionsGet );

//Focuser-specific functions
//Properties
//GET
  server.on(F("/api/v1/focuser/0/absolute"),          	HTTP_GET, handlerFocuserModeGet );  
  server.on(F("/api/v1/focuser/0/ismoving"),            HTTP_GET, handlerFocuserIsMovingGet );  
  server.on(F("/api/v1/focuser/0/maxincrement"),        HTTP_GET, handlerFocuserMaxIncrementGet );  
  server.on(F("/api/v1/focuser/0/maxstep"),		          HTTP_GET, handlerFocuserMaxStepGet );
  server.on(F("/api/v1/focuser/0/position"),          	HTTP_GET, handlerFocuserPositionGet );
  server.on(F("/api/v1/focuser/0/handleStepSize"),      HTTP_GET, handlerFocuserStepSizeGet );
  server.on(F("/api/v1/focuser/0/tempcompavailable"),	  HTTP_GET, handlerFocuserTempCompAvailableGet );
  server.on(F("/api/v1/focuser/0/tempcomp"),          	HTTP_GET, handlerFocuserTempCompModeGet );
  server.on(F("/api/v1/focuser/0/temperature"),     	  HTTP_GET, handlerFocuserTempGet );

//PUT
  server.on(F("/api/v1/focuser/0/tempcomp"),          	HTTP_PUT, handleFocuserTempCompModePut );

//Methods
  server.on(F("/api/v1/focuser/0/halt"),        		    HTTP_PUT, handlerFocuserHaltPut );
  server.on(F("/api/v1/focuser/0/move"),                HTTP_PUT, handlerFocuserMovePut );

//Additional ASCOM ALPACA Management setup calls
  //Per device
  //TODO - split whole device setup from per-instance driver setup e.g. hostname, alpaca port to device compared to ccal setup on the driver, 
  server.on(F("/setup"),                               HTTP_GET, handlerDeviceSetup );
  server.on(F("/setup/hostname") ,                     HTTP_ANY, handlerDeviceHostname );
  server.on(F("/setup/udpport"),                       HTTP_ANY, handlerDeviceUdpPort );
  server.on(F("/setup/location"),                      HTTP_ANY, handlerDeviceLocation );
  
  //Per driver - there may be several on this device. 
  server.on(F("/setup/v1/focuser/0/setup/"),  		     HTTP_ANY, handlerDriver0Setup );
  server.on(F("/setup/v1/focuser/0/setup/motor"),      HTTP_ANY, handlerDriver0Motor );
  server.on(F("/setup/v1/focuser/0/setup/rotation"),   HTTP_ANY, handlerDriver0Rotation );
  server.on(F("/setup/v1/focuser/0/setup/position"),   HTTP_ANY, handlerDriver0Position );
  server.on(F("/setup/v1/focuser/0/setup/backlash"),   HTTP_ANY, handlerDriver0Backlash );
  server.on(F("/setup/v1/focuser/0/setup/limits"),     HTTP_ANY, handlerDriver0Limits );
  
//Custom
  server.on(F("/status"),                              HTTP_GET, handlerStatus);
  server.on(F("/restart"),                             HTTP_ANY, handlerRestart); 

//Management interface calls.
/* ALPACA Management and setup interfaces
 * The main browser setup URL would be http://<hostname>:<port>/api/v1/setup  
 * The JSON list of supported interface versions would be available through a GET to http://<hostname>:<port>/management/apiversions
 * The JSON list of configured ASCOM devices would be available through a GET to http://<hostname>:<port>/management/v1/configureddevices
 * Where <hostname> is whatever you set it to be at wifi startup time via myHostname 
 * and the port is set in the Web server settings - typically 80.
 */
  //Management API - https://www.ascom-standards.org/api/?urls.primaryName=ASCOM%20Alpaca%20Management%20API#/Management%20Interface%20(JSON)
  server.on(F("/management/apiversions"),             HTTP_GET, handleMgmtVersions );
  server.on(F("/management/v1/description"),          HTTP_GET, handleMgmtDescription );
  server.on(F("/management/v1/configureddevices"),     HTTP_GET, handleMgmtConfiguredDevices );
  
  //Basic settings
  server.on(F("/"), handlerStatus );
  server.onNotFound(handlerNotFound); 

  updater.setup( &server );
  server.begin();
  debugI("Web server handlers setup & started\n");
  
  //Starts the ALPACA UDP discovery responder server
  Udp.begin( udpPort );
  
  //Setup timers
  //setup interrupt-based 'soft' alarm handler for periodic update of handler loop
  ets_timer_setfn( &timer, onTimer, NULL ); 
  
  //Timer for MQTT reconnect handler
  ets_timer_setfn( &timeoutTimer, onTimeoutTimer, NULL ); 
  
  //fire loop function handler timer every xx msec
  ets_timer_arm_new( &timer, 250, 1 /*repeat*/, 1);

  //Supports the MQTT async reconnect timer. Armed elsewhere on demand by MQTT reconnect
  //ets_timer_arm_new( &timeoutTimer, 2500, 0/*one-shot*/, 1);
  
  //baseline driver variables
 
  debugI( "Setup complete\n" );

#if !defined DEBUG_ESP_MH && !defined DEBUG_DISABLE
//turn off serial debug if we are not actively debugging.
//use telnet access for remote debugging
   Debug.setSerialEnabled(false); 
#endif

}

/*
  Interrupt handler function captures button press changes pulses from user buttons and flags to main loop to process. 
  Do this by capturing time of interrupt, waiting until after timeout msecs and checking again.
  https://github.com/esp8266/esp8266-wiki/wiki/gpio-registers
*/
void pulseCounter(void)
{
 //if edge rising, capture counter
 byte signalDirection = GPIP(3); //READ_PERI_REG( 0x60000318, PulseCounterPin );
 if ( signalDirection == 0 ) 
 {
    startTime =  ESP.getCycleCount();
    //set flag.
    //newButtonFlag = 1;
 }
}

void loop()
{
  String outbuf;
  long int nowTime = system_get_time();
  long int indexTime = startTime - nowTime;
  
  String timestamp;
  String output;
  
  // Main code here, to run repeatedly:
  if( newDataFlag == true ) 
  {
    if( focuser.focuserState == FocuserStates::FOCUSER_MOVING )
      debugV( "focuser position : %d", position ); 
    
    if ( focuserPresent ) 
      manageFocuserState( targetFocuserState );
    
    newDataFlag = false;
  }
    
  if( client.connected() )
  {
    if (callbackFlag == true )
    {
      //publish results
      publishHealth();
      publishFunction();
      callbackFlag = false;
    }
  }
  else
  {
    reconnectNB();
    client.subscribe (inTopic);
  }
  client.loop(); 

  //Handle Alpaca requests
  handleManagement();

  //Handle web requests
  server.handleClient();
  
#if !defined DEBUG_DISABLED
  // Remote debug over WiFi
  Debug.handle();
  // Or
  //debugHandle(); // Equal to SerialDebug  
#endif
 }

//Function to manage cover states
 int manageFocuserState( FocuserStates targetFocuserState )
 {
  String msg = "";
  //need a targetFocuserState.
  if ( focuser.focuserState == targetFocuserState ) 
    return 0;
   
  switch( focuser.focuserState )
  {          
      //current status is Closed, new target status is asking for a different action. 
      case FocuserStates::FOCUSER_MOVING:
          //current status is moving, new target status is asking for a different action. 
          switch (targetFocuserState)
          {
            //If we change these target states while moving, the timer handler managing movement will respond automatically
            case FocuserStates::FOCUSER_IDLE:
                  debugD("Still moving  - %s to %s", focuserStateCh[focuserState], focuserStateCh[targetFocuserState] );
                  break;
            case FocuserStates::FOCUSER_HALTED:
                  debugD( "Change of state requested from Moving to Halted - halting Focuser if not already stopped");
                  break;
            case FocuserStates::FOCUSER_MOVING:
                  debugD( "Change of state requested from Moving to Halted - halting Focuser if not already stopped");
                  break;
            default:
                  debugW("Requested target state of %s while moving - does it make sense ?", focuserStateCh[targetFocuserState] ); 
                  break;
          }
          break;
      //Used to be Halted but Halted is not an ASCOM value - unknown should be returned as the state when the flap is halted during movement or before initialised
      case FocuserStates::FOCUSER_IDLE: 
          switch( targetFocuserState )
          {
            case FocuserStates::FOCUSER_IDLE:
                 debugD("targetFocuserState changed to IDLE from IDLE");
                 break;
            case FocuserStates::FOCUSER_MOVING:
                 debugD("targetFocuserState changed to MOVING from IDLE. Starting Focuser");
                 focuserState = FocuserStates::FOCUSER_MOVING;
                 //turn on the timer to move the servo smoothly
                 FocuserStateFlag = 0;
                 Focuser.Enable( &FocuserStateFlag );
                 focuserState = FocuserStates::FOCUSER_MOVING;
                 break;
            case FocuserStates::FOCUSER_HALTED:
                 //Already idle. 
                 Focuser.Halt( 0 );
            default:
                 debugW("Unexpected targetFocuserState %s from IDLE", focuserStateCh[ targetFocuserState ] );
              break;
          }
          break;
      case FocuserStates::FOCUSER_HALTED: 
          switch( targetFocuserState )
          {
            case FocuserStates::FOCUSER_MOVING:
              debugD("targetFocuserState set to MOVING from HALTED - position : %d", position );
              focuserState = FocuserStates::FOCUSER_MOVING;
              break;
            case FocuserStates::FOCUSER_HALTED:
              debugD("targetFocuserState set to Halted from HALTED, Focuser status is: %d ", Focuser.Status() );
              break;
            case FocuserStates::FOCUSER_IDLE:
              debugD("targetFocuserState set to IDLE from HALTED, position is %d ", position );
              focuserState = FocuserStates::FOCUSER_IDLE;
              break;
            default:
              debugW("unexpected targetFocuserState from Open %s", focuserStateCh[targetFocuserState]);
              break;
          }
      default:
          break;
  }
  //debugD( "Exiting - final state %s, targetFocuserState %s ", focuserStatusCh[focuserState], focuserStatusCh[targetfocuserState] );
  return 0;
}

/* MQTT callback for subscription and topic.
 * Only respond to valid states ""
 * Publish under ~/skybadger/sensors/<sensor type>/<host>
 * Note that messages have an maximum length limit of 18 bytes - set in the MQTT header file. 
 */
void callback(char* topic, byte* payload, unsigned int length) 
{  
  //set callback flag
  callbackFlag = true;  
}

/*
 * Had to do a lot of work to get this to work 
 * Mostly around - 
 * length of output buffer
 * reset of output buffer between writing json strings otherwise it concatenates. 
 * Writing to serial output was essential.
 */
 void publishHealth( void )
 {
  String outTopic;
  String output;
  String timestamp;
  
  //checkTime();
  getTimeAsString2( timestamp );

  //publish to our device topic(s)
  DynamicJsonBuffer jsonBuffer(256);
  JsonObject& root = jsonBuffer.createObject();
  root["Time"] = timestamp;
  root["hostname"] = myHostname;
  root["Message"] = "Listening";
  
  root.printTo( output);
  
  //Put a notice out regarding device health
  outTopic = outHealthTopic;
  outTopic.concat( myHostname );
  if ( client.publish( outTopic.c_str(), output.c_str(), true ) )
  {
    debugI( "topic: %s, published with value %s \n", outTopic.c_str(), output.c_str() );
  }
  else
  {
    debugW( "Failed to publish : %s to %s", output.c_str(), outTopic.c_str() );
  }
 }
 
 //Publish state to the function subscription point. 
 void publishFunction(void )
 {
  String outTopic;
  String output;
  String timestamp;
  
  //checkTime();
  getTimeAsString2( timestamp );

  //publish to our device topic(s)
  DynamicJsonBuffer jsonBuffer(256);
  JsonObject& root = jsonBuffer.createObject();
  
  root[F("Time")] = timestamp;
  root[F("hostname")] = myHostname;
  root[F("position")] = (int) position;
  root[F("tempCompMode")] = (int) tempCompEnabled;
  
  root.printTo( output);
  
  //Put a notice out regarding device health
  outTopic = outFnTopic;
  outTopic.concat( myHostname );
  outTopic.concat( "/" );
  outTopic.concat( DriverType );
  
  if ( client.publish( outTopic.c_str(), output.c_str(), true ) )
  {
    debugI( "topic: %s, published with value %s \n", outTopic.c_str(), output.c_str() );
  }
  else
  {
    debugW( "Failed to publish : %s to %s", output.c_str(), outTopic.c_str() );
  }
 }

 void setup_wifi()
{
  int zz = 0;
  WiFi.mode(WIFI_STA); 
  WiFi.hostname( myHostname );
  WiFi.begin( ssid2, password2 );
  //WiFi.begin( ssid1, password1 );
  Serial.print(F("Searching for WiFi.."));
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
      Serial.print(F("."));
   if ( zz++ > 200 ) 
    device.restart();
  }

  Serial.println(F("WiFi connected") );
  Serial.printf("SSID: %s, Signal strength %i dBm \n\r", WiFi.SSID().c_str(), WiFi.RSSI() );
  Serial.printf("Hostname: %s\n\r",      WiFi.hostname().c_str() );
  Serial.printf("IP address: %s\n\r",    WiFi.localIP().toString().c_str() );
  Serial.printf("DNS address 0: %s\n\r", WiFi.dnsIP(0).toString().c_str() );
  Serial.printf("DNS address 1: %s\n\r", WiFi.dnsIP(1).toString().c_str() );

  //Setup sleep parameters
  //wifi_set_sleep_type(LIGHT_SLEEP_T);
  wifi_set_sleep_type(NONE_SLEEP_T);

  delay(5000);
  Serial.println( "WiFi connected" );
}
