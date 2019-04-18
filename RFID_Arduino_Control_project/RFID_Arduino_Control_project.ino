#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>

#define SS_PIN 10
#define RST_PIN 9
#define RELAY0 7
#define BUZZER 8
int greenLedPin = 5;
int redLedPin = 6;
MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

byte* data;
String a = "";
String b = "";
String c = "";
String d = "";
String content;
String recv = "";
String dataAppend;

const byte numChars = 32;
char receivedChars[numChars];   // an array to store the received data
String recvString;
boolean newData = false;
boolean recvState = false;
static boolean recvInProgress = false;
unsigned long initialTime = 0;
unsigned long finalTime = 10000;

void writeString(String stringData) {
        for (int i = 0; i < stringData.length(); i++) {
                Serial.write(stringData[i]);
        }
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
        writeString(" "+dataAppend);
        recvState = true;
        delay(500);
}

void setup() {
        // put your setup code here, to run once:
        SPI.begin();
        mfrc522.PCD_Init();
        Serial.begin(115200);
        lcd.begin(16, 4);
        pinMode(RELAY0, OUTPUT);
        pinMode(greenLedPin, OUTPUT);
        pinMode(redLedPin, OUTPUT);
        pinMode(BUZZER, OUTPUT);
        Serial.write(" <Arduino is ready>");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("  RFID RIM LOCK ");
        lcd.setCursor(0, 1);
        lcd.print("   PLEASE INSERT   ");
        lcd.setCursor(4, 2);
        lcd.print("     YOUR CARD     ");
        digitalWrite(greenLedPin, LOW);
        digitalWrite(RELAY0, LOW);
        digitalWrite(redLedPin, LOW);
        
         
}

void loop() {
        // put your main code here, to run repeatedly:  
  initialTime = millis();     
  rfidData();
  while (Serial.available() > 0 ) {  
    recvWithStartEndMarker();
    dataResponse();
    if (recvString == "GRANTED") {
        initialTime = millis();
        digitalWrite(greenLedPin, HIGH);
        digitalWrite(RELAY0, HIGH);
        lcd.setCursor(0, 1);
        lcd.print("CARD NO: " + dataAppend);
        lcd.setCursor(4, 2);
        lcd.print("ACCESS GRANTED ");
        delay(1000);
        digitalWrite(RELAY0, LOW);
        delay(1000);
        digitalWrite(greenLedPin, LOW);
     }
    else if (recvString == "DENIED") {
      lcd.setCursor(0, 1);
      lcd.print("CARD NO: " + dataAppend);
      lcd.setCursor(4, 2);
      lcd.print("ACCESS DENIED  ");
      for (int j = 0; j < 5; j++) {
        digitalWrite(redLedPin, HIGH);
        digitalWrite(BUZZER, HIGH);
        delay(500);
        digitalWrite(redLedPin, LOW);
        digitalWrite(BUZZER, LOW);
        delay(500);
        }
      }
     }
    if ((millis()-initialTime)>finalTime) {
          lcd.setCursor(0, 1);
          lcd.print("   PLEASE INSERT   ");
          lcd.setCursor(4, 2);
          lcd.print("     YOUR CARD     ");
    }
   
}




void recvWithStartEndMarker() {
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;
   
    while (Serial.available() > 0) {
        rc = Serial.read();

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







