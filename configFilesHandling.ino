bool loadSystemConfig() {
  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/"+configJsonFile)) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/"+configJsonFile, "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(ccuip,   json["ccuip"]);
          strcpy(ip,      json["ip"]);
          strcpy(netmask, json["netmask"]);
          strcpy(gw,      json["gw"]);
          strcpy(ise_LEVEL,      json["ise_level"]);
          strcpy(ise_COLOR,      json["ise_color"]);
          isRGBavailable = (String(ise_COLOR).length() > 1);
        } else {
          Serial.println("failed to load json config");
        }
      }
      return true;
    } else {
      Serial.println("/"+configJsonFile+" not found.");
      return false;
    }
    SPIFFS.end();
  } else {
    Serial.println("failed to mount FS");
    return false;
  }
}
