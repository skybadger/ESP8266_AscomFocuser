<h1>ESP8266_ASCOMCoverCal</h1>
This application provides a remote ASCOM-enabled Telescope focuser for use in Astronomical observatories and instrument setups. 

This instance runs on the ESP8266-01 wifi-enabled SoC device to provide remote REST access and control of the focusing motor. 
This driver is closely related to the ASCOM Filtyer wheel driver. 
The device reports system health to the central MQTT service and supports OTA updates using the http://<hostname>/update interface.

The device implements simple client (STATION) WiFi, including setting its hostname in the local network DNS and requires use of local DHCP services to provide a device IPv4 address, naming and network resolution services, and NTP time services. 

The unit takes 12v DC power only and provides drive output to a bipolar 2-coil 4-wire stepper motor that is assumed to be appropriately geared. 
The unit can be adapted to drive two DC motors bi-directionally with teh loss of positional control due to loss of step encoding. 

<h2>Dependencies:</h2>
<ul><li>Arduino 1.86 IDE, </li>
<li>ESP8266 V2.6+ in lwip1.4 low bandwidth mode</li>
<li>Arduino MQTT client (https://pubsubclient.knolleary.net/api.html)</li>
<li>Arduino JSON library (pre v6) </li>
<li>RemoteDebug library </li>
<li>Common Alpaca and Skybadger helper files </li>
</ul>

<h3>Testing</h3>
In single servo mode, the Tx pin is free for use as a TTL serial output. 
Read-only monitoring by serial port - Tx only is available from device at 115,600 baud (8n1) at 3.3v. This provides debug monitoring output via Putty or another com terminal.
Wifi is used for MQTT reporting only and servicing REST API web requests
Use http://<hostname>/status to receive json-formatted output of current pins. 
Use the batch file to test direct URL response via CURL. 
Setup the ASCOM remote client and use the ASCOM script file ( Python or VBS) file to test response of the switch as an ASCOM device using the ASCOM remote interface. 

<h3>Use</h3>
Install latest <a href="https://www.ascom-standards.org/index.htm">ASCOM drivers </a> onto your platform. Add the <a href="https://www.ascom-standards.org/Developer/Alpaca.htm"> ASCOM ALPACA </a> remote interface.
Start the remote interface, configure it for the DNS name above on port 80 and select the option to explicitly connect.
ASCOM platfrom 6.5 adds some intelligence for auto-discovery which i am still working through but should be a good thing. 

Setup your device with a name and location and update the hostname as required. 
Specify what your focuser driver limits and resolution are. 

Use the custom setup Urls: 
<ul>
 <li>http://"hostname"/api/v1/focuser/0/setup - web page to manually configure settings. </li>
 <li>http://"hostname"/api/v1/focuser/0/status - json listing of current device status</li>
 <li>http://"hostname"/update                 - Update the firmware from your browser </li>
 <li>http://"hostname"/restart                - Restart - sometimes required after a network glitch </li>
 </ul>
Once configured, the device keeps your settings through reboot and power outage by use of the onboard EEProm memory.

This driver supports Alpaca auto-discovery via the Ascom Alpaca mechanism on the standard port. 

<h3>ToDo:</h3>

 
<h3>Caveats:</h3> 
Currently there is no user access control on the connection to the web server interface. Anyone can connect. So use this behind a well-managed reverse proxy.
Also, note that the 'connected' settings is checked to ensure that the REST command is coming from a client who has previously called 'connected' and should effectively be in charge of the device from that point. Hence that user must then call disconnect to relinquish ownership. By default this device is only controllable by one device at a time. 

<h3>Structure:</h3>
This code pulls the source code into the file using header files inclusion. Hence there is an implied order, typically importing ASCOM headers last. At some point I will re-factor to look more like standard C compilation practice.

