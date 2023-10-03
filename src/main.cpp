#include <Arduino.h>
#include <Arduino.h>
#include <BLE2902.h>
#include <BLEServer.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEDevice.h>

#define SERVICE_UUID "7d1cf919-457e-469f-af89-8d376f791275"
#define CHARACTERISIC_UUID_RX "114514d2-c5ee-4f53-9b21-247db8dc5dd5"
#define CHARACTERISIC_UUID_TX "19198102-9b0f-48fa-bfc3-e11234c74301"

// uint8_t* pvalue = 0;

BLEServer *pserver = NULL;
BLECharacteristic *pTxCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
bool isAdvertising = true;
int clientCount = 0;

//回调
class mServerCallback : public BLEServerCallbacks{
    void onConnect(BLEServer *pserver){
        deviceConnected = true;
        clientCount++;
        isAdvertising = false;
    }

    void onDisconnect(BLEServer *pserver){
        deviceConnected = false;
        clientCount--;
    }
};

class mBLECharacterisicCallback : public BLECharacteristicCallbacks{
    void onWrite(BLECharacteristic* pCharacteristic){
        std::string rxValue = pCharacteristic -> getValue();
        if (rxValue.length() > 0)
        {
            Serial.print("RX :");
            for (int i = 0; i < rxValue.length(); i++)
            {
                Serial.print(rxValue[i]);
            }
            Serial.println();
        }
        if (rxValue == "0")
        {
            digitalWrite(2,HIGH);
        } else {
            digitalWrite(2,LOW);
        }
        
        
    }
};



void setup() {
    //传感器
    Serial.begin(9600);
    pinMode(2,INPUT_PULLDOWN);

    //蓝牙

// 初始化设备:名称
    BLEDevice::init("ESP32");

// 创建ble服务，可多个
    pserver = BLEDevice::createServer();
    pserver -> setCallbacks(new mServerCallback());
    //服务
    BLEService* pservice = pserver -> createService(SERVICE_UUID);

// 创建特征
    pTxCharacteristic = pservice -> createCharacteristic(CHARACTERISIC_UUID_TX,
        BLECharacteristic::PROPERTY_NOTIFY |
        BLECharacteristic::PROPERTY_BROADCAST);
    pTxCharacteristic -> addDescriptor(new BLE2902());
    // pTxCharacteristic->setValue(pvalue,1);
    
    BLECharacteristic *pRXCharacteristic = pservice -> createCharacteristic(CHARACTERISIC_UUID_RX,
        BLECharacteristic::PROPERTY_WRITE );
    pRXCharacteristic -> setCallbacks(new mBLECharacterisicCallback());


    pservice->start();
//    pserver->getAdvertising()->start();
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    delay(100);
    Serial.println("等待连接：");

}

void loop() {

    int value1 = digitalRead(2);
    Serial.println(value1);

    delay(100);
    
    if (BLEDevice::getInitialized() && !isAdvertising && clientCount < 1){
        delay(500);
        BLEDevice::startAdvertising();
        isAdvertising = true;
        Serial.println("start advertising");
    }
    if (deviceConnected){
         if(value1 > 0){
            pTxCharacteristic->setValue("notify!");
            pTxCharacteristic->notify();
            Serial.println("notify");
            delay(900);
         }
        delay(100);
    } else {
        delay(3900);
    }
    
    if (!deviceConnected && oldDeviceConnected)
    {
        delay(500);
        pserver->startAdvertising();
        Serial.println("开始广播：");
        oldDeviceConnected = deviceConnected;
    }

    if (deviceConnected && !oldDeviceConnected)
    {
        oldDeviceConnected = deviceConnected;
    }

}
