#include <Wire.h>
#include <PCF8574.h>

void I2C_Init() {
    static bool bInitialized = false;
    if (!bInitialized)
    {
        Wire.begin();
        bInitialized = true;
    }
}
