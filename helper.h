/*
 * helper.h
 * ----------------------------------------------------------------------------
 * helper methods and defines
 * ----------------------------------------------------------------------------
 * Source:     https://github.com/Jorgen-VikingGod/ESP8266-WiFi-Relay
 * Copyright:  Copyright (c) 2017 Juergen Skrotzky
 * Author:     Juergen Skrotzky <JorgenVikingGod@gmail.com>
 * License:    MIT License
 * Created on: 23.Nov. 2017
 * ----------------------------------------------------------------------------
 */

#ifndef _HELPER_H
#define _HELPER_H

/*
 * Debug mode
 */
#define _debug 1

/*
 * relay container
 * ----------------------------------------------------------------------------
 */
struct sRelay {
  uint8_t pin;
  uint8_t type;
  uint8_t state;
  sRelay(uint8_t relayPin, uint8_t relayType = HIGH, uint8_t relayState = LOW) {
    pin = relayPin;
    type = relayType;
    state = relayState;
  }
};

/*
 * format ip address as String
 * ----------------------------------------------------------------------------
 */
String printIP(IPAddress adress) {
  return (String)adress[0] + "." + (String)adress[1] + "." + (String)adress[2] + "." + (String)adress[3];
}

/*
 * debug messages
 * ----------------------------------------------------------------------------
 */
template <typename... Args>
void DEBUG_PRINTF(const char *format, Args &&...args) {
  if (_debug) {
    Serial.printf(format, args...);
  }
}
template <typename Generic>
void DEBUG_PRINT(Generic text) {
  if (_debug) {
    Serial.print(text);
  }
}
template <typename Generic, typename Format>
void DEBUG_PRINT(Generic text, Format format) {
  if (_debug) {
    Serial.print(text, format);
  }
}
template <typename Generic>
void DEBUG_PRINTLN(Generic text) {
  if (_debug) {
    Serial.println(text);
  }
}
template <typename Generic, typename Format>
void DEBUG_PRINTLN(Generic text, Format format) {
  if (_debug) {
    Serial.println(text, format);
  }
}

#endif // _HELPER_H
