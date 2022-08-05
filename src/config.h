#ifndef CONFIG_H
#define CONFIG_H

#include <ArduinoJson.h>
#include <LittleFS.h>

class config_effect_sw
{
    public:
        int effect_index;
        int effect_seconds;
};
class config_wifi
{
    public:
        char ssid[64];
        char password[64];
};
class config_mqtt
{
    public:
        char host[64];
        int port;
        char base_topic[30];
        char full_topic[52];
        char username[64];
        char password[64];
        bool retained;
};
class config
{
    public:
        char name[26];
        char device_id[26];
        config_wifi wifi;
        config_mqtt mqtt;
        config_effect_sw effect_sw;
        boolean loaded = false;
        config();
    private:
        bool loadConfig();
};
#endif