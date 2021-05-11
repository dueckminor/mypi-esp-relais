#include "wifi.h"
#include "settings.h"
#include "debug.h"
#include "timer.h"
#include "hardware.h"

WiFiClientSecure espClient;

String ssid;
static uint8_t BSSID[6];
static String BSSIDstr;
static bool bHaveBSSID=false;
static bool bHaveChangedStatus = false;

void WifiOnComplete(int nNetworks);

bool WifiHavePSK()
{
  return WiFi.psk().length() > 0;
}

void WifiInitialize()
{
  WiFi.mode(WIFI_STA);

  String hostname("esp-");
  hostname += WiFi.macAddress();
  hostname.replace(":","");

  WiFi.hostname(hostname);

  DebugPuts("WifiInitialize");
  DebugPuts(WiFi.SSID().c_str());

  static BearSSL::X509List trust((const uint8_t *)MYPI_CA_CERT, sizeof(MYPI_CA_CERT));
  espClient.setTrustAnchors(&trust);
  ssid = WiFi.SSID();
  espClient.setInsecure();

  if (WifiHavePSK()) {
    WiFi.scanNetworksAsync(&WifiOnComplete);
  }

  int cWPS = 0;
  while (!WifiSetupLoop() || cWPS) {
    TimerLoop();
    delay(100);
    if (HardwareReadWPS()) {
      cWPS++;
    } else {
      cWPS = 0;
    }

    if (cWPS == 30) {
      HardwareLED(true);
      WifiInitializeWPS();
      cWPS = 0;
    }
  }

  HardwareReadWPSDone();
}

void WifiInitializeWPS() {
  DebugPuts("WPS started");
  if (WiFi.beginWPSConfig()) {
    if(WiFi.psk().length() > 0) {
      DebugPuts("WPS succedded");
      DebugPuts(WiFi.SSID().c_str());
    } else {
      DebugPuts("WPS failed (timeout)");
    }
  } else {
    DebugPuts("WPS failed");
  }
  ssid = WiFi.SSID();
  if (WifiHavePSK()) {
    WiFi.scanNetworksAsync(&WifiOnComplete);
  }
}

bool WifiSetupLoop() {
  if (WifiIsConnected()) {
    return true;
  }
  if (WiFi.SSID().length()==0) {
    return false;
  }
  return WifiLoop();
}

bool WifiIsConnected()
{
  return WiFi.status() == WL_CONNECTED;
}

void WifiOnComplete(int nNetworks)
{
  Serial.print("nNetworks:");
  Serial.println(nNetworks);

  int32_t rssi = -1000;
  int iBestNetwork = -1;
  String WifiReport = "WiFi scan results:\n";
  char dump[100];

  for (int i = 0; i < nNetworks; i++)
  {
    int32_t rssiThis = WiFi.RSSI(i);

    sprintf(dump, "- %s (%s) %d dBm\n", WiFi.BSSIDstr(i).c_str(), WiFi.SSID(i).c_str(), WiFi.RSSI(i));
    WifiReport += dump;

    if (WiFi.SSID(i) != ssid)
    {
      continue;
    }

    if ((rssiThis > rssi) || (rssi > 0))
    {
      rssi = rssiThis;
      iBestNetwork = i;
    }
  }

  String bestBSSID = WiFi.BSSIDstr(iBestNetwork);
  String usedBSSID = WiFi.BSSIDstr();

  WifiReport += "Using:\n";
  sprintf(dump, "- %s (%s) %d dBm\n", bestBSSID.c_str(), WiFi.SSID(iBestNetwork).c_str(), rssi);
  WifiReport += dump;

  memcpy(BSSID, WiFi.BSSID(iBestNetwork),sizeof(BSSID));
  BSSIDstr = bestBSSID;

  DebugPuts(WifiReport.c_str());

  bHaveBSSID = true;
}
static bool WifiSwitchIfPossible()
{
  if (!bHaveBSSID)
  {
    return false;
  }

  bHaveChangedStatus = true;
  Serial.printf("WiFi.begin: %s (%s)\n",WiFi.SSID().c_str(),BSSIDstr.c_str());
  WiFi.begin(WiFi.SSID(), WiFi.psk(), 0, BSSID);
  bHaveBSSID = false;
  return true;
}


bool WifiLoop()
{
  wl_status_t wifiStatus = WiFi.status();
  static Timer timer(1000);
  bool bTick = timer.Tick();

  if (WifiSwitchIfPossible())
  {
    return false;
  }

  #define REPORT_FREQENCY 10
  static int cSecondsNotConnected = 0;
  static int cSecondsBadState = 0;
  static int cSecondsNoReport = REPORT_FREQENCY;

  if (wifiStatus != WL_CONNECTED)
  {
    cSecondsNotConnected += bTick;
    if (cSecondsNotConnected > 20 && (WiFi.scanComplete() != WIFI_SCAN_RUNNING)) {
        DebugPuts("scanning...");
        WiFi.scanNetworksAsync(&WifiOnComplete);
        cSecondsNotConnected = 0;
    }
    return false;
  }

  cSecondsNotConnected = 0;
  cSecondsNoReport += bTick;
  long rssi = WiFi.RSSI();
  if (rssi > 0)
  {
    cSecondsBadState += bTick;
  }

  if (cSecondsBadState > 10)
  {
    DebugPuts("disconnect because of status 31");
    WiFi.disconnect();
    cSecondsBadState = 0;
    return false;
  }

  if (bHaveChangedStatus || cSecondsNoReport>=REPORT_FREQENCY)
  {
    cSecondsNoReport = 0;
    DebugPrintf("{\"rssi\":%i,\"ssid\":\"%s\",\"bssid\":\"%s\"}",(int)rssi,WiFi.SSID().c_str(),WiFi.BSSIDstr().c_str());
    if (rssi <= -90)
    {
        if (WiFi.scanComplete() != WIFI_SCAN_RUNNING) {
          DebugPuts("scanning...");
          WiFi.scanNetworksAsync(&WifiOnComplete);
        }
    }
    bHaveChangedStatus = false;
  }

  // if (espClient.connected())
  // {
  //   cSecondsNoTLS=0;
  // }
  // else
  // {
  //   cSecondsNoTLS++;
  //   DebugPuts("no tls connection!");
  //   if (cSecondsNoTLS>10)
  //   {
  //     ESP.restart();
  //   }
  // }

  return true;
}

WiFiClient &WifiGetClient()
{
  return espClient;
}