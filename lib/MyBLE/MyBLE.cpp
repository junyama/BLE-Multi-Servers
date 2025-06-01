#ifndef MY_BLE_CPP
#define MY_BLE_CPP

#include <NimBLEDevice.h>
#include "MyLog.cpp"
// #include <MyNotifyCB.cpp>

typedef struct
{
    byte start;
    byte type;
    byte status;
    byte dataLen;
} bmsPacketHeaderStruct;

typedef struct
{
    uint16_t Volts; // unit 1mV
    int32_t Amps;   // unit 1mA
    int32_t Watts;  // unit 1W
    uint16_t CapacityRemainAh;
    uint8_t CapacityRemainPercent; // unit 1%
    uint32_t CapacityRemainWh;     // unit Wh
    uint16_t Temp1;                // unit 0.1C
    uint16_t Temp2;                // unit 0.1C
    uint16_t BalanceCodeLow;
    uint16_t BalanceCodeHigh;
    uint8_t MosfetStatus;
} packBasicInfoStruct;

typedef struct
{
    uint8_t NumOfCells;
    uint16_t CellVolt[15]; // cell 1 has index 0 :-/
    uint16_t CellMax;
    uint16_t CellMin;
    uint16_t CellDiff; // difference between highest and lowest
    uint16_t CellAvg;
    uint16_t CellMedian;
    uint32_t CellColor[15];
    uint32_t CellColorDisbalance[15]; // green cell == median, red/violet cell => median + c_cellMaxDisbalanceAdd commentMore actions
} packCellInfoStruct;

class MyBLE
{
public:
    const char *TAG = "MyBLE";

    NimBLERemoteCharacteristic *pChr_rx = nullptr;
    NimBLERemoteCharacteristic *pChr_tx = nullptr;
    NimBLEAddress peerAddress;
    bool toggle = false;
    byte ctrlCommand = 0;
    bool newPacketReceived = false;
    packBasicInfoStruct packBasicInfo; // here shall be the latest data got from BMS
    packCellInfoStruct packCellInfo;   // here shall be the latest data got from BMS
    int numberOfTemperature = 2;
    String deviceName = "UNKNOWN";

    NimBLEClientCallbacks clientCallbacks;

    void bmsGetInfo3()
    {
        //    DD     A5      03     00    FF     FD      77
        uint8_t data[7] = {0xdd, 0xa5, 0x3, 0x0, 0xff, 0xfd, 0x77};
        if (pChr_tx)
        {
            sendCommand(pChr_tx, data, sizeof(data));
            toggle = !toggle;
        }
    }

    void bmsGetInfo4()
    {
        //   DD  A5 04 00  FF  FC  77
        uint8_t data[7] = {0xdd, 0xa5, 0x4, 0x0, 0xff, 0xfc, 0x77};
        if (pChr_tx)
        {
            sendCommand(pChr_tx, data, sizeof(data));
            toggle = !toggle;
        }
    }

    void bmsGetInfo5()
    {
        //   DD  A5 05 00  FF  FC  77
        uint8_t packet[7] = {0xdd, 0xa5, 0x5, 0x0, 0xff, 0xfb, 0x77};
        if (pChr_tx)
            sendCommand(pChr_tx, packet, sizeof(packet));
    }

    void sendCommand(NimBLERemoteCharacteristic *pChr, uint8_t *data, uint32_t dataLen)
    {
        pChr->writeValue(data, dataLen);
    }

    bool processDeviceInfo(byte *data, unsigned int dataLen)
    {
        char chars[dataLen + 1];
        memcpy(chars, data, dataLen);
        chars[dataLen] = '\0';
        DEBUG_PRINT("deviceName = %s\n", chars);
        deviceName = String(chars);
        // LOGD(TAG, "deviceName: " + deviceName);
        // M5.Lcd.println(deviceName);
        return true;
    }

    bool bleCollectPacket(char *data, uint32_t dataSize) // reconstruct packet from BLE incomming data, called by notifyCallback function
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
            DEBUG_PRINT("got full packet");
            uint8_t packet[dataSize + previousDataSize];
            memcpy(packet, packetbuff, dataSize + previousDataSize);

            bmsProcessPacket(packet); // pass pointer to retrieved packet to processing function
            packetstate = 0;
            retVal = true;
        }
        return retVal;
    }

    int16_t two_ints_into16(int highbyte, int lowbyte) // turns two bytes into a single long integerAdd commentMore actions
    {
        // TRACE;
        int16_t result = (highbyte);
        result <<= 8;                // Left shift 8 bits,
        result = (result | lowbyte); // OR operation, merge the two
        return result;
    }

    bool processBasicInfo(packBasicInfoStruct *output, byte *data, unsigned int dataLen)
    {
        // TRACE;
        //  Expected data len
        /* Jun, it should not be checked for variable length
        if (dataLen != 0x1B)
        {
            LOGD(TAG, "BasicInfo data length invalid: " + String(dataLen));
            return false;
        }
        */

        output->Volts = ((uint32_t)two_ints_into16(data[0], data[1])) * 10; // Resolution 10 mV -> convert to milivolts   eg 4895 > 48950mV
        DEBUG_PRINT("output->Volts = %d\n", output->Volts);
        output->Amps = ((int32_t)two_ints_into16(data[2], data[3])) * 10; // Resolution 10 mA -> convert to miliamps

        output->Watts = output->Volts * output->Amps / 1000000; // W

        output->CapacityRemainAh = ((uint16_t)two_ints_into16(data[4], data[5])) * 10;
        output->CapacityRemainPercent = ((uint8_t)data[19]);

        // output->CapacityRemainWh = (output->CapacityRemainAh * c_cellNominalVoltage) / 1000000 * packCellInfo.NumOfCells;

        output->Temp1 = (((uint16_t)two_ints_into16(data[23], data[24])) - 2731);
        output->Temp2 = (((uint16_t)two_ints_into16(data[25], data[26])) - 2731);
        if (numberOfTemperature == 1)
            output->Temp2 = output->Temp1;
        output->BalanceCodeLow = (two_ints_into16(data[12], data[13]));
        output->BalanceCodeHigh = (two_ints_into16(data[14], data[15]));
        output->MosfetStatus = ((byte)data[20]);

        return true;
    }

    bool processCellInfo(packCellInfoStruct *output, byte *data, unsigned int dataLen)
    {
        // TRACE;
        uint16_t _cellSum = 0;
        uint16_t _cellMin = 5000;
        uint16_t _cellMax = 0;
        // uint16_t _cellAvg;
        // uint16_t _cellDiff;

        output->NumOfCells = dataLen / 2; // Data length * 2 is number of cells !!!!!!

        // go trough individual cells
        // for (byte i = 0; i < dataLen / 2; i++)
        for (byte i = 0; i < output->NumOfCells; i++)
        {
            output->CellVolt[i] = ((uint16_t)two_ints_into16(data[i * 2], data[i * 2 + 1])); // Resolution 1 mV
            _cellSum += output->CellVolt[i];
            if (output->CellVolt[i] > _cellMax)
            {
                _cellMax = output->CellVolt[i];
            }
            if (output->CellVolt[i] < _cellMin)
            {
                _cellMin = output->CellVolt[i];
            }

            // output->CellColor[i] = getPixelColorHsv(mapHue(output->CellVolt[i], c_cellAbsMin, c_cellAbsMax), 255, 255);
        }
        output->CellMin = _cellMin;
        output->CellMax = _cellMax;
        output->CellDiff = _cellMax - _cellMin; // Resolution 10 mV -> convert to volts
        output->CellAvg = _cellSum / output->NumOfCells;

        //----cell median calculation----
        uint16_t n = output->NumOfCells;
        uint16_t i, j;
        uint16_t temp;
        uint16_t x[n];

        for (uint8_t u = 0; u < n; u++)
        {
            x[u] = output->CellVolt[u];
        }

        for (i = 1; i <= n; ++i) // sort data
        {
            for (j = i + 1; j <= n; ++j)
            {
                if (x[i] > x[j])
                {
                    temp = x[i];
                    x[i] = x[j];
                    x[j] = temp;
                }
            }
        }

        if (n % 2 == 0) // compute median
        {
            output->CellMedian = (x[n / 2] + x[n / 2 + 1]) / 2;
        }
        else
        {
            output->CellMedian = x[n / 2 + 1];
        }
        /*
        for (uint8_t q = 0; q < output->NumOfCells; q++)
        {
            uint32_t disbal = abs(output->CellMedian - output->CellVolt[q]);
            // output->CellColorDisbalance[q] = getPixelColorHsv(mapHue(disbal, c_cellMaxDisbalance, 0), 255, 255);
        }
        */
        return true;
    }

    bool bmsProcessPacket(byte *packet)
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

        DEBUG_PRINT("Decision based on packet header type (%d)\n", pHeader->type);
        switch (pHeader->type)
        {
        case cBasicInfo3:
        {
            DEBUG_PRINT("bmsProcessPacket, process BasicInfo3. Type: %d\n", pHeader->type);
            result = processBasicInfo(&packBasicInfo, data, dataLen);
            DEBUG_PRINT("packBasicInfo.Volts = %d\n", packBasicInfo.Volts);
            newPacketReceived = true;
            break;
        }
        case cCellInfo4:
        {
            DEBUG_PRINT("bmsProcessPacket, process CellInfo4. Type: %d\n", pHeader->type);
            result = processCellInfo(&packCellInfo, data, dataLen);
            DEBUG_PRINT("packCellInfo.CellDiff = %d\n", packCellInfo.CellDiff);
            newPacketReceived = true;
            break;
        }
        /*case cMOSFETCtrl:
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
            DEBUG_PRINT("bmsProcessPacket, process DeviceName. Type: %d\n", pHeader->type);
            result = processDeviceInfo(data, dataLen);
            newPacketReceived = true;
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