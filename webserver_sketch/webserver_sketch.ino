/*--------------------------------------------------------------
  Program:      Arduino Powered Home Automation

  Description:  Arduino web server that displays and controls 4 outputs,
                using buttons. The web page is stored on the micro SD card.

  Hardware:     Arduino Uno and official Arduino Ethernet
                shield. Should work with other Arduinos and
                compatible Ethernet shields.
                2Gb micro SD card formatted FAT16.
                Pins 6 to 9 as outputs to 4 channel relay circuit.

  Software:     Developed using Arduino 1.0.5 software
                Should be compatible with Arduino 1.0 +
                SD card contains web page called index.htm

  References:   - WebServer example by David A. Mellis and
                  modified by Tom Igoe
                - SD card examples by David A. Mellis and
                  Tom Igoe
                - Ethernet library documentation:
                  http://arduino.cc/en/Reference/Ethernet
                - SD Card library documentation:
                  http://arduino.cc/en/Reference/SD

  Date:         4 April 2013
  Modified:     19 June 2013
                - removed use of the String class

                04 May  2014
                - analog output removed
                - switch removed
                - description update
                28 March 2015
                - Temperature sensor added
                - Redesigned website
                - Minor bug fixes

  Author:       W.A. Smith, http://startingelectronics.com
  --------------------------------------------------------------*/

#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
#include <Thermistor.h>

// size of buffer used to capture HTTP requests
#define REQ_BUF_SZ   60

Thermistor temp(2);

// MAC address from Ethernet shield sticker under board
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// IP address, may need to change depending on network
IPAddress ip(192, 168, 0, 120);
// create a server at port 80
EthernetServer server(80);
// the web page file on the SD card
File webFile;
// buffered HTTP request stored as null terminated string
char HTTP_req[REQ_BUF_SZ] = {0};
// index into HTTP_req buffer
char req_index = 0;
// stores the states of the RELAYs
boolean RELAY_state[4] = {0};

void setup() {
    // disable Ethernet chip
    pinMode(10, OUTPUT);
    digitalWrite(10, HIGH);

    // Switches
    pinMode(5, OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(7, OUTPUT);
    pinMode(8, OUTPUT);
    pinMode(9, OUTPUT);

    Ethernet.begin(mac, ip);  // initialize Ethernet device
    server.begin();           // start to listen for clients
}

void loop() {
    EthernetClient client = server.available();  // try to get client

    if (client) {  // got client?
        boolean currentLineIsBlank = true;
        while (client.connected()) {
            if (client.available()) {   // client data available to read
                char c = client.read(); // read 1 byte (character) from client
                // limit the size of the stored received HTTP request
                // buffer first part of HTTP request in HTTP_req array (string)
                // leave last element in array as 0 to null terminate string (REQ_BUF_SZ - 1)

                if (req_index < (REQ_BUF_SZ - 1)) {
                    HTTP_req[req_index] = c;          // save HTTP request character
                    req_index++;
                }
                // last line of client request is blank and ends with \n
                // respond to client only after last line received

                if (c == '\n' && currentLineIsBlank) {
                    // send a standard http response header
                    client.println("HTTP/1.1 200 OK");
                    // remainder of header follows below, depending on if
                    // web page or XML page is requested
                    // Ajax request - send XML file

                    if (StrContains(HTTP_req, "buttons")) {
                        // send rest of HTTP header
                        client.println("Content-Type: text/xml");
                        client.println("Connection: keep-alive");
                        client.println();
                        SetRELAYs();
                        // send XML file containing input states
                        XML_response(client);
                    }

                    else {  // web page request
                        // send rest of HTTP header
                        client.println("Content-Type: text/html");
                        client.println("Connection: keep-alive");
                        client.println();
                        // send web page
                        webFile = SD.open("index.htm");        // open web page file

                        if (webFile) {
                            while(webFile.available()) {
                                client.write(webFile.read()); // send web page to client
                            }
                            webFile.close();
                        }
                    }
                    // reset buffer index and all buffer elements to 0
                    req_index = 0;
                    StrClear(HTTP_req, REQ_BUF_SZ);
                    break;
                }
                // every line of text received from the client ends with \r\n

                if (c == '\n') {
                    // last character on line of received text
                    // starting new line with next character read
                    currentLineIsBlank = true;
                }
                else if (c != '\r') {
                    // a text character was received from client
                    currentLineIsBlank = false;
                }
            } // end if (client.available())
        } // end while (client.connected())
        delay(1);      // give the web browser time to receive the data
        client.stop(); // close the connection
    } // end if (client)
}

// checks if received HTTP request is switching on/off RELAYs
// also saves the state of the RELAYs
void SetRELAYs(void) {
    // Living Room (pin 5)
    if (StrContains(HTTP_req, "RELAY1=1")) {
        RELAY_state[0] = 1;         // save Switch 1 state to On
        digitalWrite(5, HIGH);
    }
    else if (StrContains(HTTP_req, "RELAY1=0")) {
        RELAY_state[0] = 0;     // save Switch 1 state to OFF
        digitalWrite(5, LOW);
    }

    // Master Bed (pin 6)
    if (StrContains(HTTP_req, "RELAY2=1")) {
        RELAY_state[1] = 1;         // save Switch 2 state to On
        digitalWrite(6, HIGH);
    }
    else if (StrContains(HTTP_req, "RELAY2=0")) {
        RELAY_state[1] = 0;     // save Switch 2 state to Off
        digitalWrite(6, LOW);
    }

    // Guest Room (pin 9)
    if (StrContains(HTTP_req, "RELAY3=1")) {
        RELAY_state[2] = 1;         // save Switch 3 state to On
        digitalWrite(9, HIGH);
    }
    else if (StrContains(HTTP_req, "RELAY3=0")) {
        RELAY_state[2] = 0;     // save Switch 3 state to Off
        digitalWrite(9, LOW);
    }

    // Kitchen (pin 7)
    if (StrContains(HTTP_req, "RELAY4=1")) {
        RELAY_state[3] = 1;         // save Switch 4 state to On
        digitalWrite(8, HIGH);
    }
    else if (StrContains(HTTP_req, "RELAY4=0")) {
        RELAY_state[3] = 0;     // save Switch 4 state to Off
        digitalWrite(8, LOW);
    }

    // Wash Room (pin 9)
    if (StrContains(HTTP_req, "RELAY5=1")) {
        RELAY_state[4] = 1;         // save Switch 5 state to On
        digitalWrite(7, HIGH);
    }
    else if (StrContains(HTTP_req, "RELAY5=0")) {
        RELAY_state[4] = 0;     // save Switch 5 state to Off
        digitalWrite(7, LOW);
    }
}

// send the XML file with Temperature and Switch status
void XML_response(EthernetClient cl) {
    byte celsius = temp.getTemp();

    cl.print("<?xml version = \"1.0\" ?>");
    cl.print("<inputs>");

        cl.print("<temp>");
        cl.print(celsius);
        cl.print("</temp>");

        // Switch1
        cl.print("<RELAY>");
        if (RELAY_state[0]) {
            cl.print("on");
        }
        else {
            cl.print("off");
        }
        cl.println("</RELAY>");

        // Switch2
        cl.print("<RELAY>");
        if (RELAY_state[1]) {
            cl.print("on");
        }
        else {
           cl.print("off");
        }
        cl.println("</RELAY>");

        // Switch3
        cl.print("<RELAY>");
        if (RELAY_state[2]) {
            cl.print("on");
        }
        else {
           cl.print("off");
        }
        cl.println("</RELAY>");

        // Switch4
        cl.print("<RELAY>");
        if (RELAY_state[3]) {
            cl.print("on");
        }
        else {
           cl.print("off");
        }
        cl.println("</RELAY>");

        // Switch5
        cl.print("<RELAY>");
        if (RELAY_state[4]) {
            cl.print("on");
        }
        else {
            cl.print("off");
        }
        cl.println("</RELAY>");

    cl.print("</inputs>");
}

// sets every element of str to 0 (clears array)
void StrClear(char *str, char length) {
    for (int i = 0; i < length; i++) {
        str[i] = 0;
    }
}

// searches for the string sfind in the string str
// returns 1 if string found
// returns 0 if string not found
char StrContains(char *str, char *sfind) {
    char found = 0;
    char index = 0;
    char len;

    len = strlen(str);

    if (strlen(sfind) > len) {
        return 0;
    }
    while (index < len) {
        if (str[index] == sfind[found]) {
            found++;

            if (strlen(sfind) == found) {
                return 1;
            }
        }
        else {
            found = 0;
        }
        index++;
    }

    return 0;
}
