#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Ticker.h>

#define SS_PIN 15
#define RST_PIN 5
int greenLedPin = 4;
int redLedPin = 2;
boolean connectState = false;
const int port = 8888;  //Port number
const IPAddress server(192, 168, 1, 170);

MFRC522 mfrc522(SS_PIN, RST_PIN);
WiFiClient client;
Ticker watchDogTick;

const char *ssid = "MW40VD_19E7";
const char *password = "Cazp2x6U7t3A9jXg";

byte* data;
String a = "";
String b = "";
String c = "";
String d = "";
String content;
String recv = "";
String dataAppend;
const byte numChars = 32;
char receivedChars[numChars];
String recvString;
boolean newData = false;
static boolean recvInProgress = false;
volatile int watchDogCount = 0;

void yield(void);

void ISRWatchDog(){
  watchDogCount++;
  Serial.println(watchDogCount);
  if (watchDogCount == 100) {
    watchDogCount = 0;
  }
}

void writeString(String stringData) {
        for (int i = 0; i < stringData.length(); i++) {
                client.write(stringData[i]);
        }
}


void recvWithStartEndMarker() {
        static byte ndx = 0;
        char startMarker = '<';
        char endMarker = '>';
        char rc;

        while (client.available() > 0) {
                rc = client.read();

                if (recvInProgress == true) {
                        if (rc != endMarker) {
                                receivedChars[ndx] = rc;
                                ndx++;
                                if (ndx >= numChars) {
                                        ndx = numChars - 1;
                                }
                        }
                        else {
                                receivedChars[ndx] = '\0'; // terminate the string
                                ndx = 0;
                                newData = true;
                        }
                }
                else if (rc == startMarker) {
                        recvInProgress = true;
                }
        }
}



void dataResponse() {
        if (newData == true) {
                recvString = String(receivedChars);
        }

        newData = false;
        recvInProgress = false;
}


void rfidData() {
   if (!mfrc522.PICC_IsNewCardPresent()) {
                return;
        }
   if (!mfrc522.PICC_ReadCardSerial()) {
                return;
        }
        data = mfrc522.uid.uidByte, mfrc522.uid.size;
        a = String(data[0], HEX);
        b = String(data[1], HEX);
        c = String(data[2], HEX);
        d = String(data[3], HEX);
        dataAppend = String(a) + String(c) + String(c) + String(d);
        Serial.println(dataAppend);
        writeString(" "+dataAppend);
        delay(500);
        
}


void setup() {
  SPI.begin();
  Serial.begin(115200);
  watchDogTick.attach(1, ISRWatchDog);
  pinMode(greenLedPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);
  while (!Serial);
  SPI.begin();
  mfrc522.PCD_Init();
  delay(4);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to Wifi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); 
  
}

void loop() {
  if (client.connect(server, port)) {
  Serial.println("Client Connected");
  }
  else {
    Serial.println("No Connection");
    delay(500);
    return;
  }
   while(client.connected()){    
        rfidData();
        if (client.connected() > 0) {
          recvWithStartEndMarker();
          dataResponse();
        if (recvString == "GRANTED") {
            digitalWrite(greenLedPin, HIGH);
            delay(2000);
            digitalWrite(greenLedPin, LOW);
            recvString = "";
          }
        else if (recvString == "DENIED") {
            for (int j = 0; j < 5; j++) {
            digitalWrite(redLedPin, HIGH);
            delay(500);
            digitalWrite(redLedPin, LOW);
            delay(500);
            recvString = "";
           }
          
        }
     }
   }
    client.stop();
    Serial.println("Client disconnected");   
  
}
