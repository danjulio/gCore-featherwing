/*
  Created by Fabrizio Di Vittorio (fdivitto2013@gmail.com) - www.fabgl.com
  Copyright (c) 2019-2020 Fabrizio Di Vittorio.
  All rights reserved.

  This file is part of FabGL Library.

  FabGL is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  FabGL is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with FabGL.  If not, see <http://www.gnu.org/licenses/>.
 */


/*
 * Ported to gCore by Dan Julio
 *   Press and hold power button for > 2 second, then release to power off
 *   
 * Compile for ESP WROVER Module
 *    Flash Mode: "DIO"
 *    Flash Frequency: "40 MHz"
 *    Partition Scheme: Default 4MB (could be others depending on application requirements)
 * 
 * Note: The Arduino ESP32 experimental library must be used instead of the released
 * package at the time this module was written in April 2020 (release version 1.0.4
 * does not include the analogReadMilliVolts function).  See instructions for installation
 * of the development repository at https://github.com/espressif/arduino-esp32
 *   
 * TFT Display signals:
 *   SCK  => GPIO 5
 *   MOSI => GPIO 18
 *   CS   => GPIO 15
 *   D/C  => GPIO 33
 *   RESX => UNUSED
 */


#include "fabgl.h"

#include <WiFi.h>

#include "ICMP.h"


#define TFT_SCK    5
#define TFT_MOSI   18
#define TFT_CS     15
#define TFT_DC     33
#define TFT_RESET  GPIO_UNUSED
#define TFT_SPIBUS VSPI_HOST

#define TS_CS      32
#define SD_CS      14

#define KBD_DATA   GPIO_NUM_13
#define KBD_CLK    GPIO_NUM_27



char const * AUTOEXEC = "info\r"
                        "scan\r";


enum class State { Prompt, PromptInput, UnknownCommand, Help, Info, Wifi, TelnetInit, Telnet, Scan, Ping, Reset, Off };


State        state = State::Prompt;
WiFiClient   client;
char const * currentScript = nullptr;
bool         error = false;


fabgl::HX8357DController DisplayController;
fabgl::PS2Controller PS2Controller;
fabgl::Keyboard Keyboard;
fabgl::Terminal      Terminal;
fabgl::LineEditor    LineEditor(&Terminal);


void exe_info()
{
  Terminal.write("\e[37m* * FabGL - Network VT/ANSI Terminal\r\n");
  Terminal.write("\e[34m* * 2019-2020 by Fabrizio Di Vittorio - www.fabgl.com\e[32m\r\n\n");
  Terminal.printf("\e[32mScreen Size        :\e[33m %d x %d\r\n", DisplayController.getScreenWidth(), DisplayController.getScreenHeight());
  Terminal.printf("\e[32mTerminal Size      :\e[33m %d x %d\r\n", Terminal.getColumns(), Terminal.getRows());
  Terminal.printf("\e[32mKeyboard           :\e[33m %s\r\n", PS2Controller.keyboard()->isKeyboardAvailable() ? "OK" : "Error");
  Terminal.printf("\e[32mFree DMA Memory    :\e[33m %d\r\n", heap_caps_get_free_size(MALLOC_CAP_DMA));
  Terminal.printf("\e[32mFree 32 bit Memory :\e[33m %d\r\n", heap_caps_get_free_size(MALLOC_CAP_32BIT));
  if (WiFi.status() == WL_CONNECTED) {
    Terminal.printf("\e[32mWiFi SSID          :\e[33m %s\r\n", WiFi.SSID().c_str());
    Terminal.printf("\e[32mCurrent IP         :\e[33m %s\r\n", WiFi.localIP().toString().c_str());
  }
  Terminal.write("\n\e[32mType \e[33mhelp\e[32m to print all available commands.\r\n");
  error = false;
  state = State::Prompt;
}


void exe_help()
{
  Terminal.write("\e[33mhelp\e[92m\r\n");
  Terminal.write("\e[37m  Shows this help.\r\n");
  Terminal.write("\e[33minfo\r\n");
  Terminal.write("\e[37m  Shows system info.\r\n");
  Terminal.write("\e[33mscan\r\n");
  Terminal.write("\e[37m  Scan for WiFi networks.\r\n");
  Terminal.write("\e[33mwifi [SSID PASSWORD]\r\n");
  Terminal.write("\e[37m  Connect to SSID using PASSWORD.\r\n");
  Terminal.write("\e[37m  Example:\r\n");
  Terminal.write("\e[37m    wifi MyWifi MyPassword\r\n");
  Terminal.write("\e[33mtelnet HOST [PORT]\r\n");
  Terminal.write("\e[37m  Open telnet session with HOST (IP or host name) using PORT.\r\n");
  Terminal.write("\e[37m   CTRL-] to end.\r\n");
  Terminal.write("\e[37m  Example:\r\n");
  Terminal.write("\e[37m    telnet towel.blinkenlights.nl\e[32m\r\n");
  Terminal.write("\e[33mreboot\r\n");
  Terminal.write("\e[37m  Restart the system.\e[32m\r\n");
  Terminal.write("\e[33moff\r\n");
  Terminal.write("\e[37m  Switch the system off.\e[32m\r\n");
  error = false;
  state = State::Prompt;
}


void decode_command()
{
  auto inputLine = LineEditor.get();
  if (*inputLine == 0)
    state = State::Prompt;
  else if (strncmp(inputLine, "help", 4) == 0)
    state = State::Help;
  else if (strncmp(inputLine, "info", 4) == 0)
    state = State::Info;
  else if (strncmp(inputLine, "wifi", 4) == 0)
    state = State::Wifi;
  else if (strncmp(inputLine, "telnet", 6) == 0)
    state = State::TelnetInit;
  else if (strncmp(inputLine, "scan", 4) == 0)
    state = State::Scan;
  else if (strncmp(inputLine, "ping", 4) == 0)
    state = State::Ping;
  else if (strncmp(inputLine, "reboot", 4) == 0)
    state = State::Reset;
  else if (strncmp(inputLine, "off", 4) == 0)
    state = State::Off;
  else
    state = State::UnknownCommand;
}


void exe_prompt()
{
  if (currentScript) {
    // process commands from script
    if (*currentScript == 0 || error) {
      // end of script, return to prompt
      currentScript = nullptr;
      state = State::Prompt;
    } else {
      // execute current line and move to the next one
      int linelen = strchr(currentScript, '\r') - currentScript;
      LineEditor.setText(currentScript, linelen);
      currentScript += linelen + 1;
      decode_command();
    }
  } else {
    // process commands from keyboard
    Terminal.write(">");
    state = State::PromptInput;
  }
}


void exe_promptInput()
{
  LineEditor.setText("");
  LineEditor.edit();
  decode_command();
}


void exe_scan()
{
  static char const * ENC2STR[] = { "Open", "WEP", "WPA-PSK", "WPA2-PSK", "WPA/WPA2-PSK", "WPA-ENTERPRISE" };
  Terminal.write("Scanning...");
  Terminal.flush();
  fabgl::suspendInterrupts();
  int networksCount = WiFi.scanNetworks();
  fabgl::resumeInterrupts();
  Terminal.printf("%d network(s) found\r\n", networksCount);
  if (networksCount) {
    Terminal.write   ("\e[90m #\e[4GSSID\e[45GRSSI\e[55GCh\e[60GEncryption\e[32m\r\n");
    for (int i = 0; i < networksCount; ++i)
      Terminal.printf("\e[33m %d\e[4G%s\e[33m\e[45G%d dBm\e[55G%d\e[60G%s\e[32m\r\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i), WiFi.channel(i), ENC2STR[WiFi.encryptionType(i)]);
  }
  WiFi.scanDelete();
  error = false;
  state = State::Prompt;
}


void exe_wifi()
{
  static const int MAX_SSID_SIZE = 32;
  static const int MAX_PSW_SIZE  = 32;
  char ssid[MAX_SSID_SIZE + 1];
  char psw[MAX_PSW_SIZE + 1] = {0};
  error = true;
  auto inputLine = LineEditor.get();
  if (sscanf(inputLine, "wifi %32s %32s", ssid, psw) >= 1) {
    Terminal.write("Connecting WiFi...");
    Terminal.flush();
    AutoSuspendInterrupts autoInt;
    WiFi.disconnect(true, true);
    for (int i = 0; i < 2; ++i) {
      WiFi.begin(ssid, psw);
      if (WiFi.waitForConnectResult() == WL_CONNECTED)
        break;
      WiFi.disconnect(true, true);
    }
    if (WiFi.status() == WL_CONNECTED) {
      Terminal.printf("connected to %s, IP is %s\r\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
      error = false;
    } else {
      Terminal.write("failed!\r\n");
    }
  }
  state = State::Prompt;
}


void exe_telnetInit()
{
  static const int MAX_HOST_SIZE = 32;
  char host[MAX_HOST_SIZE + 1];
  int port;
  error = true;
  auto inputLine = LineEditor.get();
  int pCount = sscanf(inputLine, "telnet %32s %d", host, &port);
  if (pCount > 0) {
    if (pCount == 1)
      port = 23;
    Terminal.printf("Trying %s...\r\n", host);
    if (client.connect(host, port)) {
      Terminal.printf("Connected to %s\r\n", host);
      error = false;
      state = State::Telnet;
    } else {
      Terminal.write("Unable to connect to remote host\r\n");
      state = State::Prompt;
    }
  } else {
    Terminal.write("Mistake\r\n");
    state = State::Prompt;
  }
}


int clientWaitForChar()
{
  // not so good...:-)
  while (!client.available())
    ;
  return client.read();
}


void exe_telnet()
{
  // process data from remote host (up to 1024 codes at the time)
  for (int i = 0; client.available() && i < 1024; ++i) {
    int c = client.read();
    if (c == 0xFF) {
      // IAC (Interpret As Command)
      uint8_t cmd = clientWaitForChar();
      uint8_t opt = clientWaitForChar();
      if (cmd == 0xFD && opt == 0x1F) {
        // DO WINDOWSIZE
        client.write("\xFF\xFB\x1F", 3); // IAC WILL WINDOWSIZE
        client.write("\xFF\xFA\x1F" "\x00\x50\x00\x19" "\xFF\xF0", 9);  // IAC SB WINDOWSIZE 0 80 0 25 IAC SE
      } else if (cmd == 0xFD && opt == 0x18) {
        // DO TERMINALTYPE
        client.write("\xFF\xFB\x18", 3); // IAC WILL TERMINALTYPE
      } else if (cmd == 0xFA && opt == 0x18) {
        // SB TERMINALTYPE
        c = clientWaitForChar();  // bypass '1'
        c = clientWaitForChar();  // bypass IAC
        c = clientWaitForChar();  // bypass SE
        client.write("\xFF\xFA\x18\x00" "wsvt25" "\xFF\xF0", 12); // IAC SB TERMINALTYPE 0 "...." IAC SE
      } else {
        uint8_t pck[3] = {0xFF, 0, opt};
        if (cmd == 0xFD)  // DO -> WONT
          pck[1] = 0xFC;
        else if (cmd == 0xFB) // WILL -> DO
          pck[1] = 0xFD;
        client.write(pck, 3);
      }
    } else {
      Terminal.write(c);
    }
  }
  // process data from terminal (keyboard)
  while (Terminal.available()) {
    int c = Terminal.read();
    if (c == 0x1D) {
      // CTRL-]
      client.stop();
    } else {
      client.write( c );
    }
  }
  // return to prompt?
  if (!client.connected()) {
    client.stop();
    state = State::Prompt;
  }
}


void exe_ping()
{
  char host[64];
  auto inputLine = LineEditor.get();
  int pcount = sscanf(inputLine, "ping %s", host);
  if (pcount > 0) {
    int sent = 0, recv = 0;
    ICMP icmp;
    while (true) {

      // CTRL-C ?
      if (Terminal.available() && Terminal.read() == 0x03)
        break;

      int t = icmp.ping(host);
      if (t >= 0) {
        Terminal.printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\r\n", icmp.receivedBytes(), icmp.hostIP().toString().c_str(), icmp.receivedSeq(), icmp.receivedTTL(), (double)t/1000.0);
        delay(1000);
        ++recv;
      } else if (t == -2) {
        Terminal.printf("Cannot resolve %s: Unknown host\r\n", host);
        break;
      } else {
        Terminal.printf("Request timeout for icmp_seq %d\r\n", icmp.receivedSeq());
      }
      ++sent;

    }
    if (sent > 0) {
      Terminal.printf("--- %s ping statistics ---\r\n", host);
      Terminal.printf("%d packets transmitted, %d packets received, %.1f%% packet loss\r\n", sent, recv, (double)(sent - recv) / sent * 100.0);
    }
  }
  state = State::Prompt;
}


void setup()
{
  gcore_begin();   // First to hold power

  // Disable unused SPI CS
  pinMode(TS_CS, OUTPUT);
  digitalWrite(TS_CS, HIGH);
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  
  Serial.begin(115200); // DEBUG ONLY
  
  Keyboard.begin(KBD_CLK, KBD_DATA);
  
  DisplayController.begin(TFT_SCK, TFT_MOSI, TFT_DC, TFT_RESET, TFT_CS, TFT_SPIBUS);
  DisplayController.setResolution(TFT_320x480);
  DisplayController.setOrientation(fabgl::TFTOrientation::Rotate270);
  
  Terminal.begin(&DisplayController);
  Terminal.loadFont(&fabgl::FONT_6x10);
  Terminal.connectLocally();      // to use Terminal.read(), available(), etc..
  //Terminal.setLogStream(Serial);  // DEBUG ONLY
  
  Terminal.setBackgroundColor(Color::Black);
  Terminal.setForegroundColor(Color::BrightGreen);
  Terminal.clear();
  
  // just to align the screen
  //Terminal.write("1234567890123456789012345678901234567890123456789012345678901234567890123456789X\r\n");
  
  Terminal.enableCursor(true);

  currentScript = AUTOEXEC;
}


void loop()
{
  switch (state) {

    case State::Prompt:
      exe_prompt();
      break;

    case State::PromptInput:
      exe_promptInput();
      break;

    case State::Help:
      exe_help();
      break;

    case State::Info:
      exe_info();
      break;

    case State::Wifi:
      exe_wifi();
      break;

    case State::TelnetInit:
      exe_telnetInit();
      break;

    case State::Telnet:
      exe_telnet();
      break;

    case State::Scan:
      exe_scan();
      break;

    case State::Ping:
      exe_ping();
      break;

    case State::Reset:
      ESP.restart();
      break;

    case State::Off:
      gcore_power_down();
      break;

    case State::UnknownCommand:
      Terminal.write("\r\nMistake\r\n");
      state = State::Prompt;
      break;

    default:
      Terminal.write("\r\nNot Implemeted\r\n");
      state = State::Prompt;
      break;
  }
}
