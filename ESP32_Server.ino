#include "BLEDevice.h"
#include <WiFi.h>
#include "time.h"
#include <Arduino.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEEddystoneURL.h>
#include <BLEEddystoneTLM.h>
#include <BLEBeacon.h>
#define ENDIAN_CHANGE_U16(x) ((((x)&0xFF00) >> 8) + (((x)&0xFF) << 8))
#define HM_MAC "C4:BE:84:F4:D7:15"

static BLEUUID serviceUUID("FFE0");
static BLEUUID charUUID("FFE1");
static BLEUUID ClockUUID("0000ffe0-0000-1000-8000-00805f9b34fb");
static BLEUUID NullUUID("<NULL>");
boolean connect = false;
boolean connected = false;
boolean wantToConnect = true;
boolean connecting = false;
int i = 0;
String hour = "";
String minutes = "";

static BLEAddress *pServerAddress;
static BLERemoteCharacteristic* pRemoteCharacteristic;
BLEClient*  pClient;

const char* ssid       = "Au";
const char* password   = "fiatpunto";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;
struct tm timeinfo, localTime;
int lastConnection, now;

int scanTime = 1; //In seconds
BLEScan *pBLEScan;
int foundClock = 0;
boolean FindFlag = false;


class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      if (FindFlag) {
        return;
      }
      else {
        foundClock = 0;
      }

      if (advertisedDevice.haveServiceUUID()) {
        if (advertisedDevice.getServiceUUID().equals(ClockUUID)) {
          FindFlag = true;
          foundClock = 1;
          if (wantToConnect && !connecting) {
            Serial.println("Device available - and want to connect");
            connecting = 1;
          }
          else {
            Serial.println("Device available but dont want to connect");
            connecting = 0;
          }
        }

        return;
      }
    }
};
//Connect to BLE Server

bool connectToClock(BLEAddress pAddress)
{
  Serial.print("Connecting");
  pClient->connect(pAddress);
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr)
  {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  /* Obtain a reference to the characteristic in the service of the remote BLE server */
  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
  if (pRemoteCharacteristic == nullptr)
  {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  pRemoteCharacteristic->registerForNotify(notifyCallback);
  return true;
}


bool getTime(tm *timeinfo) {

  if (!getLocalTime(timeinfo)) {
    return false;
  }
  return true;
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Server Initialization");
  BLEDevice::init("");
  pClient = BLEDevice::createClient();
  pServerAddress = new BLEAddress(HM_MAC);

  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" CONNECTED TO WiFi");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  while (1) {
    if ( getTime(&timeinfo) ) {
      localTime = timeinfo;
      break;
    }
    else {
      Serial.println(" Can't get time .... retry");
      delay(100);
    }
  }

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99); // less or equal setInterval value

  Serial.print("Setup compleated\n");
}

void loop()
{

  if (getTime(&timeinfo)) {
    now = timeinfo.tm_hour * 3600 + timeinfo.tm_min * 60;
    if ( (lastConnection + 3600 ) <= now ) {
      wantToConnect = true;
    }
  }

  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.print("FoundClock:");
  Serial.println(foundClock);
  Serial.print("connecting:");
  Serial.println(connecting);

  if (connecting) {
    Serial.println("CONNECT TO CLOCK");
    connect = 1;
    wantToConnect = false;
  }
  else {
    Serial.println("DO NOTHING");
  }

  if ( !wantToConnect && !foundClock) {
    wantToConnect = true;
  }
  Serial.print("wantToConnect:");
  Serial.println(wantToConnect);
  FindFlag = false;
  pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
  delay(2000);
  Serial.println();

  if (connect)
  {
    if (!connectToClock(*pServerAddress)) {
      Serial.println("ERROR !!!!!!!!!!!!");
    }
    Serial.println("Connection has been made");
    connected = true;
    connect = false;
  }
  if (connected)
  {
    Serial.println("Send Data");
    //
    while (1) {
      if ( getTime(&localTime) ) {
        timeinfo = localTime;
        break;
      }
      else {
        Serial.println(" Can't get time .... retry");
        delay(101);
      }
    }

    lastConnection = timeinfo.tm_hour * 3600 + timeinfo.tm_min * 60;

    if (int(timeinfo.tm_hour) < 10) {
      hour = "0" + String(timeinfo.tm_hour);
    }
    else {
      hour = String(timeinfo.tm_hour);
    }
    if (int(timeinfo.tm_min) < 10) {
      minutes = "0" + String(timeinfo.tm_min);
    }
    else {
      minutes = String(timeinfo.tm_min);
    }

    String newValue = "T" + hour + minutes;
    Serial.println(newValue);
    pRemoteCharacteristic->writeValue(newValue.c_str(), newValue.length());
    delay(11);
    
    Serial.println("Disconnecting");
    connect = false;
  }
  if( connected ) {
    pClient->disconnect();
    connected = false;
  }
}
