#include "AppInterface.h"
#include "PulseCapture.h"
#include "Knotmeter.h"
#include <utility/BufferedSerial.h>

#include "main.h"

CircularQ<char, 256>       m_serial1Inq;
CircularQ<char, 256>      m_serial1Outq;

Knotmeter knotmeter;

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

    if(knotmeter.getOutQ()->valueAvailable()) {
        char c;
        for( uint8_t i = 0; i < 48 && knotmeter.getOutQ()->valueAvailable(); i++) {
            c = knotmeter.getOutQ()->get();
            m_serial1Outq.put(c);
        }
    }
}