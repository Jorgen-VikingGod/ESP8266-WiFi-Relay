/*
 * ESP8266-WiFi-Relay.ino
 * ----------------------------------------------------------------------------
 * simple sketch of using ESP8266WebServer to switch relays on GPIO pins
 * it serves a simple website with toggle buttons for each relay
 * ----------------------------------------------------------------------------
* Source:     https://github.com/Jorgen-VikingGod/ESP8266-WiFi-Relay
 * Copyright:  Copyright (c) 2017 Juergen Skrotzky
 * Author:     Juergen Skrotzky <JorgenVikingGod@gmail.com>
 * License:    MIT License
 * Created on: 23.Nov. 2017
 * ----------------------------------------------------------------------------
 */

extern "C" {
#include "user_interface.h"
}

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <Wire.h>
#include <Hash.h>
#include <FS.h>
#include "ArduinoJson.h"                  //https://github.com/bblanchon/ArduinoJson
#include "webserver.h"

// declare and initial list of default relay settings
sRelay relay[8] = {sRelay(D5), sRelay(D6), sRelay(D7), sRelay(D8), sRelay(D3), sRelay(D4), sRelay(D2), sRelay(D1)};
uint8_t relayCount = sizeof(relay) / sizeof(sRelay);

// declare WiFiMulti
ESP8266WiFiMulti WiFiMulti;

/*
 * setup function
 * ----------------------------------------------------------------------------
 */
void setup() {
  // serial interface for debug messages
  if (_debug) {
    Serial.begin(115200);
  }
  // initial all relay pins as OUTPUT pin
  for (uint8_t idx = 0; idx < relayCount; ++idx) {
    pinMode(relay[idx].pin, OUTPUT);
  }
  
  DEBUG_PRINT(F("\n"));
  // use EEPROM
  EEPROM.begin(512);
  // use SPIFFS
  SPIFFS.begin();
  // handle requests
  server.on("/list", HTTP_GET, handleFileList);
  server.on("/edit", HTTP_GET, handleGetEditor);
  server.on("/edit", HTTP_PUT, handleFileCreate);
  server.on("/edit", HTTP_DELETE, handleFileDelete);
  server.on("/edit", HTTP_POST, []() {
    server.send(200, "text/plain", "");
  }, handleFileUpload);
  server.onNotFound([]() {
    if (!handleFileRead(server.uri()))
      server.send(404, "text/plain", "FileNotFound");
  });
  server.on("/all", HTTP_GET, handleGetAll);
  server.on("/current", HTTP_GET, sendCurrentStates);
  server.on("/relay", HTTP_GET, handleGetRelay);
  server.on("/toggle", HTTP_POST, handlePostToggle);
  server.on("/settings/status", HTTP_GET, handleGetStatus);
  server.on("/settings/configfile", HTTP_GET, handleGetConfigfile);
  server.on("/settings/configfile", HTTP_POST, handlePostConfigfile);
  server.begin();
  // Add service to MDNS
  MDNS.addService("http", "tcp", 80);
  // load config.json and connect to WiFI
  if (!loadConfiguration()) {
    // if no configuration found,
    // try to connect hard coded multi APs
    connectSTA(nullptr, nullptr, "wifi-relay");
  }
  // load last states from EEPROM
  loadSettings();
}

/*
 * main loop function
 * ----------------------------------------------------------------------------
 */  
void loop() {
  // handle http clients
  server.handleClient();
}

/*
 * load stored settings from EEPROM
 * ----------------------------------------------------------------------------
 */
void loadSettings() {
  // load relay states set outputs
  DEBUG_PRINT(F("loadSettings: "));
  for (uint8_t idx = 0; idx < relayCount; ++idx) {
    relay[idx].state = EEPROM.read(idx);
    DEBUG_PRINTF("relay%d: %d, ", idx+1, relay[idx].state);
    // initialize IO pin
    pinMode(relay[idx].pin, OUTPUT);
    // set relay state
    setRelay(idx, relay[idx].state);
  }
  DEBUG_PRINTLN("");
}

/*
 * load config.json file and try to connect
 * ----------------------------------------------------------------------------
 */
bool loadConfiguration() {
  // try to open config.json file
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    DEBUG_PRINTLN(F("[WARN] Failed to open config file"));
    return false;
  }
  size_t size = configFile.size();
  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);
  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());
  if (!json.success()) {
    DEBUG_PRINTLN(F("[WARN] Failed to parse config file"));
    return false;
  }
  DEBUG_PRINTLN(F("[INFO] Config file found"));
  if (_debug) {
    json.prettyPrintTo(Serial);
  }
  // get relay settings
  for (uint8_t idx = 0; idx < relayCount; ++idx) {
    relay[idx].type = json["relay"+String(idx+1)]["type"];
    relay[idx].pin = json["relay"+String(idx+1)]["pin"];
  }
  // get stored wifi settings
  const char * ssid = json["ssid"];
  const char * password = json["wifipwd"];
  const char * hostname = json["hostname"];
  // try to connect with stored settings and hard coded ones
  if (!connectSTA(ssid, password, hostname)) {
    return false;
  }
  return true;
}

/*
 * try to connect WiFi
 * ----------------------------------------------------------------------------
 */
bool connectSTA(const char* ssid, const char* password, const char * hostname) {
  WiFi.mode(WIFI_STA);
  // add here your hard coded backfall wifi ssid and password
  // WiFiMulti.addAP("<YOUR-SSID-1>", "<YOUR-WIFI-PASS-1>");
  // WiFiMulti.addAP("<YOUR-SSID-2>", "<YOUR-WIFI-PASS-2>");
  if (ssid && password) {
    WiFiMulti.addAP(ssid, password);
  }
  // We try it for 30 seconds and give up on if we can't connect
  unsigned long now = millis();
  uint8_t timeout = 30; // define when to time out in seconds
  DEBUG_PRINT(F("[INFO] Trying to connect WiFi: "));
  while (WiFiMulti.run() != WL_CONNECTED) {
    if (millis() - now < timeout * 1000) {
      delay(200);
      DEBUG_PRINT(F("."));
    } else {
      DEBUG_PRINTLN("");
      DEBUG_PRINTLN(F("[WARN] Couldn't connect in time"));
      return false;
    }
  }
  if (hostname) {
    if (MDNS.begin(hostname)) {
      DEBUG_PRINTLN(F("MDNS responder started"));
    }
  }
  DEBUG_PRINTLN("");
  DEBUG_PRINT(F("[INFO] Client IP address: ")); // Great, we connected, inform
  DEBUG_PRINTLN(WiFi.localIP());
  return true;
}

/*
 * send ESP8266 status
 * ----------------------------------------------------------------------------
 */
void sendStatus() {
  DEBUG_PRINTLN("sendStatus()");
  struct ip_info info;
  FSInfo fsinfo;
  if (!SPIFFS.info(fsinfo)) {
    DEBUG_PRINTLN(F("[ WARN ] Error getting info on SPIFFS"));
  }
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["command"] = "status";
  root["heap"] = ESP.getFreeHeap();
  root["chipid"] = String(ESP.getChipId(), HEX);
  root["cpu"] = ESP.getCpuFreqMHz();
  root["availsize"] = ESP.getFreeSketchSpace();
  root["availspiffs"] = fsinfo.totalBytes - fsinfo.usedBytes;
  root["spiffssize"] = fsinfo.totalBytes;
  wifi_get_ip_info(STATION_IF, &info);
  struct station_config conf;
  wifi_station_get_config(&conf);
  root["ssid"] = String(reinterpret_cast<char*>(conf.ssid));
  root["dns"] = printIP(WiFi.dnsIP());
  root["mac"] = WiFi.macAddress();
  IPAddress ipaddr = IPAddress(info.ip.addr);
  IPAddress gwaddr = IPAddress(info.gw.addr);
  IPAddress nmaddr = IPAddress(info.netmask.addr);
  root["ip"] = printIP(ipaddr);
  root["gateway"] = printIP(gwaddr);
  root["netmask"] = printIP(nmaddr);
  String json;
  root.printTo(json);
  DEBUG_PRINTLN(json);
  server.setContentLength(root.measureLength());
  server.send(200, "application/json", json);
}

/*
 * send whole configfile and status
 * ----------------------------------------------------------------------------
 */
void sendConfigfile() {
  // try to open config.json file
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    DEBUG_PRINTLN(F("[WARN] Failed to open config file"));
  }
  size_t size = configFile.size();
  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);
  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(buf.get());
  if (!root.success()) {
    DEBUG_PRINTLN(F("[WARN] Failed to parse config file"));
  }
  DEBUG_PRINTLN(F("[INFO] Config file found"));
  struct ip_info info;
  FSInfo fsinfo;
  if (!SPIFFS.info(fsinfo)) {
    DEBUG_PRINTLN(F("[ WARN ] Error getting info on SPIFFS"));
  }
  root["heap"] = ESP.getFreeHeap();
  root["chipid"] = String(ESP.getChipId(), HEX);
  root["cpu"] = ESP.getCpuFreqMHz();
  root["availsize"] = ESP.getFreeSketchSpace();
  root["availspiffs"] = fsinfo.totalBytes - fsinfo.usedBytes;
  root["spiffssize"] = fsinfo.totalBytes;
  wifi_get_ip_info(STATION_IF, &info);
  struct station_config conf;
  wifi_station_get_config(&conf);
  root["ssid"] = String(reinterpret_cast<char*>(conf.ssid));
  root["dns"] = printIP(WiFi.dnsIP());
  root["mac"] = WiFi.macAddress();
  IPAddress ipaddr = IPAddress(info.ip.addr);
  IPAddress gwaddr = IPAddress(info.gw.addr);
  IPAddress nmaddr = IPAddress(info.netmask.addr);
  root["ip"] = printIP(ipaddr);
  root["gateway"] = printIP(gwaddr);
  root["netmask"] = printIP(nmaddr);
  String json;
  root.printTo(json);
  DEBUG_PRINTLN(json);
  server.setContentLength(root.measureLength());
  server.send(200, "application/json", json);
}

/*
 * send all states
 * ----------------------------------------------------------------------------
 */
void sendAll() {
  DEBUG_PRINTLN("sendAll()");
  // try to open config.json file
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    DEBUG_PRINTLN(F("[WARN] Failed to open config file"));
  }
  size_t size = configFile.size();
  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);
  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());
  if (!json.success()) {
    DEBUG_PRINTLN(F("[WARN] Failed to parse config file"));
  }
  DynamicJsonBuffer jsonBuffer2;
  JsonObject &root = jsonBuffer2.createObject();
  // create relay json objects
  for (uint8_t idx = 0; idx < relayCount; ++idx) {
    JsonObject& relayJson = jsonBuffer2.createObject();
    relayJson["name"] = json["relay"+String(idx+1)]["name"];
    relayJson["state"] = relay[idx].state;
    root.set("relay"+String(idx+1), relayJson);
  }
  String jsonStr;
  root.printTo(jsonStr);
  DEBUG_PRINTLN(jsonStr);
  server.setContentLength(root.measureLength());
  server.send(200, "application/json", jsonStr);
}

/*
 * send current states
 * ----------------------------------------------------------------------------
 */
void sendCurrentStates() {
  DEBUG_PRINTLN("sendCurrentStates()");
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  // create relay json values
  for (uint8_t idx = 0; idx < relayCount; ++idx) {
    JsonObject& relayJson = jsonBuffer.createObject();
    relayJson["state"] = relay[idx].state;
    root.set("relay"+String(idx+1), relayJson);
  }
  String jsonStr;
  root.printTo(jsonStr);
  DEBUG_PRINTLN(jsonStr);
  server.setContentLength(root.measureLength());
  server.send(200, "application/json", jsonStr);
}

/*
 * set current relay state and store it in EEPROM
 * ----------------------------------------------------------------------------
 */
void setRelay(uint8_t idx, uint8_t value) {
  DEBUG_PRINTF("setRelay(%d, %d)\n", idx, value);
  uint8_t lowValue  = (relay[idx].type == 0 ? HIGH : LOW);
  uint8_t highValue = (relay[idx].type == 0 ? LOW  : HIGH);
  relay[idx].state = (value == 0 ? LOW : HIGH);
  digitalWrite(relay[idx].pin, (relay[idx].state == LOW ? lowValue : highValue));
  EEPROM.write(idx, relay[idx].state);
  EEPROM.commit();
}

/*
 * send relay state back to website (idx = index of relay)
 * ----------------------------------------------------------------------------
 */
void sendRelay(uint8_t idx) {
  DEBUG_PRINTF("sendRelay(%d)\n", idx);
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  root["relay"+String(idx+1)] = relay[idx].state;
  String json;
  root.printTo(json);
  DEBUG_PRINTLN(json);
  server.setContentLength(root.measureLength());
  server.send(200, "application/json", json);
}

