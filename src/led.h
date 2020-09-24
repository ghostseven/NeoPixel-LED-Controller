#ifndef LED_H
#define LED_H

#include <math.h>
#include <FastLED.h>
#include <PubSubClient.h>
#include <config.h>

/*DESK 73 - 3D Printer 18 - TV  95*/
/*SML Candle Strip 12 - LRG Candle 16*/ 
/*LRG Candle V2 6*/
/*Fireplace 22*/
/*Basement Stairs 96*/

#define NUM_LEDS 73
#define LED_PIN D5
#define LED_STRIP_VOLTAGE 5
#define LED_STRIP_MILLIAMPS 4000 

extern CRGB leds[NUM_LEDS];

class HSV{
    public:
        HSV();
        HSV(int H, int S, int V, boolean isMQTT = true);
        HSV(String HSVStr, boolean isMQTT = true);
        int h;
        int s;
        int v;
        int _h;
        int _s;
        int _v;
        private:
            void convertFromMQTT();
            void convertToMQTT();
};

class LED{
    public: 
        LED();
        void begin(PubSubClient &MQTTClient, config &cfg);
        void loop();

        void setAll(byte red, byte green, byte blue);
        void setAllHSV(HSV colour);
        void setAllHSV();
        void setBlank();
        void showStrip();
        void setEffect(int e_index, int e_seconds);
        void exitEffect();

        int getEffect();
        HSV getHSV();
    private:

        void setPixel(int Pixel, byte red, byte green, byte blue);
        void setPixel(int Pixel, HSV colour);
        void publish();
        void runEffect();
        void endEffect();
        
        void breathing(uint16_t interval);
        void RGBLoop();
        void RunningLights(byte red, byte green, byte blue, int WaveDelay);
        void colorWipe(HSV colour, int SpeedDelay);
        byte * Wheel(byte WheelPos);
        void rainbowCycle(int SpeedDelay);
        void theaterChase(HSV colour, int SpeedDelay);
        void theaterChaseRainbow(int SpeedDelay);
        void Fire(int Cooling, int Sparking, int SpeedDelay);
        void setPixelHeatColor (int Pixel, byte temperature);
        void fadeToBlack(int ledNo, byte fadeValue);
        void meteorRain(byte red, byte green, byte blue, byte meteorSize, byte meteorTrailDecay, boolean meteorRandomDecay, int SpeedDelay);
        void CylonBounce(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay);
        void Twinkle(HSV colour, int Count, int SpeedDelay, boolean OnlyOne);
        void TwinkleRandom(int Count, int SpeedDelay, boolean OnlyOne);
        void Sparkle(HSV colour, int SpeedDelay);
        void SnowSparkle(HSV colour, int SparkleDelay, int SpeedDelay);
        
        HSV _colour;
        HSV _pre_effect_colour;
        int _effect_index;
        unsigned int _effect_timeout;
        int _next_effect_index;
        unsigned int _next_effect_timeout;        
        boolean _effect_takes_colour = false;
        boolean _exit_effect = false;
        PubSubClient * _MQTTClient;
        config * _cfg;
};

#endif  