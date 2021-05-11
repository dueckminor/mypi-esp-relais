#ifndef __MYPI_ESP_GATE_TIMER_H__
#define __MYPI_ESP_GATE_TIMER_H__

#include <stdint.h>

extern unsigned long TimerLoop();
extern void TimerSetBlinkPattern(uint32_t blinkPattern);
extern void TimerSetBlinkPatternOK();
extern void TimerSetBlinkPatternSOS();
extern void TimerSetBlinkPatternInit();


class Timer
{
public:
    Timer(unsigned long timeout=0, bool once=false);
    void SetTimeout(unsigned long timeout, bool once=false);
    bool Tick();
protected:
    bool m_once;
    unsigned long m_timeout;
    unsigned long m_lastTrigger;
};

#endif/*__MYPI_ESP_GATE_TIMER_H__*/
