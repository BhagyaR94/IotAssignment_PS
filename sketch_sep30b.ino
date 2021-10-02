#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

#define SS_PIN 10 //RFID
#define RST_PIN 9 //RFID
MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo myservo;

double pos = 0;
boolean isGateOpened = false;
boolean isVehicleParked = false;

boolean isParkingIn = false;
boolean isLeaving = false;

const int pingPin = 7; // Trigger Pin of Ultrasonic Sensor
const int echoPin = 6; // Echo Pin of Ultrasonic Sensor
const int LED_RED = 4;

void setup() {
  Serial.begin(9600);
  myservo.attach(8);
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();   // Initiate MFRC522
  Serial.println("Approximate your card to the reader...");
  Serial.println();
}

void loop() {
  detectRFID();
  pickDistance();
}

void detectRFID() {
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial())
  {
    return;
  }
  //Show UID on serial monitor
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
    isRFIDAuthenticated();
  }

  else   {
    Serial.println(" Access denied");
    delay(3000);
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
  pinMode(8, HIGH);
  for (pos = 0; pos <= 90; pos += 0.5) {
    myservo.write(pos);
    delay(5);
  }
  isGateOpened = true;
}

void pickDistance() {
  long duration, inches, cm;
  pinMode(pingPin, OUTPUT);
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(pingPin, LOW);
  pinMode(echoPin, INPUT);
  duration = pulseIn(echoPin, HIGH);
  inches = microsecondsToInches(duration);
  delay(100);
  if (inches < 3) {
    if (isGateOpened == true) {
      lightDownRedLED();
      delay(1000);
      closeGate();
    }

  }
}

void closeGate() {
  for (pos = 90; pos >= 0; pos -= 0.5) {
    myservo.write(pos);
    delay(5);
  }
  pinMode(8, LOW);
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
