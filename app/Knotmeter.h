#ifndef KNOTMETER_H_
#define KNOTMETER_H_

#include "PulseCapture.h"

// External components
#include <CircularQ.h>
#include <utility/DebugLog.h>

class Knotmeter : public PulseCapture {
private:
    static const uint32_t s_FREQ_TO_KNOTS;
    uint32_t m_speed_kts;

    char m_nmeaSentence[83];
    CircularQ<char, 256> m_outQ;

    DebugLog *m_pdbg;

    void generateNmeaSentence(uint32_t speedKnots, uint32_t speedKmH);
public:
    Knotmeter(DebugLog *dbg);
    virtual const char* sliceName( void) { return "Knotmeter"; }
    virtual void slice( void) override;

    void init() override;

    bool isValid();

    CircularQBase<char> *getOutQ() { return &m_outQ; }
};


#endif // KNOTMETER_H_