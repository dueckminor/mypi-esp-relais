#ifndef SYSLOG_H
#define SYSLOG_H

#include <WiFiUdp.h>

#define MAX_PACKET_SIZE 500
// <PRI> in syslog is defined as (facility << 3) + severity
// We always use facility as 1 (which means "user"), 1<<3 = 8
#define PRI_DEBUG 15 // 8 + 7
#define PRI_INFO  14 // 8 + 6
#define PRI_WARNING 12 // 8 + 4
#define PRI_ERROR 11 // 8 + 3
class Syslog {
public:
  Syslog(const char *host, const char *app, const char *server, uint16_t port=514):
                  _host(host),
                  _app(app),
                  _server(server),
                  _port(port)
                  {}
  void debug(const char *message) { dolog(PRI_DEBUG, message); }
  void info(const char *message) { dolog(PRI_INFO, message); }
  void warn(const char *message) {dolog(PRI_WARNING, message); }
  void error(const char *message) {dolog(PRI_ERROR, message); }
private:
  const char *_host;
  const char *_app;
  const char *_server;
  const int _port;
  void dolog(uint8_t pri, const char *message);
  WiFiUDP udp;
};
#endif
