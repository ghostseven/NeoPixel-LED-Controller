#include <Arduino.h>
#include <config.h>
#include <connection.h>
#include <led.h>

config cfg;
CONNECTION con;
LED led;

unsigned long loadedTimeout;

void callbackMQTT(char* p_topic, byte* p_payload, unsigned int p_length) {
    // concat the payload into a string
    String payload;
    for (uint8_t i = 0; i < p_length; i++) {
        payload.concat((char)p_payload[i]);
    }
    //Debug payload data to serial console
    Serial.println(p_topic);
    Serial.println(payload);

    char _MQTT_TOPIC[100];

    //We are in restart mode, do we have any active previous states if so restore
    if(cfg.loaded == false){
        sprintf(_MQTT_TOPIC,"%s%s",cfg.mqtt.full_topic,"/hsv");
        if (strcmp(_MQTT_TOPIC,p_topic) == 0) {
            led.setAllHSV(HSV(payload));
        }

        sprintf(_MQTT_TOPIC,"%s%s",cfg.mqtt.full_topic,"/effectsw");
        if (strcmp(_MQTT_TOPIC,p_topic) == 0) {
            if(payload == "true"){
                led.setEffect(cfg.effect_sw.effect_index, cfg.effect_sw.effect_seconds, false);
            }else{
                led.exitEffect();
            }
        }         
    }

    sprintf(_MQTT_TOPIC,"%s%s",cfg.mqtt.full_topic,"/hsv/set");
    if (strcmp(_MQTT_TOPIC,p_topic) == 0) {
        led.setAllHSV(HSV(payload));
    }
    
    sprintf(_MQTT_TOPIC,"%s%s",cfg.mqtt.full_topic,"/effect");
    if (strcmp(_MQTT_TOPIC,p_topic) == 0) {
        StaticJsonDocument<500> doc;
        deserializeJson(doc, p_payload, p_length);
        int _ei = doc["effect_index"];
        int _es = doc["effect_seconds"]; //Will be 0 if no value set
        boolean _ef = doc["effect_restore"];
        led.setEffect(_ei,_es,_ef);
    }

    sprintf(_MQTT_TOPIC,"%s%s",cfg.mqtt.full_topic,"/effectsw/set");
    if (strcmp(_MQTT_TOPIC,p_topic) == 0) {
        if(payload == "true"){
            led.setEffect(cfg.effect_sw.effect_index, cfg.effect_sw.effect_seconds, false);
        }else{
            led.exitEffect();
         }
    }   
}

void setup() {
    loadedTimeout = millis() +  5000;
    Serial.begin(115200);
    con.begin(cfg,callbackMQTT);
    led.begin(MQTTclient, cfg);
    digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
    if(millis()>loadedTimeout){cfg.loaded = true;}
    con.loop();
    led.loop();
}