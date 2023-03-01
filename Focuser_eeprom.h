/*
File to define the eeprom variable save/restor operations for the ASCOM CCal web driver
*/
#ifndef _COVERCAL_EEPROM_H_
#define _COVERCAL_EEPROM_H_

#include "Focuser_common.h"
#include "DebugSerial.h"
#include "eeprom.h"
#include "EEPROMAnything.h"

static const byte magic = '*';

//definitions
void setDefaults(void );
void saveToEeprom(void);
void setupFromEeprom(void);

/*
 * Write default values into variables - don't save yet. 
 */
void setDefaults( void )
{
  DEBUGSL1( F("Eeprom setDefaults: entered") );

  if ( myHostname != nullptr ) 
     free ( myHostname );
  myHostname = (char* )calloc( sizeof (char), MAX_NAME_LENGTH );
  strcpy( myHostname, defaultHostname);
  WiFi.hostname( myHostname );

  if ( Location != nullptr ) 
     free ( Location );
  Location = (char* )calloc( sizeof (char), MAX_NAME_LENGTH );
  strcpy( Location, "update me" );

  //MQTT thisID copied from hostname
  if ( thisID != nullptr ) 
     free ( thisID );
  thisID = (char*) calloc( MAX_NAME_LENGTH, sizeof( char)  );       
  strcpy ( thisID, myHostname );

  udpPort = ALPACA_DISCOVERY_PORT; 
     
#if defined DEBUG
  //Read them back for checking  - also available via status command.
  Serial.printf( "Hostname: %s \n" , myHostname );
  Serial.printf( "Discovery port: %i \n" , udpPort );
  Serial.printf( "Location: %s \n" , Location );
  
#endif
  DEBUGSL1( "setDefaults: exiting" );
}

/*
 * Save current variables of interest into EEPROM
 */
void saveDeviceToEeprom( void )
{
  int eepromAddr = 4;
  DEBUGSL1( "savetoEeprom: Entered ");
   
  //UDP Port
  EEPROMWriteAnything( eepromAddr, udpPort );
  eepromAddr += sizeof(int);  
  DEBUGS1( "Written udpPort: ");DEBUGSL1( udpPort );

  //hostname
  EEPROMWriteString( eepromAddr, myHostname, MAX_NAME_LENGTH );
  eepromAddr += MAX_NAME_LENGTH;   
  DEBUGS1( "Written hostname: ");DEBUGSL1( myHostname );

  //Location for Management
  EEPROMWriteString( eepromAddr, Location, MAX_NAME_LENGTH );
  eepromAddr += MAX_NAME_LENGTH;   
  DEBUGS1( "Written Location: ");DEBUGSL1( Location );
  
  EEPROMWriteString( eepromAddr, mqttTemperatureSource, MAX_NAME_LENGTH );
  eepromAddr += MAX_NAME_LENGTH;   
  DEBUGS1( "Written mqttTemperatureSource: ");DEBUGSL1( mqttTemperatureSource );

  DEBUGSL1( F("Finished writing all eeprom data ") );
  
  //Magic number write for data write complete. 
  EEPROM.put( 0, magic );
  EEPROM.commit();
  
  DEBUGS1( F("Wrote "));DEBUGS1(eepromAddr);DEBUGSL1( F(" bytes "));
   
#if defined DEBUG
  //Test readback of contents
  String input = "";
  char ch;
  for ( int i = 0; i < EEPROMSIZE ; i++ )
  {
    ch = (char) EEPROM.read( i );
    if ( ch == '\0' )
      ch = '~';
    if ( (i % 32 ) == 0 )
      input.concat( "\n\r" );
    input.concat( ch );
  } 
  Serial.printf( "EEPROM contents after: \n %s \n", input.c_str() );
#endif 
  
  DEBUGSL1( "saveToEeprom: exiting ");
}

void setupFromEeprom( void )
{
  int eepromAddr = 0;
    
  DEBUGSL1( F("setUpFromEeprom: Entering "));
  byte myMagic = '\0';
  //Setup internal variables - read from EEPROM.
  myMagic = EEPROM.read( 0 );
  DEBUGS1( "Read magic: ");DEBUGSL1( (char) myMagic );
  
  if ( (byte) myMagic != magic ) //initialise eeprom for first time use. 
  {
    setDefaults();
    saveToEeprom();
    DEBUGSL1( "Failed to find init magic byte - wrote defaults.");
    return;
  }    
    
  //UDP port 
  EEPROMReadAnything( eepromAddr = 4, udpPort );
  eepromAddr  += sizeof(int);  
  DEBUGS1( "Read UDPport: "); DEBUGSL1( udpPort );

  //hostname - directly into variable array 
  if( myHostname != nullptr )
    free( myHostname );
  myHostname = (char*) calloc( MAX_NAME_LENGTH, sizeof( char ) );  
  EEPROMReadString( eepromAddr, myHostname, MAX_NAME_LENGTH );
  eepromAddr  += MAX_NAME_LENGTH * sizeof(char);  
  DEBUGS1( F("Read hostname: ")); DEBUGSL1( myHostname );
  
  //Setup MQTT client id based on hostname
  if ( thisID != nullptr ) 
     free ( thisID );
  thisID = (char*) calloc( MAX_NAME_LENGTH, sizeof( char)  );       
  strcpy ( thisID, myHostname );
  DEBUGS1( F("Read MQTT ID: "));DEBUGSL1( thisID );
  
  //Location
  if( Location != nullptr )
    free( Location );
  Location = (char*) calloc( MAX_NAME_LENGTH, sizeof( char ) );  
  EEPROMReadString( eepromAddr, Location, MAX_NAME_LENGTH );
  eepromAddr  += MAX_NAME_LENGTH * sizeof(char);  
  DEBUGS1( F("Read location: ")); DEBUGSL1( Location );

  //Temperature MQTT source host 
  if( mqttTemperatureSource != nullptr )
    free( mqttTemperatureSource );
  mqttTemperatureSource = (char*) calloc( MAX_NAME_LENGTH, sizeof( char ) );  
  EEPROMReadString( eepromAddr, mqttTemperatureSource, MAX_NAME_LENGTH );
  eepromAddr  += MAX_NAME_LENGTH * sizeof(char);  
  DEBUGS1( F("Read mqttTemperatureSource: ")); DEBUGSL1( mqttTemperatureSource );
  
  DEBUGSL1( F("setupFromEeprom: exiting") );
}
#endif
