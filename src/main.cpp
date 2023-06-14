/**
 * @file player-littlefs-i2s.ino
 * @brief example using the LittleFS library
 *
 * @author Phil Schatzmann
 * @copyright GPLv3
 */

#include "AudioTools.h"
#include "AudioLibs/AudioSourceLittleFS.h"
#include "AudioCodecs/CodecMP3Helix.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

const char *startFilePath1 = "/1";
const char *startFilePath2 = "/2";
const char *ext = "mp3";

AudioSourceLittleFS s1(startFilePath1, ext);
AudioSourceLittleFS s2(startFilePath2, ext);
I2SStream i2s;
MP3DecoderHelix decoder;
AudioPlayer player(s1, i2s, decoder);

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      if (value.length() > 0) {
        Serial.print("New value: ");
        for (int i = 0; i < value.length(); i++)
          Serial.print(value[i]);
        if(value=="1") {
          player.setAudioSource(s1);
          player.begin();
        } else {
          player.setAudioSource(s2);
          player.begin();
        }

      }
    }
};


void printMetaData(MetaDataType type, const char *str, int len)
{
  Serial.print("==> ");
  Serial.print(toStr(type));
  Serial.print(": ");
  Serial.println(str);
}

void setup()
{
  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Info);

  BLEDevice::init("MyESP32");
  BLEServer *pServer = BLEDevice::createServer();

  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());

  pCharacteristic->setValue("Hello World");
  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();

  // setup output
  auto cfg = i2s.defaultConfig(TX_MODE);
  i2s.begin(cfg);

  // setup player
  // source.setFileFilter("*Bob Dylan*");
  player.setMetadataCallback(printMetaData);
}

void loop()
{
  player.copy();
}