#include "Knotmeter.h"

// STM / C Libraries
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define KNOTMETER_DIAMETER_M  (0.023f)
#define KNOTMETER_DIAMETER_MM  (23.0f)
#define KNOTMETER_EST_SLIP    (1.0f)
#define FREQ_TO_SPEED         (M_PI * KNOTMETER_DIAMETER_M / KNOTMETER_EST_SLIP)
#define FREQ_TO_SPEED_MM      (M_PI * KNOTMETER_DIAMETER_MM / KNOTMETER_EST_SLIP)

// Anything above this is invalid, in tenths of a Hz
#define KNOTMETER_MAX_FREQ    (1000)
// Anything less than this is invalid
#define KNOTMETER_MIN_COUNT   (2500)

const uint32_t Knotmeter::s_FREQ_TO_KNOTS = (uint32_t) (FREQ_TO_SPEED * 1.94384f * 100.0f);
const uint32_t Knotmeter::s_FREQ_TO_KMH = (uint32_t) (FREQ_TO_SPEED * 3.6f * 100.0f);

const uint32_t Knotmeter::s_FREQ_TO_mKNOTS = (uint32_t) (FREQ_TO_SPEED_MM * 1.94384f);
const uint32_t Knotmeter::s_FREQ_TO_MH = (uint32_t) (FREQ_TO_SPEED_MM * 3.6f);

extern TIM_HandleTypeDef htim2;

Knotmeter::Knotmeter() :
    m_speed_kts(0)
{
    m_timer.setInterval(5000);
}

void Knotmeter::slice() {
    if(!m_initialized) return;

    uint16_t availableQ = m_captureQ.size();
    uint16_t bytesInDMA = __HAL_DMA_GET_COUNTER(htim2.hdma[TIM_DMA_ID_CC1]);
    m_captureQ.setHead(availableQ - bytesInDMA);

    if(m_captureQ.used() > 0) {
        uint16_t captureValue = m_captureQ.get();
        m_captures[m_captureIndex] = (captureValue - m_lastCapture);
        m_lastCapture = captureValue;
        m_captureIndex++;
        if(m_captureIndex >= MAX_NUM_CAPTURES) {
            m_captureIndex = 0;
        }
        m_numCaptures++;
    }

    if(m_timer.isNextInterval()) {
        uint32_t average = 0;
        uint32_t numCaptures = m_numCaptures;
        m_numCaptures = 0;
        if(numCaptures > 0) {
            average = calculateAverage();
            if(average < KNOTMETER_MIN_COUNT) {
                average = 0;
            }
        } else {
            resetCaptures();
        }
        if(average > 0) {
            m_speed_kts = (m_apbFreq * s_FREQ_TO_mKNOTS + (average / 2)) / average;
            uint32_t speed_km_per_hour = (m_apbFreq * s_FREQ_TO_MH + (average / 2)) / average;
            generateNmeaSentence(m_speed_kts, speed_km_per_hour);
        } else {
            m_speed_kts = 0;
            generateNmeaSentence(m_speed_kts, 0);
        }
    }
}

void Knotmeter::init() {
    PulseCapture::init();
}

bool Knotmeter::isValid() {
    return m_speed_kts > 0;
}

void Knotmeter::generateNmeaSentence(uint32_t speedKnots, uint32_t speedKmH) {
    // Format: $VHW,x.x,T,x.x,M,x.x,N,x.x,K*hh<CR><LF>
    static char startStr[] = "DMVHW,,T,,M,";
    char nmeaBuf[100];

    strncpy(nmeaBuf, startStr, 100);
    size_t len = strlen(nmeaBuf);

    uint16_t knotsInt = (uint16_t) (speedKnots / 1000);
    uint16_t knotsTenths = (uint16_t) (speedKnots / 100);
    knotsTenths = knotsTenths % 10;

    nmeaBuf[len++] = '0' + knotsInt / 10;
    nmeaBuf[len++] = '0' + knotsInt % 10;
    nmeaBuf[len++] = '.';
    nmeaBuf[len++] = '0' + knotsTenths;
    nmeaBuf[len++] = ',';
    nmeaBuf[len++] = 'N';
    nmeaBuf[len++] = ',';

    uint16_t kmInt = (uint16_t) (speedKmH / 1000);
    uint16_t kmTenths = (uint16_t) (speedKmH / 100);
    kmTenths = kmTenths % 10;

    nmeaBuf[len++] = '0' + kmInt / 10;
    nmeaBuf[len++] = '0' + kmInt % 10;
    nmeaBuf[len++] = '.';
    nmeaBuf[len++] = '0' + kmTenths;
    nmeaBuf[len++] = ',';
    nmeaBuf[len++] = 'K';

    uint8_t checksum = 0;
    for(uint8_t i=0; i < len; i++) {
        checksum ^= nmeaBuf[i];
    }

    nmeaBuf[len++] = '*';

    uint8_t c = (checksum & 0xF0) >> 4;
    if(c >  9) {
        nmeaBuf[len++] = '7' + c;
    } else {
        nmeaBuf[len++] = '0' + c;
    }
    c = (checksum & 0x0F);
    if(c >  9) {
        nmeaBuf[len++] = '7' + c;
    } else {
        nmeaBuf[len++] = '0' + c;
    }
    nmeaBuf[len++] = '\r';
    nmeaBuf[len++] = '\n';


    len = strlen(nmeaBuf);
    if(m_outQ.spaceAvailable(len+1)) {
        m_outQ.put('$');
        for(uint8_t i=0; i < len; i++) {
            m_outQ.put(nmeaBuf[i]);
        }
    }
}