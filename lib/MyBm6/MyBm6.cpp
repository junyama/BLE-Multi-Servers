#include "MyBm6.hpp"

MyBm6::MyBm6()
{
}

MyBm6::MyBm6(NimBLEAddress peerAddress_, String deviceName_) : peerAddress(peerAddress_), mac(String(peerAddress_.toString().c_str())), deviceName(deviceName_)
{
}

void MyBm6::sendInfoCommand()
{
    DEBUG_PRINT("send info request command to %s: Service = %s, Charastaric = %s\n",
                pChr_tx->getClient()->getPeerAddress().toString().c_str(),
                pChr_tx->getRemoteService()->getUUID().toString().c_str(),
                pChr_tx->getUUID().toString().c_str());
    uint8_t data[16] = {0x69, 0x7e, 0xa0, 0xb5, 0xd5, 0x4c, 0xf0, 0x24, 0xe7, 0x94, 0x77, 0x23, 0x55, 0x55, 0x41, 0x14};
    if (pChr_tx)
    {
        sendCommand(pChr_tx, data, sizeof(data));
    }
    newPacketReceived = false;
}

void MyBm6::sendCommand(NimBLERemoteCharacteristic *pChr, uint8_t *data, uint32_t dataLen)
{
    pChr->writeValue(data, dataLen);
}

JsonDocument MyBm6::getState()
{
    JsonDocument doc;
    doc["deviceName"] = deviceName;
    doc["connected"] = (int)connected;
    doc["voltage"] = voltage;
    doc["temperature"] = temperature;
    doc["soc"] = soc;
    return doc;
}

bool MyBm6::bleCollectPacket(char *data, uint32_t dataSize)
{
    newPacketReceived = true;
    if (MyLog::DEBUG)
    {
        DEBUG_PRINT("Received data[%d]: ", dataSize);
        for (int index = 0; index < dataSize; index++)
        {
            Serial.printf("0x%02X, ", (unsigned char)data[index]);
        }
        Serial.printf("\n");
    }
    myAes.decrypt(data);
    if (MyLog::DEBUG)
    {
        DEBUG_PRINT("Decrypted data[16]: ");
        for (int i = 0; i < 16; i++)
        {
            Serial.printf("0x%02X, ", myAes.decryptedText[i]);
        }
        Serial.println();
    }
    char bytes[2];
    bytes[0] = myAes.decryptedText[7];
    DEBUG_PRINT("bytes[0]: %02X\n", bytes[0]);
    bytes[1] = myAes.decryptedText[8];
    DEBUG_PRINT("bytes[1]: %02X\n", bytes[1]);
    int volt_i = (int)bytes[0] * 256 + (int)bytes[1];
    DEBUG_PRINT("volt_i: %d\n", volt_i);
    voltage = volt_i / 100.0;
    DEBUG4_PRINT("voltage: %5.2f volt\n", voltage);

    bytes[0] = myAes.decryptedText[4];
    DEBUG_PRINT("bytes[0]: %02X\n", bytes[0]);
    bytes[1] = myAes.decryptedText[5];
    DEBUG_PRINT("bytes[1]: %02X\n", bytes[1]);
    temperature = (int)bytes[0];
    DEBUG4_PRINT("temprature: %d C\n", temperature);

    soc = (int)myAes.decryptedText[6];
    DEBUG4_PRINT("soc: %d %%\n", soc);

    return true;
}

/*
bool MyBm6::processInfo(byte *data, unsigned int dataLen)
{
    return true;
}
*/

bool MyBm6::timeout(unsigned long currentTime)
{
    unsigned long tempTime = lastMeasurment + measurmentIntervalMs;
    if (currentTime >= tempTime)
    {
        DEBUG_PRINT("currentTime(%lu) >= lastMeasurment + measurmentIntervalMs(%lu)\n", currentTime, tempTime);
        lastMeasurment = currentTime;
        return true;
    }
    else
        return false;
}