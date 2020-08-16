#include <config.h>

config::config(){
    Serial.println("Mounting FS...");
    if (!LittleFS.begin()) {
        Serial.println("Failed to mount file system");
    }
    if (!loadConfig()) {
        Serial.println("Failed to load config");
    } else {
        Serial.println("Config loaded");
    }    
}

bool config::loadConfig(){
    File configFile = LittleFS.open("/config.json", "r");
    if(!configFile){
        Serial.println("Failed to open config file");
        return false;
    }

    size_t size = configFile.size();
    if (size > 1024) {
        Serial.println("Config file size is too large");
        return false;
    }    
    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);
    configFile.readBytes(buf.get(), size);

    StaticJsonDocument<500> doc;
    auto error = deserializeJson(doc, buf.get());
    if (error) {
        Serial.println("Failed to parse config file");
        return false;
    }

    strcpy(name, doc["name"]);
    strcpy(device_id, doc["device_id"]);

    strcpy(wifi.ssid, doc["wifi"]["ssid"]);
    strcpy(wifi.password, doc["wifi"]["password"]);

    strcpy(mqtt.host, doc["mqtt"]["host"]);
    mqtt.port = doc["mqtt"]["port"];
    strcpy(mqtt.base_topic, doc["mqtt"]["base_topic"]);
    strcpy(mqtt.username, doc["mqtt"]["username"]);
    strcpy(mqtt.password, doc["mqtt"]["password"]);
    mqtt.retained = doc["mqtt"]["retained"];

    effect_sw.effect_index = doc["effect_sw"]["effect_index"];
    effect_sw.effect_seconds = doc["effect_sw"]["effect_seconds"];

    strcpy(mqtt.full_topic, mqtt.base_topic);
    strcat(mqtt.full_topic, device_id);
        
    return true;
}