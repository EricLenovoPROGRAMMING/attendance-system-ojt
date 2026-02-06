 /*
 * Initial Author: ryand1011 (https://github.com/ryand1011)
 *
 * Reads data written by a program such as "rfid_write_personal_data.ino"
 *      
 * See: https://github.com/miguelbalboa/rfid/tree/master/examples/rfid_write_personal_data
 *
 * Uses MIFARE RFID card using RFID-RC522 reader
 * Uses MFRC522 - Library
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 *
 * More pin layouts for other boards can be found here: https://github.com/miguelbalboa/rfid#pin-layout
*/


#include <SPI.h>
#include <MFRC522.h>


#define RST_PIN 9  // Configurable, see typical pin layout above
#define SS_PIN 10  // Configurable, see typical pin layout above

#define ACTIVITY_LED 6
#define GREEN_LED 3
#define RED_LED 4
#define BUZZ 5
#define REFRESH_DATABASE 7


using namespace std;

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance


byte authorizedUIDs[][4] = {
  { 0xAD, 0xCA, 0x33, 0x12 },  // Example UID 1
};

int activityFrames = 0, readFrames = 0;

const int authorizedCount = sizeof(authorizedUIDs) / sizeof(authorizedUIDs[0]);
//*****************************************************************************************//
void setup() {
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZ, OUTPUT);
  pinMode(ACTIVITY_LED, OUTPUT);
  pinMode(REFRESH_DATABASE, INPUT);
  Serial.begin(9600);  // Initialize serial communications with the PC
  SPI.begin();         // Init SPI bus
  mfrc522.PCD_Init();  // Init MFRC522 card
  Serial.setTimeout(1);
}

void readSuccess(String uuid) {
  for (uint8_t i = 0; i < 3; i++) {
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(BUZZ, HIGH);
    delay(10);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(BUZZ, LOW);
    delay(10);
  }

  Serial.println(uuid);//retu("scan", uuid));
}


void readFail() {
}

bool isAuthorized(byte *uid) {
  for (int i = 0; i < 1; i++) {
    bool match = true;
    for (int j = 0; j < 4; j++) {
      if (uid[j] != authorizedUIDs[i][j]) {
        match = false;
        break;
      }
    }
    if (match) return true;
  }
  return false;
}

String retu(String type, String content) {
  String a = ("{\"type\":\"" + type +"\",\"content\":\""+content+"\"}");
  return a;
}

bool isRefresh = false;

//*****************************************************************************************//
void loop() {
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  if (v == 0 || v == 0xFF) {

    for (uint8_t i = 0; i < 6; i++) {
    digitalWrite(i % 2 == 0 ? GREEN_LED : RED_LED, HIGH);
    digitalWrite(BUZZ, HIGH);
    delay(40);
    digitalWrite(i % 2 == 0 ? GREEN_LED : RED_LED, LOW);
    digitalWrite(BUZZ, LOW);
    delay(40);
  }
    delay(1500);
    mfrc522.PCD_Init();
    return;
  }
  /*int hsh = digitalRead(REFRESH_DATABASE);
  bool us = hsh == LOW;
  if (us) {
    for (uint8_t i = 0; i < 23; i++) {
        digitalWrite(BUZZ, HIGH);
        delay(i);
        digitalWrite(BUZZ, LOW);
        delay(10);
      }
      delay(1000);
    return;
  }*/

  activityFrames++;
  if (activityFrames == 60) {
    digitalWrite(ACTIVITY_LED, HIGH);
  }
  if (activityFrames == 65) {
    activityFrames = 0;
    digitalWrite(ACTIVITY_LED, LOW);
  }
 if (Serial.available() > 0) {
    String read = Serial.readString();
    digitalWrite(ACTIVITY_LED, LOW);
    if (read == ("success")) {
      digitalWrite(GREEN_LED, HIGH);
      for (uint8_t i = 0; i < 3; i++) {
        digitalWrite(BUZZ, HIGH);
        delay(100);
        digitalWrite(BUZZ, LOW);
        delay(100);
      }
      delay(1500);
      digitalWrite(GREEN_LED, LOW);
    }
    if (read == "cooldown") {
      for (uint8_t i = 0; i < 6; i++) {
      digitalWrite(RED_LED, HIGH);
        digitalWrite(BUZZ, HIGH);
        delay(200);
        digitalWrite(BUZZ, LOW);
      digitalWrite(RED_LED, LOW);
      delay(200);
      }
      //delay(500);
    }
    if (read == "maxout") {
      for (uint8_t i = 0; i < 20; i++) {
      digitalWrite(RED_LED, HIGH);
        digitalWrite(BUZZ, HIGH);
        delay(50);
        digitalWrite(BUZZ, LOW);
      digitalWrite(RED_LED, LOW);
      delay(10 + 2*i);
      }
      //delay(500);
    }
    if (read == "fail") {
      digitalWrite(RED_LED, HIGH);
      for (uint8_t i = 0; i < 1; i++) {
        digitalWrite(BUZZ, HIGH);
        delay(1000);
        digitalWrite(BUZZ, LOW);
        //delay(100);
      }
      delay(500);
      digitalWrite(RED_LED, LOW);
    }
  }


  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }


  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  digitalWrite(ACTIVITY_LED, LOW);


  String uidString;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uidString += String(mfrc522.uid.uidByte[i], HEX);
  }
  //uidString.toUpperCase(); // Convert UID to uppercase


  //Serial.println(uidString);
  //byte key[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };  // Default key A

  readSuccess(uidString);
  // Authenticate and write to block 2
 

mfrc522.PICC_HaltA();
mfrc522.PCD_StopCrypto1();
}



//*****************************************************************************************//
