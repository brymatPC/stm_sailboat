#ifndef PULSE_CAPTURE_H_
#define PULSE_CAPTURE_H_

// STM / C Libraries


// External components
#include <utility/IntervalTimer.h>
#include <utility/Sliceable.h>

#define MAX_NUM_CAPTURES 32

class PulseCapture : public Sliceable {
private:
    uint32_t m_pin;
    // uint8_t m_group;
    // mcpwm_cap_timer_handle_t m_handle;
    // mcpwm_cap_channel_handle_t m_chanHandle;
    uint32_t m_apbFreq;


protected:
    bool m_initialized;
    IntervalTimer m_timer;
    float calculateFrequency();
    void resetCaptures();
public:
    PulseCapture(uint32_t pin);
    virtual ~PulseCapture();
    virtual const char* sliceName( void) { return "PulseCapture"; }
    virtual void init();
    virtual void slice( void);
};

#endif //PULSE_CAPTURE_H_