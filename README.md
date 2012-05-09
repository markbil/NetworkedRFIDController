NetworkedRFIDController
=======================

Arduino sketch for a networked RFID controller (using SNARC or Arduino with an Ethernet Shield)

/*
 NetworkedRFIDController Sketch by Mark Bilandzic, 11 April 2012
 
 This sketch reads an RFID card through an RFID reader and makes a GET request to a URL with the RFID numbers as a parameter.
 The sketch is written for the SNARC (Simple NetworkAble RFID Controller), designed by Lawrence "Lemming" Dixon http://www.hsbne.org/projects/SNARC, however
 the code is fully compatible with an Arduino 1.0+ and attached Ethernet-Shield.
 
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