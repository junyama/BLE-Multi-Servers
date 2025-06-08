//#include <NimBLEDevice.h>
#include "MyBLE.cpp"
//#include "MyScanCallbacks.cpp"
#include "MyLog.cpp"
#include "MyTimer.cpp"
#include "MyMqtt.cpp"
#include "MyScanCallBacks.cpp"

extern char *TAG;
extern VoltMater voltMater;
extern LipoMater lipoMater;
extern MyMqtt myMqtt;
extern MyBLE myBleArr[3];
extern MyScanCallbacks myScanCallbacks;

void mqttCallback(char *topic_, byte *payload, unsigned int length)
    {
        DEBUG_PRINT("mqttCallback invoked.");

        String msgStr = "";
        for (int i = 0; i < length; i++)
        {
            msgStr = msgStr + (char)payload[i];
        }
        String logStr = "Message arrived[";
        logStr += topic_;
        logStr += "] ";
        logStr += msgStr;
        DEBUG_PRINT(logStr.c_str());
        // LOGLCD(TAG, logStr);

        if (String(topic_).equals("cmnd/" + voltMater.topic + "getState"))
        {
            DEBUG_PRINT("responding to geState of %s", voltMater.topic.c_str());
            myMqtt.publishJson(("stat/" + voltMater.topic + "RESULT").c_str(), voltMater.getState(), false);
            return;
        }

        if (String(topic_).equals("cmnd/" + lipoMater.topic + "getState"))
        {
            DEBUG_PRINT("responding to geState of %s", lipoMater.topic.c_str());
            myMqtt.publishJson(("stat/" + lipoMater.topic + "RESULT").c_str(), lipoMater.getState(), false);
            return;
        }

        int chargeStatus;
        int dischargeStatus;
        bool connectionStatus;
        for (int bleIndex = 0; bleIndex < myScanCallbacks.numberOfAdvDevices; bleIndex++)
        {
            DEBUG_PRINT("myBleArr[%d].topic = %s", bleIndex, myBleArr[bleIndex].topic.c_str());
            // DEBUG_PRINT("myBleArr[%d].available = %d", bleIndex, myBleArr[bleIndex].available);
            // if (!myBleArr[bleIndex].available)
            {
                DEBUG_PRINT("myBleArr[%d] is not available so continue.", bleIndex);
                continue;
            }
            String deviceTopic = myBleArr[bleIndex].topic;
            chargeStatus = myBleArr[bleIndex].packBasicInfo.MosfetStatus & 1;
            dischargeStatus = (myBleArr[bleIndex].packBasicInfo.MosfetStatus & 2) >> 1;
            // connectionStatus = myBleArr[bleIndex].isConnected();

            if (String(topic_).equals("cmnd/" + deviceTopic + "getBmsState"))
            {
                DEBUG_PRINT("responding to getBmsState of %s!", deviceTopic.c_str());
                myMqtt.publishJson(("stat/" + deviceTopic + "RESULT").c_str(), myBleArr[bleIndex].getState(), false);
                return;
            }
            /*
            if (String(topic_).equals("cmnd/" + deviceTopic + "connection"))
            {
                DEBUG_PRINT("connection status: %d", connectionStatus);
                if (msgStr.equals(""))
                {
                    if (connectionStatus)
                        msgStr = "ON";
                    else
                        msgStr = "OFF";
                }
                else if (msgStr.equals("0"))
                {
                    myBleArr[bleIndex].disconnectFromServer();
                    msgStr = "OFF";
                    connectionStatus = false;
                }
                else if (msgStr.equals("1"))
                {
                    myBleArr[bleIndex].connectToServer();
                    msgStr = "ON";
                    connectionStatus = true;
                }
                else
                {
                    msgStr = "INVALID";
                }
                DEBUG_PRINT("responding to connection");
                myMqtt.publish(("stat/" + deviceTopic + "CONNECTION").c_str(), msgStr.c_str());
                msgStr = "{\"connectionStatus\": " + String(connectionStatus) + "}";
                myMqtt.publish(("stat/" + deviceTopic + "RESULT").c_str(), msgStr.c_str());
                myMqtt.publish(("stat/" + deviceTopic + "STATE").c_str(), msgStr.c_str());
                return;
            }*/

            if ((String(topic_).equals("cmnd/" + deviceTopic + "charge")) || ((String(topic_).equals("cmnd/" + deviceTopic + "discharge"))))
            {
                if (String(topic_).equals("cmnd/" + deviceTopic + "charge"))
                {
                    DEBUG_PRINT("charge status: %d, discharge status: %d", chargeStatus, dischargeStatus);
                    if (msgStr.equals(""))
                    {
                        if (chargeStatus)
                            msgStr = "ON";
                        else
                            msgStr = "OFF";
                    }
                    else if (msgStr.equals("0"))
                    {
                        myBleArr[bleIndex].mosfetCtrl(0, dischargeStatus);
                        chargeStatus = 0;
                        msgStr = "OFF";
                    }
                    else if (msgStr.equals("1"))
                    {
                        myBleArr[bleIndex].mosfetCtrl(1, dischargeStatus);
                        chargeStatus = 1;
                        msgStr = "ON";
                    }
                    else if (msgStr.equals("toggle"))
                    {
                        myBleArr[bleIndex].mosfetCtrl((chargeStatus ^ 1), dischargeStatus);
                        chargeStatus = chargeStatus ^ 1;
                        msgStr = "TOGGLE";
                    }
                    else
                    {
                        msgStr = "INVALID";
                    }
                    DEBUG_PRINT("responding to charge!");
                    myMqtt.publish(("stat/" + deviceTopic + "CHARGE").c_str(), msgStr.c_str());
                }
                else if (String(topic_).equals("cmnd/" + deviceTopic + "discharge"))
                {
                    DEBUG_PRINT("charge status: %d, discharge status: %d", chargeStatus, dischargeStatus);
                    if (msgStr.equals(""))
                    {
                        if (dischargeStatus)
                            msgStr = "ON";
                        else
                            msgStr = "OFF";
                    }
                    else if (msgStr.equals("0"))
                    {
                        myBleArr[bleIndex].mosfetCtrl(chargeStatus, 0);
                        dischargeStatus = 0;
                        msgStr = "OFF";
                    }
                    else if (msgStr.equals("1"))
                    {
                        myBleArr[bleIndex].mosfetCtrl(chargeStatus, 1);
                        dischargeStatus = 1;
                        msgStr = "ON";
                    }
                    else if (msgStr.equals("toggle"))
                    {
                        myBleArr[bleIndex].mosfetCtrl(chargeStatus, (dischargeStatus ^ 1));
                        dischargeStatus = dischargeStatus ^ 1;
                        msgStr = "TOGGLE";
                    }
                    else
                    {
                        msgStr = "INVALID";
                    }
                    DEBUG_PRINT("responding to discharge!");
                    myMqtt.publish(("stat/" + deviceTopic + "DISCARGE").c_str(), msgStr.c_str());
                }
                msgStr = "{\"chargeStatus\": " + String(chargeStatus) + ", \"dischargeStatus\": " + String(dischargeStatus) + "}";
                myMqtt.publish(("stat/" + deviceTopic + "RESULT").c_str(), msgStr.c_str());
                myMqtt.publish(("stat/" + deviceTopic + "STATE").c_str(), msgStr.c_str());
                return;
            }
        }
    }