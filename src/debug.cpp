#include "Arduino.h"
#include "debug.h"
#include "mqtt.h"
#include "settings.h"
#include "hardware.h"
#include "wifi.h"

#define MAX_LINES 64
static String aszLines[MAX_LINES];
static int cLines=0;
static uint8_t iLineRead=0;
static uint8_t iLineWrite=0;

#include "syslog.h"
Syslog logger("gadget", "testapp", "mypi.fritz.box");

static void debugAddLine(const char * msg)
{
    HardwareSerialOn();
    Serial.println(msg);

    if (cLines==MAX_LINES) {
        iLineRead++;
        iLineRead %= MAX_LINES;
        cLines--;
    }

    aszLines[iLineWrite] = msg;

    iLineWrite++;
    iLineWrite %= MAX_LINES;
    cLines++;
}

void DebugPrintf(const char * format, ...) {
    if (!DebugEnabled())
    {
        return;
    }

    va_list arg;
    va_start(arg, format);

    char buffer[256];
    int n = vsnprintf(buffer,sizeof(buffer)-1,format,arg);
    buffer[n]='\0';
    DebugPuts(buffer);
}

void DebugPuts(const char * msg)
{
    if (!msg)
    {
        return;
    }
    if (DebugEnabled())
    {
        char szLine[81];
        int cchLine=0;

        for (;*msg;)
        {
            char ch = *msg++;
            szLine[cchLine++]=ch;
            szLine[cchLine]='\0';
            if ((ch == '\n') || cchLine==80)
            {
                debugAddLine(szLine);
                cchLine = 0;
            }
        }
        if (cchLine) {
            if (szLine[cchLine-1]!='\n') {
                szLine[cchLine]='\n';
                szLine[cchLine+1]='\0';
            }
            debugAddLine(szLine);
        }
    }
}

void DebugLoop() {
    if (!cLines ||!WifiIsConnected())
    {
        return;
    }
    static unsigned long millisLastSend = 0;
    unsigned long millisNow = millis();
    if ((millisNow-millisLastSend) < 5)
    {
        return;
    }

    millisLastSend = millisNow;
    logger.debug(aszLines[iLineRead].c_str());
    aszLines[iLineRead].clear();
    iLineRead++;
    iLineRead %= MAX_LINES;
    cLines--;
}

bool DebugEnabled()
{
    return true;
}