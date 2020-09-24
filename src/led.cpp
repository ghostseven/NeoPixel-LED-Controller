#include <led.h>

CRGB leds[NUM_LEDS];

HSV::HSV(){
  _h = 0;
  _s = 0;
  _v = 0;
  h = 0;
  s = 0;
  v = 0;      
}

HSV::HSV(int H, int S, int V, boolean isMQTT){
    if(isMQTT){
      //Store unscalled HSV values from MQTTThing
      _h = H;
      _s = S;
      _v = V;
      //Convert to FastLED HSV values
      convertFromMQTT();
    }else{
      //Store unscalled HSV values from FastLED
      h = H;
      s = S;
      v = V;
      //Convert to MQTTThing HSV values
      convertToMQTT();      
    }  
}

HSV::HSV(String HSVStr, boolean isMQTT){
    int iHSV[3];
    unsigned int i = 0;
    while(HSVStr.indexOf(",")!=-1){
      // take the substring from the start to the first occurence of a comma, convert it to int and save it in the array
      iHSV[i] = HSVStr.substring(0,HSVStr.indexOf(",")).toInt();
      i++; // increment our data counter
      //cut the data string after the first occurence of a comma
      HSVStr = HSVStr.substring(HSVStr.indexOf(",")+1);
    } 
    // get the last value out of the string, which as no more commas in it
    iHSV[i] = HSVStr.toInt();

    if(isMQTT){
      //Store unscalled HSV values from MQTTThing
      _h = iHSV[0];
      _s = iHSV[1];
      _v = iHSV[2];
      //Convert to FastLED HSV values
      convertFromMQTT();
    }else{
      //Store unscalled HSV values from FastLED
      h = iHSV[0];
      s = iHSV[1];
      v = iHSV[2];
      //Convert to MQTTThing HSV values
      convertToMQTT();      
    }
}

void HSV::convertFromMQTT(){
    //Convert HSV ranges sent from MQTTThing H:0-360 S:0-100 V:0-100 to FastLED Ranges H:0-255 S:0-255 V:0-255 using scaling formula 
    //NewValue = (((OldValue - OldMin) * (NewMax - NewMin)) / (OldMax - OldMin)) + NewMin
    h = (((_h - 0) * (255 - 0)) / (360 - 0)) + 0;
    s = (((_s - 0) * (255 - 0)) / (100 - 0)) + 0;
    v = (((_v - 0) * (255 - 0)) / (100 - 0)) + 0;
}

void HSV::convertToMQTT(){
    //Convert HSV ranges sent from FastLED Ranges H:0-255 S:0-255 V:0-255 to MQTTThing Ranges H:0-360 S:0-100 V:0-100  using scaling formula 
    //NewValue = (((OldValue - OldMin) * (NewMax - NewMin)) / (OldMax - OldMin)) + NewMin
    _h = (((h - 0) * (360 - 0)) / (255 - 0)) + 0;
    _s = (((s - 0) * (100 - 0)) / (255 - 0)) + 0;
    _v = (((v - 0) * (100 - 0)) / (255 - 0)) + 0;  
}

LED::LED(){
    FastLED.setMaxPowerInVoltsAndMilliamps(LED_STRIP_VOLTAGE, LED_STRIP_MILLIAMPS);
    FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);     
}

void LED::begin(PubSubClient &MQTTClient, config &cfg){
    _MQTTClient = &MQTTClient;
    _cfg = &cfg;
    _effect_index = 0;
    _effect_timeout = 0;
}

void LED::publish(){
    if(_cfg->loaded){
      char c[12];
      char _MQTT_TOPIC[100];
      sprintf(c, "%d,%d,%d", _colour._h, _colour._s, _colour._v);
      sprintf(_MQTT_TOPIC,"%s%s",_cfg->mqtt.full_topic,"/hsv");
      _MQTTClient->publish(_MQTT_TOPIC, c, true);

      sprintf(_MQTT_TOPIC,"%s%s",_cfg->mqtt.full_topic,"/effectsw");
      if(_effect_index > 0){
          _MQTTClient->publish(_MQTT_TOPIC, "true", true);
      }else{
          _MQTTClient->publish(_MQTT_TOPIC, "false", true);
      }
          
      sprintf(_MQTT_TOPIC,"%s%s",_cfg->mqtt.full_topic,"/state");
      if(_colour._v > 0 || _effect_index >0){
          _MQTTClient->publish(_MQTT_TOPIC, "true", true);
      }else{
          _MQTTClient->publish(_MQTT_TOPIC, "false", true);
      }    
    }
}

void LED::setPixel(int Pixel, byte red, byte green, byte blue) {
   leds[Pixel].r = red;
   leds[Pixel].g = green;
   leds[Pixel].b = blue;
}

void LED::setPixel(int Pixel, HSV colour) {
   leds[Pixel].setHSV(colour.h,colour.s,colour.v);
}

// Set all LEDs to a given color and apply it (visible)
void LED::setAll(byte red, byte green, byte blue) {
  for(int i = 0; i < NUM_LEDS; i++ ) {
    setPixel(i, red, green, blue);
  }
  showStrip();
}

//set all leds to colour to a specific HSV and store as current
void LED::setAllHSV(HSV colour){ 
    _colour = colour;
    if(_effect_index>0 && _effect_takes_colour == false){_exit_effect = true;}
    fill_solid(leds, NUM_LEDS, CHSV(colour.h,colour.s,colour.v));
    showStrip();
    publish(); 
}

//set all leds to currently stored HSV
void LED::setAllHSV(){
    setAllHSV(_colour);
}

HSV LED::getHSV(){
  return _colour;
}

// Cancel any effects and set all pixels to nothing.
void LED::setBlank(){
  setAll(0,0,0);
}
// Apply LED color changes
void LED::showStrip() {
  FastLED.show();
  _MQTTClient->loop();
}

void LED::setEffect(int e_index, int e_seconds){
  //copy live colour object to pre-effect backup to restore after effect.
  _pre_effect_colour = _colour;
  // check we have a valid effect id, if not clear effect settings and carry on.
  if(e_index>0 && e_index <15){
    // restore birghtness as an effect may have scaled it.
    FastLED.setBrightness(255);
    if(_effect_index !=0){ //if we already have an effect running queue up the next effect and its runtime.
      _next_effect_index=e_index;
      _exit_effect = true; //set the boolean to let the rest of the code know we want to exit the current effect as quickly as we can.

      if(e_seconds == 0){_next_effect_timeout = UINT32_MAX;} else //if 0 set this to max unsigned int to indicate perpetual effect
      {
        _next_effect_timeout = millis() + (e_seconds*1000);
      }
    }else{
      _effect_index = e_index;
      if(e_seconds == 0){_effect_timeout = UINT32_MAX;} else //if 0 set this to max unsigned int to indicate perpetual effect
      {
        _effect_timeout = millis() + (e_seconds*1000);
      }
    }
    publish();
  }else{
    endEffect();
  }   
}

int LED::getEffect(){
  return _effect_index;
}

void LED::exitEffect(){
  _exit_effect = true;
}

void LED::runEffect(){
  switch(_effect_index){
  case 1  : {
              // RGBLoop - no parameters
              RGBLoop();
              break;
            }
  case 2: {
              // Running Lights - Color (red, green, blue), wave dealy
              RunningLights(0xff,0x00,0x00, 50);  // red
              RunningLights(0xff,0xff,0xff, 50);  // white
              RunningLights(0x00,0x00,0xff, 50);  // blue
              break;
            }
            
  case 3: {
              _effect_takes_colour = true;
              // colorWipe - Color HSV, speed delay
              //if we do not have a colour live, wipe green
              if(_colour.v <1){
                colorWipe(HSV(120,100,100), 70);
              }else{
                colorWipe(_colour, 70);
              }
              colorWipe(HSV(0,0,0), 70);
              break;
            }

  case 4 : {
              // rainbowCycle - speed delay
              rainbowCycle(20);
              break;
            }    

  case 5 : {
              _effect_takes_colour = true;
              // theatherChase - Color HSV, speed delay
              if(_colour.v <1){
                theaterChase(HSV(0,100,100),90);
              }else{
                theaterChase(_colour,90);
              }              
              break;
            }

  case 6 : {
              // theaterChaseRainbow - Speed delay
              theaterChaseRainbow(90);
              break;
            }

  case 7 : {
              // Fire - Cooling rate, Sparking rate, speed delay
              Fire(55,40,80);
              break;
            }
            
  case 8 : {
              // meteorRain - Color (red, green, blue), meteor size, trail decay, random trail decay (true/false), speed delay
              meteorRain(0xff,0xff,0xff,10, 64, true, 50);
              break;
            }

  case 9  : {
              // CylonBounce - Color (red, green, blue), eye size, speed delay, end pause
              CylonBounce(0xff, 0x00, 0x00, 4, 20, 50);
              break;
            }
              
  case 10  : {
              // Twinkle - Color HSV, count, speed delay, only one twinkle (true/false)
              Twinkle(HSV(0,100,100), 10, 100, false);
              break;
            }
            
  case 11  : {
              // TwinkleRandom - twinkle count, speed delay, only one (true/false)
              TwinkleRandom(20, 100, false);
              break;
            }
            
  case 12  : {
              // Sparkle - Color HSV, speed delay
              Sparkle(HSV(0,0,100), 100);
              break;
            }
              
  case 13  : {
              // SnowSparkle - HSV, sparkle delay, speed delay
              SnowSparkle(HSV(0,0,7), 20, random(100,1000));
              break;
            }     
  case 14  : {
              // Breathing - uses current HSV colour that has been set, if no colour it uses white.
              _effect_takes_colour = true;
              breathing(30);
              break;
            }            
  } 
}

void LED::endEffect(){
  _effect_takes_colour = false;
  _exit_effect = false;
  //restore colour object 
  _colour = _pre_effect_colour;
  if(_next_effect_index >0){
    _effect_index = _next_effect_index;
    _next_effect_index = 0;
    _effect_timeout = _next_effect_timeout;
    _next_effect_timeout = 0;
  }else{
    _effect_index = 0;
    _effect_timeout = 0;
    _next_effect_index = 0;
    _next_effect_timeout = 0;   
    if(_colour.v>0){ //we have a value above 0 so set back to stored HSV
      setAllHSV();
    }else{
      //set to off, 0 value and preserve hue and saturation.
      setAllHSV(HSV(_colour.h,_colour.s,0,false));
    }    
  }
}

void LED::loop(){
  if(_exit_effect){endEffect(); goto end;} 
  if(_effect_index > 0){
    if(_effect_timeout == UINT32_MAX){runEffect();} // If timeout set to MAX it is perpetual
    else{
      if(millis() > _effect_timeout){
        endEffect();
      }else{
        runEffect();
      }
    }
  }
  end:
  _MQTTClient->loop();
}

// *************************
// ** LEDEffect Functions **
// *************************

void LED::RGBLoop(){
  for(int j = 0; j < 3; j++ ) {
    // Fade IN
    for(int k = 0; k < 256; k++) {
      if(_exit_effect){goto end;} // goto to jump out of loop fast so we can switch effects
      switch(j) {
        case 0: setAll(k,0,0); break;
        case 1: setAll(0,k,0); break;
        case 2: setAll(0,0,k); break;
      }
      showStrip();
      FastLED.delay(3);
    }
    // Fade OUT
    for(int k = 255; k >= 0; k--) {
      if(_exit_effect){goto end;} // goto to jump out of loop fast so we can switch effects
      switch(j) {
        case 0: setAll(k,0,0); break;
        case 1: setAll(0,k,0); break;
        case 2: setAll(0,0,k); break;
      }
      showStrip();
      FastLED.delay(3);
    }
  }
  end:
  FastLED.delay(0);
}

void LED::RunningLights(byte red, byte green, byte blue, int WaveDelay) {
  int Position=0;
 
  for(int i=0; i<NUM_LEDS*2; i++)
  {
      Position++; // = 0; //Position + Rate;
      for(int i=0; i<NUM_LEDS; i++) {
        if(_exit_effect){goto end;} // goto to jump out of loop fast so we can switch effects
        // sine wave, 3 offset waves make a rainbow!
        //float level = sin(i+Position) * 127 + 128;
        //setPixel(i,level,0,0);
        //float level = sin(i+Position) * 127 + 128;
        setPixel(i,((sin(i+Position) * 127 + 128)/255)*red,
                   ((sin(i+Position) * 127 + 128)/255)*green,
                   ((sin(i+Position) * 127 + 128)/255)*blue);
      }
      showStrip();
      FastLED.delay(WaveDelay);
  }
  end:
  FastLED.delay(0);
}

void LED::breathing(uint16_t interval) {

    const uint8_t min_brightness = 2;
    static uint8_t delta = 255; // goes up to 255 then overflows back to 0

    static uint32_t pm = 0; // previous millis
    if ( (millis() - pm) > interval ) {
        pm = millis();

        // for the LEDs in the current state setting the brightness higher than max_brightness will not actually increase the brightness displayed
        uint8_t max_brightness = calculate_max_brightness_for_power_vmA(leds, NUM_LEDS, 255, LED_STRIP_VOLTAGE, LED_STRIP_MILLIAMPS);
        uint8_t b = scale8(triwave8(delta), max_brightness-min_brightness)+min_brightness;
        //if we do not have a colour live, set it to white
        if(_colour.v <1){fill_solid(leds, NUM_LEDS, CHSV(0,0,100));}
        FastLED.setBrightness(b);

        showStrip();
        delta++;
    }
}

// used by rainbowCycle and theaterChaseRainbow
byte * LED::Wheel(byte WheelPos) {
  static byte c[3];
 
  if(WheelPos < 85) {
   c[0]=WheelPos * 3;
   c[1]=255 - WheelPos * 3;
   c[2]=0;
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   c[0]=255 - WheelPos * 3;
   c[1]=0;
   c[2]=WheelPos * 3;
  } else {
   WheelPos -= 170;
   c[0]=0;
   c[1]=WheelPos * 3;
   c[2]=255 - WheelPos * 3;
  }

  return c;
}

void LED::rainbowCycle(int SpeedDelay) {
  byte *c;
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< NUM_LEDS; i++) {
      if(_exit_effect){goto end;} // goto to jump out of loop fast so we can switch effects
      c=Wheel(((i * 256 / NUM_LEDS) + j) & 255);
      setPixel(i, *c, *(c+1), *(c+2));
    }
    showStrip();
    FastLED.delay(SpeedDelay);
  }
  end:
  FastLED.delay(0);
}

void LED::colorWipe(HSV colour, int SpeedDelay) {
  for(uint16_t i=0; i<NUM_LEDS; i++) {
      if(_exit_effect){goto end;} // goto to jump out of loop fast so we can switch effects
      setPixel(i, colour);
      showStrip();
      FastLED.delay(SpeedDelay);
  }
  end:
  FastLED.delay(0);
}

void LED::theaterChase(HSV colour, int SpeedDelay) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < NUM_LEDS; i=i+3) {
        if(_exit_effect){goto end;} // goto to jump out of loop fast so we can switch effects
        setPixel(i+q, colour);    //turn every third pixel on
      }
      showStrip();
     
      FastLED.delay(SpeedDelay);
     
      for (int i=0; i < NUM_LEDS; i=i+3) {
        if(_exit_effect){goto end;} // goto to jump out of loop fast so we can switch effects
        setPixel(i+q, 0,0,0);        //turn every third pixel off
      }
    }
  }
  end:
  FastLED.delay(0);
}

void LED::theaterChaseRainbow(int SpeedDelay) {
  byte *c;
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
        for (int i=0; i < NUM_LEDS; i=i+3) {
          if(_exit_effect){goto end;} // goto to jump out of loop fast so we can switch effects
          c = Wheel( (i+j) % 255);
          setPixel(i+q, *c, *(c+1), *(c+2));    //turn every third pixel on
        }
        showStrip();
        FastLED.delay(SpeedDelay);
       
        for (int i=0; i < NUM_LEDS; i=i+3) {
          if(_exit_effect){goto end;} // goto to jump out of loop fast so we can switch effects
          setPixel(i+q, 0,0,0);        //turn every third pixel off
        }
    }
  }
  end:
  FastLED.delay(0);
}

void LED::setPixelHeatColor (int Pixel, byte temperature) {
  // Scale 'heat' down from 0-255 to 0-191
  byte t192 = round((temperature/255.0)*191);
 
  // calculate ramp up from
  byte heatramp = t192 & 0x3F; // 0..63
  heatramp <<= 2; // scale up to 0..252
 
  // figure out which third of the spectrum we're in:
  if( t192 > 0x80) {                     // hottest
    setPixel(Pixel, 255, 255, heatramp);
  } else if( t192 > 0x40 ) {             // middle
    setPixel(Pixel, 255, heatramp, 0);
  } else {                               // coolest
    setPixel(Pixel, heatramp, 0, 0);
  }
}

void LED::Fire(int Cooling, int Sparking, int SpeedDelay) {
  static byte heat[NUM_LEDS];
  int cooldown;
 
  // Step 1.  Cool down every cell a little
  for( int i = 0; i < NUM_LEDS; i++) {
    if(_exit_effect){goto end;} // goto to jump out of loop fast so we can switch effects
    cooldown = random(0, ((Cooling * 10) / NUM_LEDS) + 2);
   
    if(cooldown>heat[i]) {
      heat[i]=0;
    } else {
      heat[i]=heat[i]-cooldown;
    }
  }
 
  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for( int k= NUM_LEDS - 1; k >= 2; k--) {
    if(_exit_effect){goto end;} // goto to jump out of loop fast so we can switch effects
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }
   
  // Step 3.  Randomly ignite new 'sparks' near the bottom
  if( random(255) < Sparking ) {
    int y = random(7);
    heat[y] = heat[y] + random(160,255);
    //heat[y] = random(160,255);
  }

  // Step 4.  Convert heat to LED colors
  for( int j = 0; j < NUM_LEDS; j++) {
    if(_exit_effect){goto end;} // goto to jump out of loop fast so we can switch effects
    setPixelHeatColor(j, heat[j] );
  }

  showStrip();
  FastLED.delay(SpeedDelay);
  end:
  FastLED.delay(0);
}

void LED::fadeToBlack(int ledNo, byte fadeValue) {
   leds[ledNo].fadeToBlackBy( fadeValue );
}

void LED::meteorRain(byte red, byte green, byte blue, byte meteorSize, byte meteorTrailDecay, boolean meteorRandomDecay, int SpeedDelay) {  
  setAll(0,0,0);
 
  for(int i = 0; i < NUM_LEDS+NUM_LEDS; i++) {
   
   
    // fade brightness all LEDs one step
    for(int j=0; j<NUM_LEDS; j++) {
      if(_exit_effect){goto end;} // goto to jump out of loop fast so we can switch effects
      if( (!meteorRandomDecay) || (random(10)>5) ) {
        fadeToBlack(j, meteorTrailDecay );        
      }
    }
   
    // draw meteor
    for(int j = 0; j < meteorSize; j++) {
      if(_exit_effect){goto end;} // goto to jump out of loop fast so we can switch effects
      if( ( i-j <NUM_LEDS) && (i-j>=0) ) {
        setPixel(i-j, red, green, blue);
      }
    }
   
    showStrip();
    FastLED.delay(SpeedDelay);
  }
  end:
  FastLED.delay(0);
}

void LED::CylonBounce(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay){

  for(int i = 0; i < NUM_LEDS-EyeSize-2; i++) {
    setAll(0,0,0);
    setPixel(i, red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) {
      if(_exit_effect){goto end;} // goto to jump out of loop fast so we can switch effects
      setPixel(i+j, red, green, blue);
    }
    setPixel(i+EyeSize+1, red/10, green/10, blue/10);
    showStrip();
    FastLED.delay(SpeedDelay);
  }

  FastLED.delay(ReturnDelay);

  for(int i = NUM_LEDS-EyeSize-2; i > 0; i--) {
    setAll(0,0,0);
    setPixel(i, red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) {
      if(_exit_effect){goto end;} // goto to jump out of loop fast so we can switch effects
      setPixel(i+j, red, green, blue);
    }
    setPixel(i+EyeSize+1, red/10, green/10, blue/10);
    showStrip();
    FastLED.delay(SpeedDelay);
  }
 
  delay(ReturnDelay);
  end:
  FastLED.delay(0);
}

void LED::Twinkle(HSV colour, int Count, int SpeedDelay, boolean OnlyOne) {
  setAll(0,0,0);
 
  for (int i=0; i<Count; i++) {
    if(_exit_effect){goto end;} // goto to jump out of loop fast so we can switch effects
     setPixel(random(NUM_LEDS),colour);
     showStrip();
     FastLED.delay(SpeedDelay);
     if(OnlyOne) {
       setAll(0,0,0);
     }
   }
 
  FastLED.delay(SpeedDelay);
  end:
  FastLED.delay(0);
}

void LED::TwinkleRandom(int Count, int SpeedDelay, boolean OnlyOne) {
  setAll(0,0,0);
 
  for (int i=0; i<Count; i++) {
    if(_exit_effect){goto end;} // goto to jump out of loop fast so we can switch effects
     setPixel(random(NUM_LEDS),random(0,255),random(0,255),random(0,255));
     showStrip();
     FastLED.delay(SpeedDelay);
     if(OnlyOne) {
       setAll(0,0,0);
     }
   }
 
  FastLED.delay(SpeedDelay);
  end:
  FastLED.delay(0);
}

void LED::Sparkle(HSV colour, int SpeedDelay) {
  int Pixel = random(NUM_LEDS);
  setPixel(Pixel,colour);
  showStrip();
  FastLED.delay(SpeedDelay);
  setPixel(Pixel,0,0,0);
}

void LED::SnowSparkle(HSV colour, int SparkleDelay, int SpeedDelay) {
  setAllHSV(colour);
  int Pixel = random(NUM_LEDS);
  setPixel(Pixel,HSV(0,0,100));
  showStrip();
  FastLED.delay(SparkleDelay);
  setPixel(Pixel,colour);
  showStrip();
  FastLED.delay(SpeedDelay);
}