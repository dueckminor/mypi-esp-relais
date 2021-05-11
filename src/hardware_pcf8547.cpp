#include "hardware_pcf8574.h"
#include <PCF8574.h>

static PCF8574 thePCF8574(0x20);

void PCF8574_Init() {
    static bool bInitialized = false;
    if (!bInitialized)
    {
        thePCF8574.begin();
    }
    if (thePCF8574.isConnected()) {
        Serial.println("PCF8574: connected");
    } else {
        Serial.println("PCF8574: not connected");
    }
}

static uint8_t oldData = 0;

void PCF8574_Write(uint8_t data)
{
    if (data == oldData) 
    {
        return;
    }
    thePCF8574.write8(data);
    int rc = thePCF8574.lastError();
    if (rc != 0)
    {
        Serial.printf("PCF8574: error %d\n",rc);
    }
    else
    {
        oldData = data;
    }

}
