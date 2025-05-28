#ifndef MY_BLE_CPP
#define MY_BLE_CPP

#include <NimBLEDevice.h>

typedef struct
{
    byte start;
    byte type;
    byte status;
    byte dataLen;
} bmsPacketHeaderStruct;

class MyBLE
{
public:
    static void bmsGetInfo3(NimBLERemoteCharacteristic *pChr)
    {
        //    DD     A5      03     00    FF     FD      77
        uint8_t data[7] = {0xdd, 0xa5, 0x3, 0x0, 0xff, 0xfd, 0x77};
        sendCommand(pChr, data, sizeof(data));
    }

    static void bmsGetInfo4(NimBLERemoteCharacteristic *pChr)
    {
        //   DD  A5 04 00  FF  FC  77
        uint8_t data[7] = {0xdd, 0xa5, 0x4, 0x0, 0xff, 0xfc, 0x77};
        sendCommand(pChr, data, sizeof(data));
    }

    static void bmsGetInfo5(NimBLERemoteCharacteristic *pChr)
    {
        //   DD  A5 05 00  FF  FC  77
        uint8_t packet[7] = {0xdd, 0xa5, 0x5, 0x0, 0xff, 0xfb, 0x77};
        sendCommand(pChr, packet, sizeof(packet));
    }
    static void sendCommand(NimBLERemoteCharacteristic *pChr, uint8_t *data, uint32_t dataLen)
    {
        pChr->writeValue(data, dataLen);
    }

    static bool processDeviceInfo(byte *data, unsigned int dataLen)
    {
        char chars[dataLen + 1];
        memcpy(chars, data, dataLen);
        chars[dataLen] = '\0';
        Serial.printf("deviceName = %s\n", chars);
        // deviceName = String(chars);
        // LOGD(TAG, "deviceName: " + deviceName);
        // M5.Lcd.println(deviceName);
        return true;
    }

    static bool bleCollectPacket(char *data, uint32_t dataSize) // reconstruct packet from BLE incomming data, called by notifyCallback function
    {
        static uint8_t packetstate = 0; // 0 - empty, 1 - first half of packet received, 2- second half of packet received
        static uint8_t packetbuff[40] = {0x0};
        static uint32_t previousDataSize = 0;
        bool retVal = false;
        // hexDump(data,dataSize);

        if (data[0] == 0xdd && packetstate == 0) // probably got 1st half of packet
        {
            packetstate = 1;
            previousDataSize = dataSize;
            for (uint8_t i = 0; i < dataSize; i++)
            {
                packetbuff[i] = data[i];
            }
            retVal = false;
        }

        if (data[dataSize - 1] == 0x77 && packetstate == 1) // probably got 2nd half of the packet
        {
            packetstate = 2;
            for (uint8_t i = 0; i < dataSize; i++)
            {
                packetbuff[i + previousDataSize] = data[i];
            }
            retVal = false;
        }

        if (packetstate == 2) // got full packet
        {
            Serial.println("got full packet");
            uint8_t packet[dataSize + previousDataSize];
            memcpy(packet, packetbuff, dataSize + previousDataSize);

            bmsProcessPacket(packet); // pass pointer to retrieved packet to processing function
            packetstate = 0;
            retVal = true;
        }
        return retVal;
    }

    static bool bmsProcessPacket(byte *packet)
    {
        const byte cBasicInfo3 = 3;    // type of packet 3= basic info
        const byte cCellInfo4 = 4;     // type of packet 4= individual cell info
        const byte cDeviceName5 = 5;   // type of packet 5= Device Name
        const byte cMOSFETCtrl = 0xE1; // type of packet E1= MOSFET Control

        /*
        bool isValid = isPacketValid(packet);

        if (isValid != true)
        {
            MyLOG::DISABLE_LOGD = false;
            LOGD(TAG, "Invalid packer received");
            return false;
        }
        */

        bmsPacketHeaderStruct *pHeader = (bmsPacketHeaderStruct *)packet;
        byte *data = packet + sizeof(bmsPacketHeaderStruct); // TODO Fix this ugly hack
        unsigned int dataLen = pHeader->dataLen;

        bool result = false;

        Serial.printf("Decision based on packet header type (%d)\n", pHeader->type);
        switch (pHeader->type)
        {
        /*case cBasicInfo3:
        {
            //MyLOG::DISABLE_LOGD = true;
            //LOGD(TAG, "bmsProcessPacket, process BasicInfo");
            //MyLOG::DISABLE_LOGD = false;
            result = processBasicInfo(&packBasicInfo, data, dataLen);
            newPacketReceived = true;
            break;
        }
        case cCellInfo4:
        {
            MyLOG::DISABLE_LOGD = true;
            LOGD(TAG, "bmsProcessPacket, process CellInfo");
            MyLOG::DISABLE_LOGD = false;
            result = processCellInfo(&packCellInfo, data, dataLen);
            newPacketReceived = true;
            break;
        }
        case cMOSFETCtrl:
        {
            MyLOG::DISABLE_LOGD = false;
            LOGD(TAG, "bmsProcessPacket, process MOSFETCtrl");
            newPacketReceived = true;
            break;
        }*/
        case cDeviceName5:
        {
            // MyLOG::DISABLE_LOGD = false;
            // LOGD(TAG, "bmsProcessPacket, process DeviceName");
            Serial.printf("bmsProcessPacket, process DeviceName. Type: %d\n", pHeader->type);
            result = processDeviceInfo(data, dataLen);
            // newPacketReceived = true;
            break;
        }
        default:
            result = false;
            char buff[256];
            sprintf(buff, "Unsupported packet type detected. Type: %d\n", pHeader->type);
            // LOGD(TAG, buff);
        }
        return result;
    }
};

#endif /* MY_BLE_CPP_ */