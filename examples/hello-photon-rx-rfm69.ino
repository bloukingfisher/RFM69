#include "RFM69-Particle/RFM69-Particle.h"
#include "RFM69-Particle/RFM69_ATC.h"
#include "RFM69-Particle/RFM69registers.h"

// Adjustments to library to work with Particle Photon including in Web IDE by Jurie Pieterse
// Forked library for Photon at https://github.com/bloukingfisher/RFM69/
// Serial NOT required to confirm working - you can watch your Particle Console logs!

/* RFM69 library and code by Felix Rusu - felix@lowpowerlab.com
// Get libraries at: https://github.com/LowPowerLab/
// Make sure you adjust the settings in the configuration section below !!!
// **********************************************************************************
// Copyright Felix Rusu, LowPowerLab.com
// Library and code by Felix Rusu - felix@lowpowerlab.com
// **********************************************************************************
// License
// **********************************************************************************
// This program is free software; you can redistribute it 
// and/or modify it under the terms of the GNU General    
// Public License as published by the Free Software       
// Foundation; either version 3 of the License, or        
// (at your option) any later version.                    
//                                                        
// This program is distributed in the hope that it will   
// be useful, but WITHOUT ANY WARRANTY; without even the  
// implied warranty of MERCHANTABILITY or FITNESS FOR A   
// PARTICULAR PURPOSE. See the GNU General Public        
// License for more details.                              
//                                                        
// You should have received a copy of the GNU General    
// Public License along with this program.
// If not, see <http://www.gnu.org/licenses></http:>.
//                                                        
// Licence can be viewed at                               
// http://www.gnu.org/licenses/gpl-3.0.txt
//
// Please maintain this license information along with authorship
// and copyright notices in any redistribution of this code
// **********************************************************************************/


//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/CONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************
int16_t NETWORKID = 100;  //the same on all nodes that talk to each other
int16_t NODEID = 1;  

//Match frequency to the hardware version of the radio on your Feather
//#define FREQUENCY     RF69_433MHZ
//#define FREQUENCY     RF69_868MHZ
#define FREQUENCY      RF69_915MHZ
#define ENCRYPTKEY     "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
#define IS_RFM69HCW    true // set to 'true' if you are using an RFM69HCW module

//*********************************************************************************************

#define RFM69_CS      A2 
#define RFM69_IRQ     2
#define RFM69_IRQN    2 //On Photon it is the same unlike Arduino
#define RFM69_RST     6

int16_t packetnum = 0;  // packet counter, we increment per xmission

RFM69 radio = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN);  //initialize radio with potential custom pin outs; otherwise you may use for default: RFM69 radio;

//*********************************************************************************************
// ***********                  Wiring the RFM69 Radio to Photon                  *************
//*********************************************************************************************
/* Arduino wiring provided for reference, color 
    Photon      Arduino	    RFM69	Color
    GND         GND	        GND	    Black
    3V3         3.3V	    VCC	    Red
    A2          10	        NSS	    Yellow
    A3          13	        SCK	    Green
    A5          11	        MOSI	Blue
    A4          12	        MISO	Violet
    D2          2	        DI00	Gray
                            ANA	    Antenna
    D6                      RST     Optional
*/

void setup() 
{
 
    Serial.begin(115200); // Open serial monitor at 115200 baud to see ping results.
    Particle.publish("RFM69 RX Startup setup","Completed",360,PRIVATE);
    Particle.publish("WiFi signal",String(WiFi.RSSI()),360,PRIVATE);
    Serial.println("RFM69 Based Receiver");
  
    // Hard Reset the RFM module - Optional
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, HIGH);
  delay(100);
  digitalWrite(RFM69_RST, LOW);
  delay(100);
  
    // Initialize radio
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
  if (IS_RFM69HCW) {
    radio.setHighPower();    // Only for RFM69HCW & HW!
  }
  
  // To improve distance set a lower bit rate. Most libraries use 55.55 kbps as default
  // See https://lowpowerlab.com/forum/moteino/rfm69hw-bit-rate-settings/msg1979/#msg1979
  // Here we will set it to 9.6 kbps instead 
  radio.writeReg(0x03,0x0D); //set bit rate to 9k6
  radio.writeReg(0x04,0x05);
  
  radio.setPowerLevel(10); // power output ranges from 0 (5dBm) to 31 (20dBm)
                          // Note at 20dBm the radio sources up to 130 mA! 
                         // Selecting a power level between 10 and 15 will use ~30-44 mA which is generally more compatible with Photon power sources
                        // As reference, power level of 10 transmits successfully at least 300 feet with 0% packet loss right through a home, sufficient for most use
    
  radio.encrypt(ENCRYPTKEY);
  
  Serial.print("\nListening at ");
  Serial.print(FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  Serial.println(" MHz");
}


//=========================MAIN LOOP===========================================
void loop() {

Serial.print("."); //THis gives us a neat visual indication of time between messages received

  //check if something was received (could be an interrupt from the radio)
  if (radio.receiveDone())
  {
    //print message received to serial
    Serial.println(" ");
    Serial.print('[');Serial.print(radio.SENDERID);Serial.print("] ");
    Serial.print((char*)radio.DATA);
    Serial.print("   [RX_RSSI:");Serial.print(radio.RSSI);Serial.print("]");
    
    //send message to Particle console if not using serial
    String RXMessage = "[" + String(radio.SENDERID) + "]  " + String((char*)radio.DATA) + " [RSSI: " + String(radio.RSSI) + "]";
    Particle.publish("Message received",RXMessage,360,PRIVATE);

    //check if received message contains Hello
    if (strstr((char *)radio.DATA, "Hello"))
    {
      //check if sender wanted an ACK
      if (radio.ACKRequested())
      {
        radio.sendACK();
        Serial.println(" - ACK sent");
      }
      else {Serial.println(" - No ACK sent");}
    }  
  }

  radio.receiveDone(); //put radio in RX mode
  
  delay(1000);


} //end loop

 
