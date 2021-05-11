#include <EEPROM.h>
#include "settings.h"
#include "hardware.h"
#include "hardware_pcf8574.h"
#include "wifi.h"
#include "mqtt.h"
#include "debug.h"
#include "timer.h"

#include <ArduinoOTA.h>

static Timer timers[8];
static bool  state[8]={false,false,false,false,false,false,false,false};

static void mqttCB(const char* topic, const byte* payload, unsigned int length) {
  if ((strncmp(topic,"relais/",7)==0) &&
      (topic[7]>='0') && (topic[7]<='7') &&
      (topic[8]=='\0'))
   {
     int iRelais = topic[7]-'0';
     if (length==1 && payload[0]=='1') {
       state[iRelais]=true;
     }
     else if (length==1 && payload[0]=='0') {
       state[iRelais]=false;
     }
     else if (length>1 && payload[length-1]=='s') {
       unsigned long timeout = 0;
       unsigned long factor = 1000;
       for (int i = 0;i<(length-1);i++) {
         if (payload[i]>='0' && payload[i]<='9') {
           timeout *= 10;
           timeout += payload[i]-'0';
         }
         else if ((i==(length-2)) && (payload[i]=='m')) {
           factor = 1;
         }
         else
         {
           return;
         }
       }
       state[iRelais]=true;
       timers[iRelais].SetTimeout(timeout*factor,true);
     }
   }
}

void setup()
{
  Serial.begin(74880);
  TimerSetBlinkPatternInit();
  HardwareInitialize();
  PCF8574_Init();
  PCF8574_Write(0xff);
  WifiInitialize();
  MqttSetCallback(mqttCB);

  ArduinoOTA.begin(true);

  MqttInitialize();
  DebugPuts("setup() completed");
}

void loop()
{
  TimerLoop();
  HardwareLoop();

  uint8_t data = 0xff;
  for (int i = 0; i < 8; i++)
  {
    if (timers[i].Tick())
    {
      state[i] = false;
    }
    if (state[i])
    {
      data -= (1<<i);
    }
  }

  PCF8574_Write(data);

  if (!WifiLoop())
  {
    TimerSetBlinkPatternSOS();
    delay(100);
    return;
  }

  ArduinoOTA.handle();

  TimerSetBlinkPatternOK();

  MqttBeginLoop();
  MqttEnsureConnected();

  DebugLoop();
}
