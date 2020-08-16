#include <connection.h>
WiFiClient wclient;
    PubSubClient MQTTclient(wclient);


void CONNECTION::begin(config &cfg, void (*pCallbackMQTT)(char* p_topic, byte* p_payload, unsigned int p_length)){
    _cfg = &cfg;
    setupWiFi();
    setupOTA();
    setupMQTT(pCallbackMQTT);
};

void CONNECTION::loop(){
    MDNS.update();
    ArduinoOTA.handle();

     if (!MQTTclient.connected())
     {
        connectMQTT();
     }
}

void CONNECTION::setupWiFi(){
    Serial.println("WiFi setup");
    WiFi.mode(WIFI_STA);
    WiFi.hostname(_cfg->device_id);
    WiFi.begin(_cfg->wifi.ssid, _cfg->wifi.password);
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("Connection Failed! Rebooting...");
        delay(5000);
        ESP.restart();
    }
    Serial.println("WiFi ready!");
    Serial.print("Connected to ");
    Serial.println(_cfg->wifi.ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void CONNECTION::setupOTA(){
    ArduinoOTA.setHostname(_cfg->device_id);

    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else { // U_FS
            type = "filesystem";
            LittleFS.end();
        }
        Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            Serial.println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
            Serial.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
            Serial.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            Serial.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
            Serial.println("End Failed");
        }
    });
    ArduinoOTA.begin(true);
}

void CONNECTION::setupMQTT(void (*pCallbackMQTT)(char* p_topic, byte* p_payload, unsigned int p_length)){
    MQTTclient.setServer(_cfg->mqtt.host, _cfg->mqtt.port);
    MQTTclient.setCallback(pCallbackMQTT);
}

void CONNECTION::connectMQTT() {
    // Loop until we're reconnected
    while (!MQTTclient.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (MQTTclient.connect(_cfg->device_id, _cfg->mqtt.username, _cfg->mqtt.password)) {
            char topic[64];
            strcpy(topic, _cfg->mqtt.base_topic);
            strcat(topic, _cfg->device_id);
            strcat(topic, "/#");
            MQTTclient.subscribe(topic);
            Serial.println("connected");
            // Once connected, return
        } else {
            Serial.print("failed, rc=");
            Serial.print(MQTTclient.state());
            Serial.println(" try again in 5 seconds");
            delay(5e3);
        }
    }
}