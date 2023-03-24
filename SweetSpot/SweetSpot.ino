#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include <set>
#include <iostream>
#include <string>
#include <vector>

using namespace std;
#include <Firebase_ESP_Client.h>

#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

extern "C" {
  #include <user_interface.h>
}

#define DATA_LENGTH           112

#define TYPE_MANAGEMENT       0x00
#define TYPE_CONTROL          0x01
#define TYPE_DATA             0x02
#define SUBTYPE_PROBE_REQUEST 0x04
using namespace std;


// Firebase Realtime Database credentials
#define WIFI_SSID "Belltower Coffeehouse"
#define WIFI_PASSWORD "901coffeehouse"

// Insert Firebase project API Key
#define API_KEY "AIzaSyDbCwzqfszPXF2W4NCUBki6Vnu6-YNUu6E"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://sweetspot-222116-default-rtdb.firebaseio.com" 
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;

bool signupOK = false;

struct RxControl {
 signed rssi:8; // signal intensity of packet
 unsigned rate:4;
 unsigned is_group:1;
 unsigned:1;
 unsigned sig_mode:2; // 0:is 11n packet; 1:is not 11n packet;
 unsigned legacy_length:12; // if not 11n packet, shows length of packet.
 unsigned damatch0:1;
 unsigned damatch1:1;
 unsigned bssidmatch0:1;
 unsigned bssidmatch1:1;
 unsigned MCS:7; // if is 11n packet, shows the modulation and code used (range from 0 to 76)
 unsigned CWB:1; // if is 11n packet, shows if is HT40 packet or not
 unsigned HT_length:16;// if is 11n packet, shows length of packet.
 unsigned Smoothing:1;
 unsigned Not_Sounding:1;
 unsigned:1;
 unsigned Aggregation:1;
 unsigned STBC:2;
 unsigned FEC_CODING:1; // if is 11n packet, shows if is LDPC packet or not.
 unsigned SGI:1;
 unsigned rxend_state:8;
 unsigned ampdu_cnt:8;
 unsigned channel:4; //which channel this packet in.
 unsigned:12;
};

struct SnifferPacket{
    struct RxControl rx_ctrl;
    uint8_t data[DATA_LENGTH];
    uint16_t cnt;
    uint16_t len;
};
int macaddressCount = 0;
// Declare each custom function (excluding built-in, such as setup and loop) before it will be called.
// https://docs.platformio.org/en/latest/faq.html#convert-arduino-file-to-c-manually
static void showMetadata(SnifferPacket *snifferPacket);
static void ICACHE_FLASH_ATTR sniffer_callback(uint8_t *buffer, uint16_t length);
static void printDataSpan(uint16_t start, uint16_t size, uint8_t* data);
static void getMAC(char *addr, uint8_t* data, uint16_t offset);
void channelHop();

// A list of known OUI values assigned by the IEEE
std::vector<int> knownOUIs = {0x0000C0, 0x08002B, 0x080030};

bool isRandomizedMAC(std::string macAddress) {
    // Extract the first 24 bits from the MAC address
    std::string ouiStr = macAddress.substr(0, 8);

    // Convert the 24-bit value to an integer
    int oui = std::stoi(ouiStr, nullptr, 16);

    // Check if the OUI matches any known OUIs
    for (int i = 0; i < knownOUIs.size(); i++) {
        if (oui == knownOUIs[i]) {
            return false;
        }
    }

    return true;
}

static void showMetadata(SnifferPacket *snifferPacket) {

  unsigned int frameControl = ((unsigned int)snifferPacket->data[1] << 8) + snifferPacket->data[0];

  uint8_t version      = (frameControl & 0b0000000000000011) >> 0;
  uint8_t frameType    = (frameControl & 0b0000000000001100) >> 2;
  uint8_t frameSubType = (frameControl & 0b0000000011110000) >> 4;
  uint8_t toDS         = (frameControl & 0b0000000100000000) >> 8;
  uint8_t fromDS       = (frameControl & 0b0000001000000000) >> 9;

  // Only look for probe request packets
  if (frameType != TYPE_MANAGEMENT ||
      frameSubType != SUBTYPE_PROBE_REQUEST)
        return;
 Serial.println("");
  Serial.print("RSSI: ");
  Serial.print(snifferPacket->rx_ctrl.rssi, DEC);

 Serial.print(" Ch: ");
  Serial.print(wifi_get_channel());

    Serial.print(" Peer MAC: ");
  char addr[] = "00:00:00:00:00:00";
  getMAC(addr, snifferPacket->data, 10);

 

  uint8_t SSID_length = snifferPacket->data[25];
  Serial.print(" SSID: ");
  printDataSpan(26, SSID_length, snifferPacket->data);

  //Serial.println();
}
std::set<String> macAddresses;
/**
 * Callback for promiscuous mode
 */
static void ICACHE_FLASH_ATTR sniffer_callback(uint8_t *buffer, uint16_t length) {
   
    // Get MAC address of the device broadcasting the beacon frame
   
  struct SnifferPacket *snifferPacket = (struct SnifferPacket*) buffer;
  showMetadata(snifferPacket);
}

static void printDataSpan(uint16_t start, uint16_t size, uint8_t* data) {
  for(uint16_t i = start; i < DATA_LENGTH && i < start+size; i++) {
    Serial.write(data[i]);
  }
}

static void getMAC(char *addr, uint8_t* data, uint16_t offset) {
  
String macAddress = "";
char tmp[20];

sprintf(tmp, "%02x:%02x:%02x:%02x:%02x:%02x", data[offset+0], data[offset+1], data[offset+2], data[offset+3], data[offset+4], data[offset+5]);
macAddress = String(tmp);
std::string stdStr(macAddress.c_str());
std::string newMac = stdStr;
Serial.print(macAddress);
    if (isRandomizedMAC(newMac)) {
       Serial.print(" Randomized Mac Address ");
    } else {
       //Serial.println("not a random Mac Address");
       Serial.println(macAddress);
    }
   if (macAddresses.count(macAddress) == 0) {
      // If not, add it to the set and print it
      macAddresses.insert(macAddress);
      Serial.println(" Unique Mac Address ");
     // Serial.println(macAddress);
    } else {
      Serial.print(" New Mac address found ");
     // Serial.println(macAddress);
    }


 //  Serial.println();
}

#define CHANNEL_HOP_INTERVAL_MS   1000
static os_timer_t channelHop_timer;

/**
 * Callback for channel hoping
 */
void channelHop()
{
  // hoping channels 1-13
  uint8 new_channel = wifi_get_channel() + 1;
  if (new_channel > 13) {
    new_channel = 1;
  }
  wifi_set_channel(new_channel);
}

#define DISABLE 0
#define ENABLE  1

void setup() {
  // set the WiFi chip to "promiscuous" mode aka monitor mode
  Serial.begin(115200);
  delay(10);
  wifi_set_opmode(STATION_MODE);
  wifi_set_channel(1);
  wifi_promiscuous_enable(DISABLE);
  delay(10);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();


    /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;


/* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
    sendDataPrevMillis = millis();
    // Write an Int number on the database path test/int
    if (Firebase.RTDB.setInt(&fbdo, "MacAddresses/int", 350)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  
  wifi_set_promiscuous_rx_cb(sniffer_callback);
  delay(10);
  wifi_promiscuous_enable(ENABLE);

  // setup the channel hoping callback timer
  os_timer_disarm(&channelHop_timer);
  os_timer_setfn(&channelHop_timer, (os_timer_func_t *) channelHop, NULL);
  os_timer_arm(&channelHop_timer, CHANNEL_HOP_INTERVAL_MS, 1);

}

void loop() {
  
  delay(10);
 
 
   if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
    sendDataPrevMillis = millis();
    // Write an Int number on the database path test/int
    if (Firebase.RTDB.setInt(&fbdo, "MacAddresses/int", macaddressCount)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    


}
