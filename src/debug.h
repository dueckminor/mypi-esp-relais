#ifndef __MYPI_ESP_GATE_DEBUG_H__
#define __MYPI_ESP_GATE_DEBUG_H__

extern void DebugPuts(const char * msg);
extern void DebugPrintf(const char * format, ...);
extern bool DebugEnabled();
extern void DebugLoop();

#endif/*__MYPI_ESP_GATE_DEBUG_H__*/
