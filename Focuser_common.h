/*
ASCOM project common variables

ESP8266_coverhandler.h
This is a firmware application to implement the ASCOM ALPACA cover calibrator API.
Each device can manage one RC based flap and a power switch to turn on or off a calbrator light source
Each power switch can be 1 of 2 types - binary switch or digital pwm intensity control.
The setup allows specifying:
  the host/device name.
  The UDP management interface listening port. 

This device assumes the use of ESP8266 devices to implement. 
Using an ESP8266_01 this leaves one pin free to be a digital device. 
Using an ESP8266_12 this leaves a number of pins free to be a digital device.

 To do:
 Debug, trial
 
 Layout: 
 (ESP8266_12)
 GPIO 4,2 to SDA
 GPIO 5,0 to SCL 
 (ESP8266_01)
 GPIO 0 - SDA
 GPIO 1 - Rx - re-use as PWM output for testing purposes
 GPIO 2 - SCL
 GPIO 3 - Tx
 All 3.3v logic. 
 
*/
#pragma once

#ifndef _COVERCAL_COMMON_H_
#define _COVERCAL_COMMON_H_

//Define the processor
#define ESP8266_01

#define DEBUG
//Enables basic debugging statements for ESP
#define DEBUG_ESP_MH       

//Manage the remote debug interface, it takes 6K of memory with all the strings even when not in use but loaded
//#define DEBUG_DISABLED //Check the impact on the serial interface. 

//#define DEBUG_DISABLE_AUTO_FUNC    //Turn on or off the auto function labelling feature .
#define WEBSOCKET_DISABLED true      //No impact to memory requirement
#define MAX_TIME_INACTIVE 0          //to turn off the de-activation of the debug telnet session

//Use for client testing
//#define DEBUG_MQTT            //enable low-level MQTT connection debugging statements.    

//Define direction constants
#define DIRN_CW 0
#define DIRN_CCW 1

//Manage different pinout variants of the ESP8266
#define __ESP8266_12

#ifdef __ESP8266_12
#define DIRN_PIN 4
#define STEP_PIN 5
#define ENABLE_PIN 2
#define HALF_STEP_PIN 7
#define BUTN_A_PIN 13
#define BUTN_B_PIN 14
#define BUTN_C_PIN 15

#else // __ESP8266_01
#define DIRN_PIN 2
#define STEP_PIN 3
#endif

#include "DebugSerial.h"
//Remote debugging setup 
#include "RemoteDebug.h"  //https://github.com/JoaoLopesF/RemoteDebug
#include "SkybadgerStrings.h"

const int MAX_NAME_LENGTH  = 40;
#include <skybadgerstrings.h>
char* myHostname = nullptr;
char* thisID   = nullptr;

const char* ASCOM_DEVICE_TYPE = "focuser"; //used in server handler uris
//ASCOM variables 
//ASCOM driver common variables 
unsigned int transactionId;
unsigned int clientId;
int connectionCtr = 0; //variable to count number of times something has connected compared to disconnected. 
extern const unsigned int NOT_CONNECTED;//Sourced from ASCOM_COMMON
unsigned int connected = NOT_CONNECTED;
const String DriverName = "Skybadger.ESPFocuser";
const String DriverVersion = "0.0.1";
const String DriverInfo = "Skybadger.ESPFocuser RESTful native device. ";
const String Description = "Skybadger ESP2866-based wireless ASCOM Focuser";
const String InterfaceVersion = "2";
const String DriverType = "Focuser"; //Must be a valid ASCOM type to be recognised by UDP discovery. 
char* focuserName = NULL;

const char* defaultHostname = "espFoc00";
char GUID[] = "0014-0000-0000-0000";

//UDP Port can be edited in setup page - eventually.
#define ALPACA_DISCOVERY_PORT 32227
int udpPort = ALPACA_DISCOVERY_PORT;

//Location setting for Management reporting
char* Location = nullptr;
int instanceNumber  = defaultInstanceNumber; //Does this need to be zero to reflect the api or something else ? 
const int defaultInstanceNumber = 0;
float instanceVersion = 1.0;

//specifying the type of motor to be supported. DC brushed motors will need an encoder. Encoder support is not yet added. Not that its not easy. 
const char* validMotorTypes[] = { "Stepper", "DC brushed" };

#define TZ              0       // (utc+) TZ in hours
#define DST_MN          60      // use 60mn for summer time in some countries
#define TZ_MN           ((TZ)*60)
#define TZ_SEC          ((TZ)*3600)
#define DST_SEC         ((DST_MN)*60)

#if defined ESP8266_01
//Verified experimentally - normal assignments
//GPIO 0 is I2C SCL      - pin 2
//GPIO 1 is Tx Serial    - pin 5
//GPIO 2 is I2C SDA      - pin 3
//GPIO 3 is Rx Serial    - pin 1 

//Pin assignments
const int NULLPIN  = -1;
const int SDAPIN   = 2;    //SCL is GPIO 2 on pin 2  
const int SCLPIN   = 0;   //SDA IS GPIO 0 on pin 3
const int STEPPIN  = 2;
const int DIRNPIN  = 0;
const int ENPIN    = 3;
const int pinMap[] = { 1, NULLPIN}; 

#elif defined ESP8266_12
//Pin assignments
const int SDAPIN  = 4;     //SDA
const int SCLPIN  = 5;    //SCL 
const int STEPPIN = 4;
const int DIRNPIN = 5;
const int ENPIN   = 6;
//Most pins are used for flash, so we assume those for SSI are available.
//Typically use 4 and 5 for I2C, leaves 
const int pinMap[] = { 2, 14, 15, NULLPIN };

#else 
#pragma warning "ESP type not specified - pin map not set correctly" 
const int pinMap[] = {NULLPIN};
#endif 

#endif
