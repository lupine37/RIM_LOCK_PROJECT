
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

#define SS_PIN 10
#define RST_PIN 9
#define RELAY0 7
#define BUZZER 8
int greenLedPin = 5;
int redLedPin = 6;
int doorSensor = 4;
MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
SoftwareSerial mySerial(2, 3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

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
boolean enrollState = false;
static boolean recvInProgress = false;
unsigned long initialTime = 0;
unsigned long finalTime = 10000;
uint8_t id;

void writeString(String stringData) {
        for (int i = 0; i < stringData.length(); i++) {
                Serial.write(stringData[i]);
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
        initialTime = millis();
        delay(500);
}

uint8_t getFingerprintEnroll() {

        int p = -1;
        Serial.print(" Waiting for valid finger to enroll as #"); Serial.println(id);
        while (p != FINGERPRINT_OK) {
                p = finger.getImage();
                switch (p) {
                case FINGERPRINT_OK:
                        Serial.println(" Image taken");
                        break;
                case FINGERPRINT_NOFINGER:
                        Serial.println(" .");
                        break;
                case FINGERPRINT_PACKETRECIEVEERR:
                        Serial.println(" Communication error");
                        break;
                case FINGERPRINT_IMAGEFAIL:
                        Serial.println(" Imaging error");
                        break;
                default:
                        Serial.println(" Unknown error");
                        break;
                }
        }

        // OK success!

        p = finger.image2Tz(1);
        switch (p) {
        case FINGERPRINT_OK:
                Serial.println(" Image converted");
                break;
        case FINGERPRINT_IMAGEMESS:
                Serial.println(" Image too messy");
                return p;
        case FINGERPRINT_PACKETRECIEVEERR:
                Serial.println(" Communication error");
                return p;
        case FINGERPRINT_FEATUREFAIL:
                Serial.println(" Could not find fingerprint features");
                return p;
        case FINGERPRINT_INVALIDIMAGE:
                Serial.println(" Could not find fingerprint features");
                return p;
        default:
                Serial.println(" Unknown error");
                return p;
        }

        Serial.println(" Remove finger");
        delay(2000);
        p = 0;
        while (p != FINGERPRINT_NOFINGER) {
                p = finger.getImage();
        }
        Serial.print(" ID "); Serial.println(id);
        p = -1;
        Serial.println(" Place same finger again");
        while (p != FINGERPRINT_OK) {
                p = finger.getImage();
                switch (p) {
                case FINGERPRINT_OK:
                        Serial.println(" Image taken");
                        break;
                case FINGERPRINT_NOFINGER:
                        Serial.print(" .");
                        break;
                case FINGERPRINT_PACKETRECIEVEERR:
                        Serial.println(" Communication error");
                        break;
                case FINGERPRINT_IMAGEFAIL:
                        Serial.println(" Imaging error");
                        break;
                default:
                        Serial.println(" Unknown error");
                        break;
                }
        }

        // OK success!

        p = finger.image2Tz(2);
        switch (p) {
        case FINGERPRINT_OK:
                Serial.println(" Image converted");
                break;
        case FINGERPRINT_IMAGEMESS:
                Serial.println(" Image too messy");
                return p;
        case FINGERPRINT_PACKETRECIEVEERR:
                Serial.println(" Communication error");
                return p;
        case FINGERPRINT_FEATUREFAIL:
                Serial.println(" Could not find fingerprint features");
                return p;
        case FINGERPRINT_INVALIDIMAGE:
                Serial.println(" Could not find fingerprint features");
                return p;
        default:
                Serial.println(" Unknown error");
                return p;
        }

        // OK converted!
        Serial.print(" Creating model for #");  Serial.println(id);

        p = finger.createModel();
        if (p == FINGERPRINT_OK) {
                Serial.println(" Prints matched!");
        } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
                Serial.println(" Communication error");
                return p;
        } else if (p == FINGERPRINT_ENROLLMISMATCH) {
                writeString(" no_match");
                initialTime = millis();
                return p;
        } else {
                Serial.println(" Unknown error");
                return p;
        }

        Serial.print(" ID "); Serial.println(id);
        p = finger.storeModel(id);
        if (p == FINGERPRINT_OK) {
                writeString(" Stored!");
        } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
                Serial.println(" Communication error");
                return p;
        } else if (p == FINGERPRINT_BADLOCATION) {
                Serial.println(" Could not store in that location");
                return p;
        } else if (p == FINGERPRINT_FLASHERR) {
                Serial.println(" Error writing to flash");
                return p;
        } else {
                Serial.println(" Unknown error");
                return p;
        }
}

int getFingerprintID() {
        uint8_t p = finger.getImage();
        if (p != FINGERPRINT_OK) return -1;

        p = finger.image2Tz();
        if (p != FINGERPRINT_OK) return -1;

        p = finger.fingerFastSearch();
        if (p != FINGERPRINT_OK) return -1;

        String fingerprintID = String(finger.fingerID);
        writeString(" #" + fingerprintID);
        initialTime = millis();
}

uint8_t readnumber(void){
        uint8_t num = 0;

        while (num == 0) {
                while (!Serial.available()) ;
                num = Serial.parseInt();
        }
        return num;
}


void setup() {
        // put your setup code here, to run once:
        SPI.begin();
        mfrc522.PCD_Init();
        Serial.begin(115200);
        finger.begin(57600);
        lcd.begin(16, 4);
        pinMode(RELAY0, OUTPUT);
        pinMode(greenLedPin, OUTPUT);
        pinMode(redLedPin, OUTPUT);
        pinMode(BUZZER, OUTPUT);
        pinMode(doorSensor, INPUT);
        Serial.write(" <Arduino is ready>");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("   RFID RIM LOCK  ");
        lcd.setCursor(0, 1);
        lcd.print("   PLEASE INSERT   ");
        lcd.setCursor(4, 2);
        lcd.print("     YOUR CARD     ");
        digitalWrite(greenLedPin, LOW);
        digitalWrite(RELAY0, LOW);
        digitalWrite(redLedPin, LOW);
        initialTime = millis();

}

void loop() {
        // put your main code here, to run repeatedly:
        int doorSensorState = digitalRead(doorSensor);
        if (doorSensorState == 1) {
                lcd.setCursor(0, 1);
                lcd.print("    PLEASE CLOSE    ");
                lcd.setCursor(4, 2);
                lcd.print("      THE DOOR      ");
                digitalWrite(greenLedPin, HIGH);
                delay(200);
                digitalWrite(greenLedPin, LOW);
                delay(200);
        }
        else if ((millis()-initialTime)>finalTime) {
                lcd.setCursor(0, 1);
                lcd.print("   PLEASE INSERT   ");
                lcd.setCursor(4, 2);
                lcd.print("     YOUR CARD     ");
                initialTime = millis();
        }
        else {
                rfidData();
                while (Serial.available() > 0) {
                        recvWithStartEndMarker();
                        dataResponse();
                        if (recvString == "Enroll") {
                                initialTime = millis();
                                writeString(" idNo");
                                id = readnumber();
                                if (id == 0) { // ID #0 not allowed, try again!
                                        return;
                                }
                                while (!getFingerprintEnroll() ) ;
                        }
                        if (recvString == "ID") {
                                recvState = true;
                                lcd.setCursor(0, 1);
                                lcd.print("   PLEASE PLACE   ");
                                lcd.setCursor(4, 2);
                                lcd.print("    YOUR FINGER    ");
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
                        while (recvState == true) {
                                getFingerprintID();
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
                                        recvState = false;
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
                                        recvState = false;
                                }
                                else if ((millis() -initialTime)>finalTime) {
                                        writeString(" break");
                                        recvState = false;
                                        lcd.setCursor(0, 1);
                                        lcd.print("   PLEASE INSERT   ");
                                        lcd.setCursor(4, 2);
                                        lcd.print("     YOUR CARD     ");
                                        initialTime = millis();
                                }

                        }
                }
        }

}
