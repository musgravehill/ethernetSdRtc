/********************************************************
 * 
 * This example shows how to connect a simple web server read that shows 
 * the value of the analog input pins. And then write the data to and read 
 * from an SD card file--"test.txt".
 * 
 * The W5200 circuit:
 * W5200 on Ethernet Shield V2.0 by seeed attached to SPI bus as follows:
 ** ----------------
 **| W5200 - Arduino|
 **| MOSI  - MOSI   |
 **| MISO  - MISO   |
 **| CLK   - SCK    |
 **| CS    - pin 10 |
 ** ----------------
 * 
 * The other Circuit:
 * SD card attached to SPI bus as follows:
 ** ----------------
 **| SD    - Arduino|
 **| MOSI  - MOSI   |
 **| MISO  - MISO   |
 **| CLK   - SCK    |
 **| CS    - pin 4  |
 ** ----------------
 * 
 * Analog inputs attached to pins A0 through A5 (optional).
 * 
 * Author: Frankie.Chu at Seeed Studio.
 * Date:   2012-11-20
 * 
 *******************************************************/

/*
digitalWrite(W5200_CS, HIGH); //W5200 sleep
 digitalWrite(W5200_CS, LOW); //W5200 ready
 
 digitalWrite(SDCARD_CS, HIGH); //SD sleep
 digitalWrite(SDCARD_CS, LOW); //SD ready
 
 delay(50);
 */

#include <SPI.h>
#include <EthernetV2_0.h>
#include <Wire.h>
#include <RTClib.h>
#include <SD.h>

RTC_DS1307 RTC;

#define W5200_CS  10
#define SDCARD_CS 4
// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,0,111);

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer server(80);

void setup()
{
  //Serial.begin(9600);  
  pinMode(W5200_CS, OUTPUT);      
  pinMode(SDCARD_CS,OUTPUT);

  digitalWrite(W5200_CS, HIGH); //W5200 sleep
  delay(50);
  SD.begin(SDCARD_CS); //it bang d10 and d4?   
  delay(50);  
  digitalWrite(SDCARD_CS, HIGH); //SD sleep
  delay(50);
  Ethernet.begin(mac, ip); //it bang d10 and d4?
  server.begin();  

  delay(50);
  //Wire.begin();
  RTC.begin();
  //RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
  delay(100);
}



void loop()
{   
  //listen for incoming clients
  EthernetClient client = server.available();
  if (client) {      
    boolean isCLientRequestEnded = false; // an http request ends with a blank line  
    char clientRequestCharPrev = ' ';
    char clientRequestCharCurr = ' ';
    String clientRequestUriRaw = "";
    uint8_t clientRequestUriRawSize = 0;
    bool clientRequestUriRawGrab = true;
    while (client.connected()) {
      if (client.available()) {
        char clientRequestChar = client.read();  

        //first "\r" in "GET /images/bob.jpg HTTP/1.1\r\nHost: 192..." OR size is over 100 chars
        if( (clientRequestChar == '\r') || (clientRequestUriRawSize > 100) ){   
          clientRequestUriRawGrab = false;
        }        
        if(clientRequestUriRawGrab){          
          clientRequestUriRaw += clientRequestChar;   
          clientRequestUriRawSize++;       
        }        

        /*
        Serial.print(clientRequestChar);
         if(clientRequestChar == '\n'){
         Serial.print('N');
         }
         if(clientRequestChar == '\r'){
         Serial.print('R');
         } */


        clientRequestCharPrev = clientRequestCharCurr;
        clientRequestCharCurr = clientRequestChar;
        if(clientRequestCharPrev == '\n' && clientRequestCharCurr == '\r'){
          isCLientRequestEnded = true;
        }
        else{
          isCLientRequestEnded = false;
        }

        if(isCLientRequestEnded){
          //Serial.println("isCLientRequestEnded = TRUE");  
          //Serial.println(clientRequestUriRaw);          
          processRequest(clientRequestUriRaw, client);

          delay(1); // give the web browser time to receive the data       
          client.stop(); // close the connection            
        }
      }      
    }    

    //int freeRam = FreeRam();
    //String freeRamInfo = "freeRAM: 2048 - ";
    //freeRamInfo += freeRam;
    //freeRamInfo += " bytes";
    //Serial.println(freeRamInfo);     
  }   //endif client

}//loop

void processRequest(String clientRequestUriRaw, EthernetClient client){     
  File currFile;
  String clientRequestUri = " ";
  clientRequestUri = clientRequestUriRaw; //GET /images/bob.jpg HTTP/1.1
  clientRequestUri.replace("GET /", "");
  clientRequestUri.replace(" HTTP/1.1", "");
  clientRequestUri.trim();
  uint8_t uriSize = clientRequestUri.length();
  String fileExt = "";
  for(int i=0; i<4; i++){
    uint8_t pos = uriSize -4 +i;
    fileExt += clientRequestUri[pos];
  } 

  if(fileExt == ".jpg"){
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: image/jpeg");
    client.println("Connnection: close");
    client.println();     //!!!! end of http headers
  }  
  else if(fileExt == ".htm"){  //see "the 8.3 convention"!!!  fileName[8].fileExt[3] 
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connnection: close");
    client.println();     //!!!! end of http headers
  }
  else if(fileExt == ".txt"){
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("Connnection: close");
    client.println();     //!!!! end of http headers
  }
  else{
    client.println("HTTP/1.1 404 Not Found");    
    client.println("Connnection: close");
    client.println();     //!!!! end of http headers
    return; //EXIT!!
  }

  //string to char[]
  char filename[clientRequestUri.length()+1];
  clientRequestUri.toCharArray(filename, sizeof(filename));

  currFile = SD.open(filename, FILE_READ);
  if (currFile) { 
    char buff[65];
    while (currFile.available()) {
      int buffSize = currFile.read(buff,64);
      client.write((byte*)buff,buffSize);	  	  
    }      
    currFile.close();
  } 

  //Serial.print("filename=");
  //Serial.print(filename);  
  //Serial.print("\r\n");

  //Serial.print("uriSize=");
  //Serial.print(uriSize);
  //Serial.print("\r\n");

  //Serial.print("fileExt=");
  //Serial.print(fileExt);  
  //Serial.print("\r\n");

  delay(10);
  logRequest(clientRequestUri);  
}


void logRequest(String clientRequestUri){ 
  DateTime now = RTC.now();
  //Serial.println(now.day(), DEC);

  File logFile; //log.txt
  logFile = SD.open("LOG.TXT", FILE_WRITE);  
  if (logFile) {  
    int randNumber = random(0, 9999);  
    char line[50];    
    //Is a C++ String, not a character array like sprintf() is expecting.
    // To convert this string to a character array such that sprintf is expecting, you must use .c_str() in your sprintf
    sprintf(line, "%s %02d:%02d:%02d %02d-%02d-%04d", 
    clientRequestUri.c_str(), 
    now.hour(), now.minute(), now.second(),
    now.day(),now.month(), now.year()    
      );
    logFile.println(line);  
    //Serial.println(line);  
    logFile.close();  
  }
}
/*
int freeRam() {
 extern int __heap_start,*__brkval;
 int v;
 return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int) __brkval);  
 }
 */



