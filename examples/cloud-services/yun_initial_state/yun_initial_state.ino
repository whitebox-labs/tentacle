// WhiteBox Labs -- Tentacle Shield -- Send sensor readings to the cloud: initialstate.com
// https://www.whiteboxes.ch/tentacle
//
//
// This sample code was written on an Arduino YUN, and depends on it's Bridge library to
// communicate wirelessly. For Arduino Mega, Uno etc, see the respective examples.
//
// This example shows how to take readings from the sensors in an asynchronous way, completely
// without using any delays. This allows to do other things while waiting for the sensor data.
// The sensor data is uploaded to InitialState cloud service. You will need an account at initialstate.com
//
// This example sketch includes code from https://github.com/InitialState/arduino_streamers
// (check the above link to see how to make the networking code compatible with other ethernet/wifi shields)
//
// USAGE:
//---------------------------------------------------------------------------------------------
// - Set all your EZO circuits to I2C before using this sketch.
//    - You can use the "tentacle-steup.ino" sketch to do so)
//    - Make sure each circuit has a unique I2C ID set
// - Change the variables NUM_CIRCUITS, channel_ids and channel_names to reflect your setup
// - Configure the InitialState settings: accessKey, bucketKey, bucketName
// - To talk to the Yun console, select your Yun's name and IP address in the Arduino IDE Port menu.
//    - The Yun will only show up in the Ports menu, if your computer is on the same Network as the Yun.
//    - If your Yun does not appear in the Ports menu, make sure the Yun is connected to your network
//      (see the basic Yun tutorials on the Arduino website to learn how to setup your Yun)
//
//---------------------------------------------------------------------------------------------
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------------------------------

#include <Wire.h>                              // enable I2C.
#include <Console.h>                           // Yun Console
#include <Process.h>                           // Yun Process, to run processes on the Yun's linux 


#define NUM_CIRCUITS 4                         // <-- CHANGE THIS  set how many I2C circuits are attached to the Tentacle

const unsigned int send_readings_every = 2000; // set at what intervals the readings are sent to the computer (NOTE: this is not the frequency of taking the readings!)
unsigned long next_serial_time;

char sensordata[30];                          // A 30 byte character array to hold incoming data from the sensors
byte sensor_bytes_received = 0;               // We need to know how many characters bytes have been received
byte code = 0;                                // used to hold the I2C response code.
byte in_char = 0;                             // used as a 1 byte buffer to store in bound bytes from the I2C Circuit.

int channel_ids[] = {97, 98, 99, 100};        // <-- CHANGE THIS. A list of I2C ids that you set your circuits to.
String channel_names[] = {"DO", "ORP", "PH", "EC"};   // <-- CHANGE THIS. A list of channel names (must be the same order as in channel_ids[]) - only used to give a name to the "Signals" sent to initialstate.com
String sensor_data[NUM_CIRCUITS];             // an array of strings to hold the readings of each channel
int channel = 0;                              // INT pointer to hold the current position in the channel_ids/channel_names array

const unsigned int reading_delay = 1000;      // time to wait for the circuit to process a read command. datasheets say 1 second.
unsigned long next_reading_time;              // holds the time when the next reading should be ready from the circuit
boolean request_pending = false;              // wether or not we're waiting for a reading

const unsigned int cloud_update_interval = 10000; // time to wait for the circuit to process a read command. datasheets say 1 second.
unsigned long next_cloud_update;              // holds the time when the next reading should be ready from the circuit



// INITIAL STATE CONFIGURATION
#define ISBucketURL "https://groker.initialstate.com/api/buckets" // bucket creation url. 
#define ISEventURL  "https://groker.initialstate.com/api/events"  // data destination. Thanks to Yun's linux, we can use SSL, yeah :)       
String bucketKey = "arduino12345";                                 // unique identifier for your bucket (unique in the context of your access key)
String bucketName = "Water Quality Monitor Yun";                  // Bucket name. Will be used to label your Bucket in the initialstate website.
String accessKey = "your-access-key-from-initialstate.com";       // <-- CHANGE THIS Access key (copy/paste from your initialstate account settings page)
// NOTE: when changing bucketName, make sure you change bucketKey, too. Bucket key can be anything - it's used to differentiate buckets, in case you have multiple.



void setup() {
  pinMode(13, OUTPUT);                                 // set the led output pin
  Bridge.begin();
  Console.begin();                                     // initialize serial communication over network:
  while (!Console) ;                                   // wait for Console port to connect.
  Console.println("starting");
  Wire.begin();			                       // enable I2C port.

  delay(1000);                                         // Time for the ethernet shield to boot

  createBucket();                                      // create a bucket, if it doesn't exist yet
  next_serial_time = millis() + send_readings_every*3; // wait a little longer before sending serial data the first time
  next_cloud_update = millis() + cloud_update_interval;
}



void loop() {
  updateSensors();                // read / write to the sensors. returns fast, does not wait for the data to arrive
  updateSerial();                 // write sensor data to the serial port
  updateCloud();                  // send the sensor data to the cloud. returns fast, except when a cloud update is due.
  // do your other arduino stuff here
}


// send the data to the cloud - but only when it's time to do so
void updateCloud() {
  
  if (millis() >= next_cloud_update) {                // is it time for the next serial communication?

    sendData();

    next_cloud_update = millis() + cloud_update_interval;
  }
}



// do serial communication in a "asynchronous" way
void updateSerial() {
  if (millis() >= next_serial_time) {              // is it time for the next serial communication?
    Console.println("---------------");
    for (int i = 0; i < NUM_CIRCUITS; i++) {       // loop through all the sensors
      Console.print(channel_names[i]);             // print channel name
      Console.print(":\t");
      Console.println(sensor_data[i]);             // print the actual reading
    }
    Console.println("---------------");
    next_serial_time = millis() + send_readings_every;
  }
}



// take sensor readings in a "asynchronous" way
void updateSensors() {
  if (request_pending) {                          // is a request pending?
    if (millis() >= next_reading_time) {          // is it time for the reading to be taken?
      receiveReading();                           // do the actual I2C communication
    }
  } else {                                        // no request is pending,
    channel = (channel + 1) % NUM_CIRCUITS;       // switch to the next channel (increase current channel by 1, and roll over if we're at the last channel using the % modulo operator)
    requestReading();                             // do the actual I2C communication
  }
}



// Request a reading from the current channel
void requestReading() {
  request_pending = true;
  Wire.beginTransmission(channel_ids[channel]); // call the circuit by its ID number.
  Wire.write('r');        		        // request a reading by sending 'r'
  Wire.endTransmission();          	        // end the I2C data transmission.
  next_reading_time = millis() + reading_delay; // calculate the next time to request a reading
}



// Receive data from the I2C bus
void receiveReading() {
  sensor_bytes_received = 0;                        // reset data counter
  memset(sensordata, 0, sizeof(sensordata));        // clear sensordata array;

  Wire.requestFrom(channel_ids[channel], 48, 1);    // call the circuit and request 48 bytes (this is more then we need).
  code = Wire.read();
  
  while (Wire.available()) {          // are there bytes to receive?
    in_char = Wire.read();            // receive a byte.

    if (in_char == 0) {               // if we see that we have been sent a null command.
      Wire.endTransmission();         // end the I2C data transmission.
      break;                          // exit the while loop, we're done here
    }
    else {
      sensordata[sensor_bytes_received] = in_char;  // load this byte into our array.
      sensor_bytes_received++;
    }
  }
  
  char *filtered_sensordata;                     // pointer to hold a modified version of the data
  filtered_sensordata = strtok (sensordata,","); // we split at the first comma - needed for the ec stamp only

  switch (code) {                  	    // switch case based on what the response code is.
    case 1:                       	    // decimal 1  means the command was successful.
      sensor_data[channel] = filtered_sensordata;
      break;                        	    // exits the switch case.

    case 2:                        	    // decimal 2 means the command has failed.
      Console.print("channel \"");
      Console.print( channel_names[channel] );
      Console.println ("\": command failed");
      break;                         	    // exits the switch case.

    case 254:                      	    // decimal 254  means the command has not yet been finished calculating.
      Console.print("channel \"");
      Console.print( channel_names[channel] );
      Console.println ("\": reading not ready");
      break;                         	    // exits the switch case.

    case 255:                      	    // decimal 255 means there is no further data to send.
      Console.print("channel \"");
      Console.print( channel_names[channel] );
      Console.println ("\": no answer");
      break;                         	    // exits the switch case.
  }
  request_pending = false;                  // set pending to false, so we can continue to the next sensor
}



// make a HTTP connection to the server and send create the InitialState Bucket (if it doesn't exist yet)
void createBucket()
{
  // Initialize the process
  Process isbucket;

  isbucket.begin("curl");
  isbucket.addParameter("-k");    // we use SSL, but we bypass certificate verification! 
  isbucket.addParameter("-v");
  isbucket.addParameter("-X");
  isbucket.addParameter("POST");
  isbucket.addParameter("-H");
  isbucket.addParameter("Content-Type:application/json");
  isbucket.addParameter("-H");
  isbucket.addParameter("Accept-Version:0.0.1");

  // IS Access Key Header
  isbucket.addParameter("-H");
  isbucket.addParameter("X-IS-AccessKey:" + accessKey);

  // IS Bucket Key Header
  isbucket.addParameter("-d");
  isbucket.addParameter("{\"bucketKey\": \"" + bucketKey + "\", \"bucketName\": \"" + bucketName + "\"}");
  
  isbucket.addParameter(ISBucketURL);
  
  // Run the process
  isbucket.run();
}



// make a HTTP connection to the server and send the sensor readings
void sendData()
{
  // Initialize the process
  Process isstreamer;

  isstreamer.begin("curl");
  isstreamer.addParameter("-k");  // we use SSL, but we bypass certificate verification! 
  isstreamer.addParameter("-v");
  isstreamer.addParameter("-X");
  isstreamer.addParameter("POST");
  isstreamer.addParameter("-H");
  isstreamer.addParameter("Content-Type:application/json");
  isstreamer.addParameter("-H");
  isstreamer.addParameter("Accept-Version:0.0.1");

  // IS Access Key Header
  isstreamer.addParameter("-H");
  isstreamer.addParameter("X-IS-AccessKey:" + accessKey);

  // IS Bucket Key Header
  // Note that bucketName is not needed here
  isstreamer.addParameter("-H");
  isstreamer.addParameter("X-IS-BucketKey:" + bucketKey);

  isstreamer.addParameter("-d");

  // Initialize a string to hold our signal data
  String jsonData;

  jsonData = "[";

  for (int i=0; i<NUM_CIRCUITS; i++)
  {
    jsonData += "{\"key\": \"" + channel_names[i] + "\", \"value\": \"" + sensor_data[i] + "\"}";

    if (i != NUM_CIRCUITS - 1)
    {
      jsonData += ",";
    }
  }

  jsonData += "]";

  isstreamer.addParameter(jsonData);

  isstreamer.addParameter(ISEventURL);

  // Print posted data for debug
  Console.print("Sending data: ");
  Console.println(jsonData);

  // Run the process
  isstreamer.run();
}
