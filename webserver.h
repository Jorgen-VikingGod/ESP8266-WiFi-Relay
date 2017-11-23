/*
 * webserver.h
 * ----------------------------------------------------------------------------
 * webserver handles
 * ----------------------------------------------------------------------------
 * Source:     https://github.com/Jorgen-VikingGod/ESP8266-WiFi-Relay
 * Copyright:  Copyright (c) 2017 Juergen Skrotzky
 * Author:     Juergen Skrotzky <JorgenVikingGod@gmail.com>
 * License:    MIT License
 * Created on: 23.Nov. 2017
 * ----------------------------------------------------------------------------
 */

#ifndef _WEBSERVER_H
#define _WEBSERVER_H

#include "helper.h"

// declare and initial upload file container and web server
File fsUploadFile;
ESP8266WebServer server = ESP8266WebServer(80);

/*
 * forward declare functions
 * ----------------------------------------------------------------------------
 */
void sendAll();
void sendCurrentStates();
void setRelay(uint8_t idx, uint8_t value);
void sendRelay(uint8_t idx);
void sendStatus();
void sendConfigfile();

/*
 * format bytes
 * ----------------------------------------------------------------------------
 */
String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}

/*
 * get content type by file extension
 * ----------------------------------------------------------------------------
 */
String getContentType(String filename) {
  if (server.hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".ttf")) return "application/font-ttf";
  else if (filename.endsWith(".woff")) return "application/x-font-woff";
  else if (filename.endsWith(".woff2")) return "application/font-woff2";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

/*
 * handle file read by given path
 * ----------------------------------------------------------------------------
 */
bool handleFileRead(String path) {
  DEBUG_PRINTLN("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.htm";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

/*
 * get directory: GET http://wifi-relay.local/list
 * ----------------------------------------------------------------------------
 */
void handleFileList() {
  if (!server.hasArg("dir")) {
    server.send(500, "text/plain", "BAD ARGS");
    return;
  }
  String path = server.arg("dir");
  DEBUG_PRINTLN("handleFileList: " + path);
  Dir dir = SPIFFS.openDir(path);
  path = String();
  String output = "[";
  while (dir.next()) {
    File entry = dir.openFile("r");
    if (output != "[") output += ',';
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir) ? "dir" : "file";
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }
  output += "]";
  server.send(200, "text/json", output);
}

/*
 * get editor: GET http://wifi-relay.local/edit
 * ----------------------------------------------------------------------------
 */
void handleGetEditor() {
  if (!handleFileRead("/edit.htm"))
    server.send(404, "text/plain", "FileNotFound");
}

/*
 * create file: PUT http://wifi-relay.local/edit
 * ----------------------------------------------------------------------------
 */
void handleFileCreate() {
  if (server.args() == 0)
    return server.send(500, "text/plain", "BAD ARGS");
  String path = server.arg(0);
  DEBUG_PRINTLN("handleFileCreate: " + path);
  if (path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if (SPIFFS.exists(path))
    return server.send(500, "text/plain", "FILE EXISTS");
  File file = SPIFFS.open(path, "w");
  if (file)
    file.close();
  else
    return server.send(500, "text/plain", "CREATE FAILED");
  server.send(200, "text/plain", "");
  path = String();
}

/*
 * delete file: DELETE http://wifi-relay.local/edit
 * ----------------------------------------------------------------------------
 */
void handleFileDelete() {
  if (server.args() == 0) return server.send(500, "text/plain", "BAD ARGS");
  String path = server.arg(0);
  DEBUG_PRINTLN("handleFileDelete: " + path);
  if (path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if (!SPIFFS.exists(path))
    return server.send(404, "text/plain", "FileNotFound");
  SPIFFS.remove(path);
  server.send(200, "text/plain", "");
  path = String();
}

/*
 * upload file: POST http://wifi-relay.local/edit
 * ----------------------------------------------------------------------------
 */
void handleFileUpload() {
  if (server.uri() != "/edit") return;
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) filename = "/" + filename;
    DEBUG_PRINT("handleFileUpload Name: "); DEBUG_PRINTLN(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    //DEBUG_PRINT("handleFileUpload Data: "); DEBUG_PRINTLN(upload.currentSize);
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile)
      fsUploadFile.close();
    DEBUG_PRINT("handleFileUpload Size: "); DEBUG_PRINTLN(upload.totalSize);
  }
}

/*
 * get all relay states: GET http://wifi-relay.local/all
 * ----------------------------------------------------------------------------
 */
void handleGetAll() {
  sendAll();
}

/*
 * get current states: GET http://wifi-relay.local/current
 * ----------------------------------------------------------------------------
 */
void handleGetCurrentStates() {
  sendCurrentStates();
}

/*
 * get relay state: GET http://wifi-relay.local/relay?id=0
 * set relay state: GET http://wifi-relay.local/relay?id=0&value=0 or 1
 * ----------------------------------------------------------------------------
 */
void handleGetRelay() {
  uint8_t idx = server.arg("id").toInt() -1;
  if (server.hasArg("value")) {
    String value = server.arg("value");
    setRelay(idx, value.toInt());
    server.sendHeader("Location", String("/"), true);
    server.send(302, "text/plain", "");
    return;
  } else {
    sendRelay(idx);
  }
}

/*
 * set toggle state: POST http://wifi-relay.local/toggle with value=0 or 1
 * ----------------------------------------------------------------------------
 */
void handlePostToggle() {
  String data = server.arg("plain");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(data);
  String cmd = json["cmd"];
  String id = json["id"];
  String value = json["value"];
  if (cmd == "relay") {
    uint8_t idx = id.substring(5).toInt() -1;
    setRelay(idx, value.toInt());
    sendRelay(idx);
  }
}

/*
 * get status: GET http://wifi-relay.local/settings/status
 * ----------------------------------------------------------------------------
 */
void handleGetStatus() {
  sendStatus();
}

/*
 * get config: GET http://wifi-relay.local/settings/configfile
 * ----------------------------------------------------------------------------
 */
void handleGetConfigfile() {
  sendConfigfile();
}

/*
 * upload config.json: POST http://wifi-relay.local/settings/configfile
 * ----------------------------------------------------------------------------
 */
void handlePostConfigfile() {
  String data = server.arg("plain");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(data);
  String cmd = json["command"];
  DEBUG_PRINT("handlePostConfigfile: ");
  DEBUG_PRINTLN(cmd);
  if (cmd == "configfile") {
    server.send(200, "text/plain", "ok");
    File f = SPIFFS.open("/config.json", "w+");
    if (f) {
      json.prettyPrintTo(f);
      f.close();
      delay(1000);
      //reset and try again, or maybe put it to deep sleep
      ESP.restart();
      delay(1000);
    } else {
      DEBUG_PRINTLN("could not open config.json");
      server.send(404, "text/plain", "FileNotFound");
    }
  } else {
    DEBUG_PRINTLN("no valid configfile");
    server.send(404, "text/plain", "FileNotFound");
  }
}

#endif //_WEBSERVER_H
