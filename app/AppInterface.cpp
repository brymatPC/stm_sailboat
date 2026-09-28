#include "AppInterface.h"
#include "PulseCapture.h"
#include "Knotmeter.h"
//#include "YRShell.h"
#include <utility/DebugLog.h>
#include <utility/BufferedSerial.h>

#include "main.h"

DebugLog debugLog;

CircularQ<char, 256>       m_serial1Inq;
CircularQ<char, 256>      m_serial1Outq;

Knotmeter knotmeter(&debugLog);

void initializeApp() {
    BSerial2.begin(4800);
    BSerial2.init(m_serial1Inq, m_serial1Outq);

    knotmeter.init();
}

void updateApp() {
    Sliceable::sliceAll();

    if(knotmeter.isValid()) {
        HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PinState::GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PinState::GPIO_PIN_RESET);
    }

    // char c;
    // for( uint8_t i = 0; i < 32 && debugLog.valueAvailable(); i++) {
    //   c = debugLog.get();
    //   m_serial1Outq.put(c);
    // }

    if(knotmeter.getOutQ()->valueAvailable()) {
        char c;
        for( uint8_t i = 0; i < 48 && knotmeter.getOutQ()->valueAvailable(); i++) {
            c = knotmeter.getOutQ()->get();
            m_serial1Outq.put(c);
        }
    }
}