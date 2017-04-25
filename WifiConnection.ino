bool doWifiConnect() {
  WiFiManager wifiManager;
  wifiManager.setDebugOutput(wifiManagerDebugOutput);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  WiFiManagerParameter custom_ccuip("ccu", "IP der CCU2", "", 15);
  WiFiManagerParameter custom_ise_level("ise_level", "ise_id LEVEL", "", 10);
  WiFiManagerParameter custom_ise_color("ise_color", "ise_id COLOR", "", 10);
  WiFiManagerParameter custom_ip("custom_ip", "IP-Adresse", "", 15);
  WiFiManagerParameter custom_netmask("custom_netmask", "Netzmaske", "", 15);
  WiFiManagerParameter custom_gw("custom_gw", "Gateway", "", 15);
  WiFiManagerParameter custom_text("<br/><br>Statische IP (wenn leer, dann DHCP):");

  wifiManager.addParameter(&custom_ccuip);
  wifiManager.addParameter(&custom_ise_level);
  wifiManager.addParameter(&custom_ise_color);
  wifiManager.addParameter(&custom_text);
  wifiManager.addParameter(&custom_ip);
  wifiManager.addParameter(&custom_netmask);
  wifiManager.addParameter(&custom_gw);

  if (createNewSystemConfig == true) {
    wifiManager.resetSettings();
  }

  const char* ipStr = ip; byte ipBytes[4]; parseBytes(ipStr, '.', ipBytes, 4, 10);
  const char* netmaskStr = netmask; byte netmaskBytes[4]; parseBytes(netmaskStr, '.', netmaskBytes, 4, 10);
  const char* gwStr = gw; byte gwBytes[4]; parseBytes(gwStr, '.', gwBytes, 4, 10);

  wifiManager.setSTAStaticIPConfig(IPAddress(ipBytes[0], ipBytes[1], ipBytes[2], ipBytes[3]), IPAddress(gwBytes[0], gwBytes[1], gwBytes[2], gwBytes[3]), IPAddress(netmaskBytes[0], netmaskBytes[1], netmaskBytes[2], netmaskBytes[3]));

  wifiManager.autoConnect(Hostname);

  Serial.println("Wifi Connected");
  if (shouldSaveConfig) {
    SPIFFS.begin();
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    if (String(custom_ip.getValue()).length() > 5) {
      Serial.println("Custom IP Address is set!");
      strcpy(ip, custom_ip.getValue());
      strcpy(netmask, custom_netmask.getValue());
      strcpy(gw, custom_gw.getValue());

      
    } else {
      strcpy(ip,      "0.0.0.0");
      strcpy(netmask, "0.0.0.0");
      strcpy(gw,      "0.0.0.0");
    }
    strcpy(ccuip, custom_ccuip.getValue());
    strcpy(ise_LEVEL, custom_ise_level.getValue());
    strcpy(ise_COLOR, custom_ise_color.getValue());
    Serial.println("Schreibe in JSON:" + String(ccuip));
    json["ccuip"] = ccuip;
    json["ise_level"] = ise_LEVEL;
    json["ise_color"] = ise_COLOR;
    json["ip"] = ip;
    json["netmask"] = netmask;
    json["gw"] = gw;
    SPIFFS.remove("/" + configJsonFile);
    File configFile = SPIFFS.open("/" + configJsonFile, "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();

    SPIFFS.end();
    delay(100);
    ESP.restart();
  }

  startOTAhandling();

  return true;
}
void configModeCallback (WiFiManager *myWiFiManager) {
    myGLCD.print("AP-Mode!", 125, 20, 90);}

void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void parseBytes(const char* str, char sep, byte* bytes, int maxBytes, int base) {
  for (int i = 0; i < maxBytes; i++) {
    bytes[i] = strtoul(str, NULL, base);  // Convert byte
    str = strchr(str, sep);               // Find next separator
    if (str == NULL || *str == '\0') {
      break;                            // No more separators, exit
    }
    str++;                                // Point to next character after separator
  }
}
