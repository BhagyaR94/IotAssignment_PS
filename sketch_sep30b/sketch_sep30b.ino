#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define SS_PIN D4 //RFID
#define RST_PIN D3 //RFID
#define LED_RED D0
#define LED_GREEN D1
#define trigPin D0 // Trigger Pin of Ultrasonic Sensor
#define echoPin D1 // Echo Pin of Ultrasonic Sensor

long lastReconnectAttempt = 0;
MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo myservo;
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

int pos = 0;
boolean isGateOpened = false;
boolean isVehicleParked = false;
boolean isParkingIn = false;
boolean isLeaving = false;

const char *ssid =  "Test";
const char *password =  "12345678";
const char* mqtt_server = "broker.mqtt-dashboard.com";
String authenticatedRFID = "";

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  authenticatedRFID = "";
  for (int i = 0; i < length; i++) {
    authenticatedRFID += ((char)payload[i]);
  }
  Serial.println(authenticatedRFID);

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("gh/kmccout", "hello world");
      client.subscribe("IT5070/topic1");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  myservo.attach(4);
  myservo.write(0);
  Serial.println("Approximate your card to the reader...");
  Serial.println();
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  detectRFID();
  if (isGateOpened == true) {
    pickDistance();
  }

}

void detectRFID() {
  client.loop();
  if (mfrc522.PICC_IsNewCardPresent()) {
    if (mfrc522.PICC_ReadCardSerial()) {
      Serial.print("UID tag :");
      String content = "";
      byte letter;
      for (byte i = 0; i < mfrc522.uid.size; i++)
      {
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], HEX);
        content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
        content.concat(String(mfrc522.uid.uidByte[i], HEX));
      }
      Serial.println();
      content.toUpperCase();
      if (content.substring(1) == authenticatedRFID) //change here the UID of the card/cards that you want to give access
      {
        Serial.println("Authorized access");
        Serial.println();
        delay(1000);
        mfrc522.PICC_HaltA();
        isRFIDAuthenticated();
      }
    }
  }

}

void isRFIDAuthenticated() {
  client.loop();
  if (isGateOpened == false) {
    if (isVehicleParked == true) {
      isLeaving = true;
      openGate();
    } else {
      isParkingIn = true;
      openGate();
    }
  }
}

void openGate() {
  client.loop();
  lightUpRedLED();
  for (pos = 0; pos <= 120; pos += 5) {
    myservo.write(pos);
    delay(5);
  }
  isGateOpened = true;
  delay(500);
}

void pickDistance() {
  client.loop();
  long duration, distance;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration / 2) / 29.1;
  if (distance <= 5) {
    if (isGateOpened == true) {
      Serial.println(distance);
      lightDownRedLED();
      delay(1000);
      closeGate(true);
    }
  }
  delay(500);
}

void closeGate(boolean vehicleParked) {
  for (pos = 120; pos >= 0; pos -= 5) {
    myservo.write(pos);
    delay(5);
  }
  isGateOpened = false;
  isVehicleParked = vehicleParked;
}

void lightUpRedLED() {
  client.loop();
  pinMode(LED_RED, OUTPUT);
  digitalWrite(LED_RED, HIGH);
}

void lightDownRedLED() {
  client.loop();
  pinMode(LED_RED, OUTPUT);
  digitalWrite(LED_RED, LOW);
}

long microsecondsToInches(long microseconds) {
  return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds) {
  return microseconds / 29 / 2;
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
