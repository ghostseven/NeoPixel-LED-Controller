#ifndef CONNECTION_H
#define CONNECTIONG_H

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <config.h>

extern WiFiClient wclient;
extern PubSubClient MQTTclient;

class CONNECTION{
    public:
        void begin(config &cfg, void (*pCallbackMQTT)(char* p_topic, byte* p_payload, unsigned int p_length));
        void loop();
        
    private:
        void setupWiFi();
        void setupOTA();
        void setupMQTT(void (*pCallbackMQTT)(char* p_topic, byte* p_payload, unsigned int p_length));
        void connectMQTT();
        config * _cfg;
};

#endif