#include <ESP8266WiFi.h>

WiFiClient client;

int ledPin = 2;
String data;
boolean connectState = false;

const int port = 8888;  //Port number
const IPAddress server(192, 168, 1, 141);

//Server connect to WiFi Network
const char *ssid = "MW40VD_19E7";  //Enter your wifi SSID
const char *password = "Cazp2x6U7t3A9jXg";  //Enter your wifi Password


void setup() 
{
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  Serial.println();

  WiFi.begin(ssid, password); //Connect to wifi
 
  // Wait for connection  
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
    digitalWrite(ledPin, HIGH);
  }
  else {
    Serial.println("No Connection");
    delay(500);
    return;
  }
   while(client.connected()){      
      while(client.available()>0){
        // read data from the connected client
        Serial.write(client.read()); 
      }
      //Send Data to connected client
      while(Serial.available()>0)
      {
         client.write(Serial.read());
      }
     }
    client.stop();
    Serial.println("Client disconnected");   
    digitalWrite(ledPin, LOW); 
  
}

