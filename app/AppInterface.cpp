#include "AppInterface.h"
#include "PulseCapture.h"
#include "Knotmeter.h"
#include "YRShell.h"
#include <utility/DebugLog.h>

#include "main.h"

class MyShell : public YRSmallShell {
protected:
    virtual const char* shellClass( void) { return "MyShell"; }
    virtual const char* mainFileName( ) { return "app/AppInterface.cpp"; }
public:
    MyShell() {}
    virtual ~MyShell() {}
};

//MyShell yrShell;
DebugLog debugLog;

CircularQ<char, 16>       m_serial1Inq;
CircularQ<char, 128>      m_serial1Outq;
CircularQ<char, 16>       m_inq;
CircularQ<char, 128>      m_outq;

//PulseCapture pulseCapture(&debugLog);
Knotmeter knotmeter(&debugLog);

void initializeApp() {
    BSerial1.begin(4800);
    BSerial1.init(m_serial1Inq, m_serial1Outq);


    BSerial2.begin(115200);

    //BSerial2.init(yrShell.getInq(), yrShell.getOutq());
    BSerial2.init(m_inq, m_outq);

    //pulseCapture.init();
    knotmeter.init();
}

void updateApp() {
    // TODO: Re-add LED blinking
    // HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    // HAL_Delay(500);
    Sliceable::sliceAll();

    char c;
    for( uint8_t i = 0; i < 32 && debugLog.valueAvailable(); i++) {
      c = debugLog.get();
      m_outq.put(c);
    }


    if(knotmeter.getOutQ()->valueAvailable()) {
        char c;
        for( uint8_t i = 0; i < 48 && knotmeter.getOutQ()->valueAvailable(); i++) {
            c = knotmeter.getOutQ()->get();
            m_serial1Outq.put(c);
        }
    }
}