/*
 NetworkedRFIDController Sketch by Mark Bilandzic, 11 April 2012
 
 This sketch reads an RFID card through an RFID reader and makes a GET request to a URL with the RFID numbers as a parameter.
 The sketch is written for the SNARC (Simple NetworkAble RFID Controller), designed by Lawrence "Lemming" Dixon http://www.hsbne.org/projects/SNARC, however
 the code is fully compatible with an Arduino 1.0+ and attached Ethernet-Shield.
 
 The sketch was written to control a Checkin Point as part of the Gelatine project:
 Gelatine project: http://kavasmlikon.wordpress.com/2013/07/09/gelatine-designing-for-digital-encounters-in-coworking-spaces/
 Gelatine Checkin Point: http://kavasmlikon.wordpress.com/2013/07/16/gelatine-checkin-point/ ‎
 
 Code is based on:
 - RFID reader code by
     - Martijn The - http://www.martijnthe.nl/ 
     - BARRAGAN - http://people.interaction-ivrea.it/h.barragan 
     - HC Gilje - http://hcgilje.wordpress.com/resources/rfid_id12_tagreader/
     - Martijn The - http://www.martijnthe.nl/
- Arduino Ethernet Client sketch by bildr.org: http://bildr.org/2011/06/arduino-ethernet-client/
- buzz() function by Rob Faludi: http://www.faludi.com/2007/04/23/buzzer-arduino-example-code/
- random MAC address generator by Joel Chia as used in: https://github.com/j-c/snarc/blob/master/Firmware/snarc/snarc.ino


Hardware Wiring:
- Buzzer according to http://www.budurl.com/buzzer
- RFID-Reader according to...
     - http://www.seeedstudio.com/wiki/Electronic_brick_-_125Khz_RFID_Card_Reader
     - set jumper on the RFID reader to UART mode
     - connect TX of RFID reader to a RX of the Arduino. define SoftSerial accordingly
 */


#include <Ethernet.h>
#include <SPI.h>
#include <SoftwareSerial.h>


////////////////////////////////////////////////////////////////////////
//CONFIGURE RFID READER
////////////////////////////////////////////////////////////////////////
    
    // Configure the Library in UART Mode
    SoftwareSerial mySerial(8, 7); // 7-Rx, 8=Tx


////////////////////////////////////////////////////////////////////////
//CONFIGURE ETHERNET
////////////////////////////////////////////////////////////////////////
    //byte server[] = { 192, 168, 0, 20 }; //ip Address of the server you will connect to
    byte server[] = { 88, 198, 156, 56 };  //fixed IP of the meetmee.javaprovider.net
    
    
  //byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };    //uncomment if hard-coded MAC address is wanted
    byte mac[] = { 0, 0, 0, 0, 0, 0};                        //MAC address will be randomly generated at setup
    
    
    EthernetClient client;
    char inString[32]; // string for incoming serial data
    int stringPos = 0; // string index counter
    boolean startRead = false; // is reading?


////////////////////////////////////////////////////////////////////////
//CONFIGURE API SETTINGS
////////////////////////////////////////////////////////////////////////
    int mainlocation = 99; // set 99 for the Edge
    int sublocation = 19; //set location key according to where the RFID reader is installed, e.g. 1 for Window Bays 1.
//    String thirdpartyid = "9999999"; //actual RFID number. set random number for test purposes...




//FEEDBACK LEDs
// SNARC has two on-board LEDs: green LED = DIGITAL PIN 5, red LED = DIGITAL PIN 6
int onboard_greenPin = 6;
int onboard_redPin = 5;

// alternatively use externally connected LEDs to PIN, external LEDs: green LED = 18, red LED = 17, 
int greenpin = 18;       //flash green LED to confirm successful connection
int redpin = 17;         //flash red LED to indicate that connection failed
int yellowpin = 16;     //no function in this sketch, but can be activated instead of buzzer to confirm that RFID card has been read
int speakerPin = 19;

boolean writingToDB = false;

void setup(){
  
  randomSeed(analogRead(A1));

  pinMode(onboard_greenPin, OUTPUT); //define led pin as output
  pinMode(onboard_redPin, OUTPUT); //define led pin as output
  
  pinMode(yellowpin, OUTPUT); //define led pin as output
  pinMode(greenpin, OUTPUT); //define led pin as output
  pinMode(redpin, OUTPUT); //define led pin as output
  pinMode(speakerPin, OUTPUT);
  
  Serial.begin(9600);
  mySerial.begin(9600);
  
  //Feedback via buzzer and on-board LEDs that Ethernet setup is about to start
    buzz(speakerPin, 1000, 300);
    blinkPin(onboard_greenPin, 500);
    blinkPin(onboard_redPin, 500);
  
  generate_random_mac_address();
  print_mac_address();
  
  Serial.println("Serial...");  
  Serial.println("Ethernet...");
  delay(100);
  int ethernetResult = Ethernet.begin(mac);
  if (ethernetResult == 1) {
    Serial.println("DHCP successful");
    //Confirmation via buzzer and on-board LEDs that Ethernet setup is finished
      blinkPin(onboard_greenPin, 2000);
      buzz(speakerPin, 1000, 300);
      delay(200);
      buzz(speakerPin, 1000, 300);

  } else{
    Serial.println("DHCP not successful :-(");
    //Confirmation via buzzer and on-board LEDs that Ethernet setup is finished
      blinkPin(onboard_redPin, 2000);
      buzz(speakerPin, 1000, 300);
      delay(200);
      buzz(speakerPin, 500, 300);
  }
  
  Serial.println("Setup finished.");
    
    
}

void loop(){
  
/////UART MODE
  byte i = 0;
  byte val = 0;
  byte code[6];
  byte checksum = 0;
  byte bytesread = 0;
  byte tempbyte = 0;
  String rfid = "";
  if(!writingToDB){
    if(mySerial.available() > 0) {
      writingToDB = true;
      if((val = mySerial.read()) == 2) {        // check for header 
      
        bytesread = 0; 
        while (bytesread < 12) {                        // read 10 digit code + 2 digit checksum
          if( mySerial.available() > 0) { 
            val = mySerial.read();
            if((val == 0x0D)||(val == 0x0A)||(val == 0x03)||(val == 0x02)) { // if header or stop bytes before the 10 digit reading 
              break;                                    // stop reading
            }
            
            // Do Ascii/Hex conversion:
            if ((val >= '0') && (val <= '9')) {
              val = val - '0';
            } else if ((val >= 'A') && (val <= 'F')) {
              val = 10 + val - 'A';
            }
  
            // Every two hex-digits, add byte to code:
            if (bytesread & 1 == 1) {
              // make some space for this hex-digit by
              // shifting the previous hex-digit with 4 bits to the left:
              code[bytesread >> 1] = (val | (tempbyte << 4));
  
              if (bytesread >> 1 != 5) {                // If we're at the checksum byte,
                checksum ^= code[bytesread >> 1];       // Calculate the checksum... (XOR)
              };
            } else {
              tempbyte = val;                           // Store the first hex digit first...
            };
  
            bytesread++;                                // ready to read next digit
          } 
        } 
  
        // Output to Serial:
  
        if (bytesread == 12) {                          // if 12 digit read is complete
          //Serial.print("5-byte code: ");
          for (i=0; i<6; i++) {
            if (code[i] < 16){
              //Serial.print("0");
              rfid = rfid + "0";
            }
            //Serial.print(code[i], HEX);
            rfid = rfid + String(code[i], HEX);
          }  
          
          //Serial.println();
          Serial.print("Checksum: ");
          Serial.print(code[5], HEX);
          if (code[5] == checksum){
            Serial.println(" -- passed.");
            
              //blinkPin(yellowpin, 500); // acknowledge that RFID card has been read
              buzz(speakerPin, 800, 500); // acknowledge that RFID card has been read, buzz the buzzer on speakerPin at xxxHz for xxx milliseconds
              
              rfid.toUpperCase();
              Serial.println("RFID: " + rfid);
              String url_base = "/php/Gelatine/blog/API/checkin_submit_manual.php?";
              
              String url_param1 = "mainlocation=" + String(mainlocation);
              String url_param2 = "&sublocation=" + String(sublocation);
              String url_param3 = "&rfid=" + rfid;
              String url_httptail = " HTTP/1.0";
              
              String url_complete = url_base + url_param1 + url_param2 + url_param3 + url_httptail;
              Serial.println("url: " + url_complete);
              

                
            String pageValue = connectAndRead(url_complete); //connect to the server and read the output
            Serial.println(pageValue); //print out the findings.
              
          }
          else
            Serial.println(" -- error.");
          
          Serial.println();
        }
  
        bytesread = 0;
      }
      //delay(2000);
      writingToDB = false;
    }
  }
}

String connectAndRead(String url){
  //connect to the server
  
  Serial.println("connecting...");

  //port 80 is typical of a www page
  if (client.connect(server, 80)) {
    Serial.println("connected");
    client.print("GET ");
    client.println(url);
    client.println();

    buzz(speakerPin, 1000, 200);
    blinkPin(greenpin, 500); //flash green LED to confirm successful connection
    return readPage(); //go and read the output

  }else{
    buzz(speakerPin, 500, 200);    
    blinkPin(redpin, 500); //flash red LED to indicate that connection failed
    return "connection failed";
  }

}

String readPage(){
  //read the page, and capture & return everything between '<' and '>'

  stringPos = 0;
  memset( &inString, 0, 32 ); //clear inString memory

  while(true){

    if (client.available()) {
      char c = client.read();
      Serial.print(c);
      
      //opportunity to store a char[] that occurs in between particular characters
      if (c == '[' ) { //'<' is our begining character
        startRead = true; //Ready to start reading the part 
      }else if(startRead){

        if(c != '+'){ //'>' is our ending character
          inString[stringPos] = c;
          stringPos ++;
        }else{
          //got what we need here! We can disconnect now
          startRead = false;
        }

      }
    
    }
    else{
      client.stop();
      client.flush();
      Serial.println();
      Serial.println("disconnecting.");
      return inString;
    }
  }

}

void blinkPin(int c, int ms){
    digitalWrite(c, HIGH);
    delay(ms);
    digitalWrite(c, LOW);
    //delay(ms);
}

void buzz(int targetPin, long frequency, long length) {
  long delayValue = 1000000/frequency/2; // calculate the delay value between transitions
  //// 1 second's worth of microseconds, divided by the frequency, then split in half since
  //// there are two phases to each cycle
  long numCycles = frequency * length/ 1000; // calculate the number of cycles for proper timing
  //// multiply frequency, which is really cycles per second, by the number of seconds to 
  //// get the total number of cycles to produce
 for (long i=0; i < numCycles; i++){ // for the calculated length of time...
    digitalWrite(targetPin,HIGH); // write the buzzer pin high to push out the diaphram
    delayMicroseconds(delayValue); // wait for the calculated delay value
    digitalWrite(targetPin,LOW); // write the buzzer pin low to pull back the diaphram
    delayMicroseconds(delayValue); // wait againf or the calculated delay value
  }
}


//Methods for generating a random MAC-address

  void generate_random_mac_address()
  {
  	//set_mac_address(random(0, 255), random(0, 255), random(0, 255), random(0, 255), random(0, 255), random(0, 255));
        set_mac_address((random(0, 63) << 2) | 2, random(0, 255), random(0, 255), random(0, 255), random(0, 255), random(0, 255));
  }
  
  void set_mac_address(byte octet0, byte octet1, byte octet2, byte octet3, byte octet4, byte octet5)
  {
  	mac[0] = octet0;
  	mac[1] = octet1;
  	mac[2] = octet2;
  	mac[3] = octet3;
  	mac[4] = octet4;
  	mac[5] = octet5;
      
  }
  
  void print_mac_address()
  {
  	if (mac[0] < 16)
  	{
  		Serial.print('0');
  	}
  	Serial.print(mac[0], HEX);
  	Serial.print(':');
  	if (mac[1] < 16)
  	{
  		Serial.print('0');
  	}
  	Serial.print(mac[1], HEX);
  	Serial.print(':');
  	if (mac[2] < 16)
  	{
  		Serial.print('0');
  	}
  	Serial.print(mac[2], HEX);
  	Serial.print(':');
  	if (mac[3] < 16)
  	{
  		Serial.print('0');
  	}
  	Serial.print(mac[3], HEX);
  	Serial.print(':');
  	if (mac[4] < 16)
  	{
  		Serial.print('0');
  	}
  	Serial.print(mac[4], HEX);
  	Serial.print(':');
  	if (mac[5] < 16)
  	{
  		Serial.print('0');
  	}
  	Serial.println(mac[5], HEX);
  }


