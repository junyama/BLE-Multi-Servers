#include "MySdCard.hpp"

MySdCard::MySdCard(MyLcd2 *myLcd_) : myLcd(myLcd_)
{
}

void MySdCard::setup()
{
    while (false == SD.begin(GPIO_NUM_4, SPI, 15000000))
    {
        M5.Lcd.println("Insert SD card...");
        delay(2000);
    }
    M5.Lcd.println("SD card recognized!");
}

void MySdCard::listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
    DEBUG_PRINT("Listing directory: %s", dirname);

    File root = fs.open(dirname);
    if (!root)
    {
        DEBUG_PRINT("Failed to open directory\n");
        return;
    }
    if (!root.isDirectory())
    {
        DEBUG_PRINT("Not a directory\n");
        return;
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            // Serial.print("  DIR : \n");
            DEBUG_PRINT("DIR : %s", file.name());
            if (levels)
            {
                listDir(fs, file.name(), levels - 1);
            }
        }
        else
        {
            // Serial.print("  FILE: \n");
            // Serial.print(file.name());
            // Serial.print("  SIZE: \n");
            DEBUG_PRINT("FILE: %s, SIZE: %d", file.name(), file.size());
        }
        file = root.openNextFile();
    }
}

void MySdCard::createDir(fs::FS &fs, const char *path)
{
    DEBUG_PRINT("Creating Dir: %s", path);
    if (fs.mkdir(path))
    {
        DEBUG_PRINT("Directry created\n");
    }
    else
    {
        DEBUG_PRINT("mkdir failed\n");
    }
}

void MySdCard::removeDir(fs::FS &fs, const char *path)
{
    DEBUG_PRINT("Removing an empty Dir: %s", path);
    if (fs.rmdir(path))
    {
        DEBUG_PRINT("Directry removed\n");
    }
    else
    {
        DEBUG_PRINT("rmdir failed\n");
    }
}

void MySdCard::removeDirR(fs::FS &fs, const char *path)
{
    DEBUG_PRINT("Removing directory recursively: %s\n", path);
    File root = fs.open(path);
    if (!root)
    {
        DEBUG_PRINT("Failed to open directory\n");
        return;
    }
    if (!root.isDirectory())
    {
        DEBUG_PRINT("Not a directory\n");
        return;
    }
    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            DEBUG_PRINT("DIR : %s", file.name());
            removeDirR(fs, file.name());
        }
        else
        {
            DEBUG_PRINT("removing FILE: %s, SIZE: %d", file.name(), file.size());
            fs.remove(file.name());
        }
        file = root.openNextFile();
    }
    // DEBUG_PRINT("Removing directory: " + String(path));
    removeDir(fs, path);
    // DEBUG_PRINT("Removed directory: " + String(path));
}

void MySdCard::readFile(fs::FS &fs, const char *path, String &output)
{
    DEBUG_PRINT("Reading file: %s", path);

    File file = fs.open(path, FILE_READ);
    if (!file)
    {
        DEBUG_PRINT("Failed to open file for reading\n");
        return;
    }
    output = "";
    while (file.available())
    {
        output = output + file.readString();
    }
    /*
    while (file.available())
    {
        Serial.write(file.read());
    }
    */
    file.close();
}

void MySdCard::writeFile(fs::FS &fs, const char *path, const char *message)
{
    DEBUG_PRINT("Writing file: %s", path);

    File file = fs.open(path, FILE_WRITE);
    if (!file)
    {
        DEBUG_PRINT("Failed to open file for writing\n");
        return;
    }
    if (file.print(message))
    {
        DEBUG_PRINT("File written\n");
    }
    else
    {
        DEBUG_PRINT("Write failed\n");
    }
    file.close();
}

void MySdCard::appendFile(fs::FS &fs, const char *path, const char *message)
{
    DEBUG_PRINT("Appending to file: %s", path);

    File file = fs.open(path, FILE_APPEND);
    if (!file)
    {
        DEBUG_PRINT("Failed to open file for appending\n");
        return;
    }
    if (file.print(message))
    {
        DEBUG_PRINT("Message appended\n");
    }
    else
    {
        DEBUG_PRINT("Append failed\n");
    }
    file.close();
}

void MySdCard::renameFile(fs::FS &fs, const char *path1, const char *path2)
{
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2))
    {
        DEBUG_PRINT("File renamed\n");
    }
    else
    {
        DEBUG_PRINT("Rename failed\n");
    }
}

void MySdCard::deleteFile(fs::FS &fs, const char *path)
{
    Serial.printf("Deleting file: %s\n", path);
    if (fs.remove(path))
    {
        DEBUG_PRINT("File deleted\n");
    }
    else
    {
        DEBUG_PRINT("Delete failed\n");
    }
}

void MySdCard::testFileIO(fs::FS &fs, const char *path)
{
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if (file)
    {
        len = file.size();
        size_t flen = len;
        start = millis();
        while (len)
        {
            size_t toRead = len;
            if (toRead > 512)
            {
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    }
    else
    {
        DEBUG_PRINT("Failed to open file for reading\n");
    }

    file = fs.open(path, FILE_WRITE);
    if (!file)
    {
        DEBUG_PRINT("Failed to open file for writing\n");
        return;
    }

    size_t i;
    start = millis();
    for (i = 0; i < 2048; i++)
    {
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
}

JsonDocument MySdCard::loadConfig(String fileName)
{
    String textStr = "";
    readFile(SD, fileName.c_str(), textStr);
    myLcd->println("loading config file...");
    DEBUG_PRINT("configJsonText: %s", textStr.c_str());
    JsonDocument configJson;
    DeserializationError error = deserializeJson(configJson, textStr.c_str());
    if (error)
    {
        DEBUG_PRINT("Deserialization error.");
    }
    return configJson;
}

void MySdCard::saveConfig(JsonDocument configJson, String fileName)
{
    String jsonStr;
    serializeJsonPretty(configJson, jsonStr);
    DEBUG_PRINT("writing configuration file: %s", fileName.c_str());
    writeFile(SD, fileName.c_str(), jsonStr.c_str());
}
