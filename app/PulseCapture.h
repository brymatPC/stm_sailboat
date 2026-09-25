#ifndef PULSE_CAPTURE_H_
#define PULSE_CAPTURE_H_

// STM / C Libraries


// External components
#include <utility/IntervalTimer.h>
#include <utility/Sliceable.h>
#include <utility/CircularQ.h>

#define MAX_NUM_CAPTURES 32

class PulseCapture : public Sliceable {
private:
    uint32_t m_pin;
    uint32_t m_apbFreq;
    uint16_t m_lastCapture;
    uint16_t m_captures[MAX_NUM_CAPTURES];
    uint32_t m_captureIndex;
    float m_freq;

    CircularQ<uint16_t, MAX_NUM_CAPTURES> m_captureQ;

protected:
    bool m_initialized;
    uint32_t m_numCaptures;
    IntervalTimer m_timer;
    float calculateFrequency();
    void resetCaptures();
public:
    PulseCapture();
    virtual ~PulseCapture();
    virtual const char* sliceName( void) { return "PulseCapture"; }
    virtual void init();
    virtual void slice( void);
};

#endif //PULSE_CAPTURE_H_