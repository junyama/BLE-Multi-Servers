#ifndef MY_SDCARD_HPP
#define MY_SDCARD_HPP

#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

#include "MyLog.cpp"
#include "MyM5.hpp"

class MySdCard
{
private:
	const String TAG = "MySdCard";
	MyM5 *myM5;

public:
	MySdCard(MyM5 *myM5_);
	void setup();
	void listDir(fs::FS &fs, const char *dirname, uint8_t levels);
	void createDir(fs::FS &fs, const char *path);
	void removeDir(fs::FS &fs, const char *path);
	void removeDirR(fs::FS &fs, const char *path);
	void readFile(fs::FS &fs, const char *path, String &output);
	void writeFile(fs::FS &fs, const char *path, const char *message);
	void appendFile(fs::FS &fs, const char *path, const char *message);
	void renameFile(fs::FS &fs, const char *path1, const char *path2);
	void deleteFile(fs::FS &fs, const char *path);
	void testFileIO(fs::FS &fs, const char *path);
	JsonDocument loadConfig(String fileName);
	void saveConfig(JsonDocument configJson, String fileName);
	bool updatePOI(String poiURL);
};

#endif /* MY_SDCARD_HPP */