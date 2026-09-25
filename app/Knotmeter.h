#ifndef KNOTMETER_H_
#define KNOTMETER_H_

#include "PulseCapture.h"

// External components
#include <CircularQ.h>
#include <utility/DebugLog.h>

#define MAX_KNOT_LOG_BUF_SIZE 256

class SdLogger;

class Knotmeter : public PulseCapture {
private:
    static const float s_DEFAULT_SCALE;
    static const float s_DEFAULT_OFFSET;
    bool m_enabled;
    float m_scale;
    float m_offset;
    float m_speed_kts;

    char m_nmeaSentence[83];
    CircularQ<char, 512> m_outQ;

    DebugLog *m_pdbg;

    void generateNmeaSentence(float speedKnots, float speedKmH);
public:
    Knotmeter(DebugLog *dbg);
    virtual const char* sliceName( void) { return "Knotmeter"; }
    virtual void slice( void) override;

    void init() override;
    void off() { m_enabled = false; }

    void setScale(float scale) { m_scale = scale; }
    void setOffset(float offset) { m_offset = offset; }

    CircularQBase<char> *getOutQ() { return &m_outQ; }
};


#endif // KNOTMETER_H_