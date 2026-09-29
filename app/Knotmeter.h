#ifndef KNOTMETER_H_
#define KNOTMETER_H_

#include "PulseCapture.h"

// External components
#include <CircularQ.h>

class Knotmeter : public PulseCapture {
private:
    static const uint32_t s_FREQ_TO_KNOTS;
    static const uint32_t s_FREQ_TO_KMH;
    uint32_t m_speed_kts;

    CircularQ<char, 256> m_outQ;

    void generateNmeaSentence(uint32_t speedKnots, uint32_t speedKmH);
public:
    Knotmeter();
    virtual const char* sliceName( void) { return "Knotmeter"; }
    virtual void slice( void) override;

    void init() override;

    bool isValid();

    CircularQBase<char> *getOutQ() { return &m_outQ; }
};


#endif // KNOTMETER_H_