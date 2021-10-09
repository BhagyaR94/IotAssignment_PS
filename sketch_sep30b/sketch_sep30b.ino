#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <ESP8266WiFi.h>

#define SS_PIN D4 //RFID
#define RST_PIN D3 //RFID
#define LED_RED D0
#define LED_GREEN D1
#define trigPin D0 // Trigger Pin of Ultrasonic Sensor
#define echoPin D1 // Echo Pin of Ultrasonic Sensor

MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo myservo;
WiFiClient client;

int pos = 0;
boolean isGateOpened = false;
boolean isVehicleParked = false;
boolean isParkingIn = false;
boolean isLeaving = false;
const char *ssid =  "Test";
const char *pass =  "12345678";

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  connectToWiFi();
  myservo.attach(4);
  myservo.write(0);
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("Approximate your card to the reader...");
  Serial.println();
}

void loop() {
  detectRFID();
  if (isGateOpened == true) {
    pickDistance();
  }
}

void detectRFID() {
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
      Serial.print("Message : ");
      content.toUpperCase();
      if (content.substring(1) == "E3 67 95 34") //change here the UID of the card/cards that you want to give access
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
  lightUpRedLED();
  for (pos = 0; pos <= 120; pos += 5) {
    myservo.write(pos);
    delay(5);
  }
  isGateOpened = true;
  delay(500);
}

void pickDistance() {
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
      closeGate();
    }
  }
  delay(500);
}

void closeGate() {
  for (pos = 120; pos >= 0; pos -= 5) {
    myservo.write(pos);
    delay(5);
  }
  isGateOpened = false;
}

void lightUpRedLED() {
  pinMode(LED_RED, OUTPUT);
  digitalWrite(LED_RED, HIGH);
}

void lightDownRedLED() {
  pinMode(LED_RED, OUTPUT);
  digitalWrite(LED_RED, LOW);
}

long microsecondsToInches(long microseconds) {
  return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds) {
  return microseconds / 29 / 2;
}

void connectToWiFi() {
  delay(10);

  Serial.println("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}
