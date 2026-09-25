#include "Knotmeter.h"

// STM / C Libraries
#include <math.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define KNOTMETER_DIAMETER_M  (0.023f)
#define KNOTMETER_EST_SLIP    (1.0f)
#define FREQ_TO_SPEED         (M_PI * KNOTMETER_DIAMETER_M / KNOTMETER_EST_SLIP)

// Anything above this is invalid
#define KNOTMETER_MAX_FREQ    (100.0f)


const float Knotmeter::s_DEFAULT_SCALE = FREQ_TO_SPEED;
const float Knotmeter::s_DEFAULT_OFFSET = 0.0f;

extern TIM_HandleTypeDef htim2;

Knotmeter::Knotmeter(DebugLog *dbg) :
    m_enabled(true),
    m_scale(s_DEFAULT_SCALE),
    m_offset(s_DEFAULT_OFFSET),
    m_speed_kts(0.0f),
    m_pdbg(dbg)
{
    m_timer.setInterval(5000);
}

void Knotmeter::slice() {
    if(!m_initialized || !m_enabled) return;

	uint16_t availableQ  = m_captureQ.size();
    uint16_t bytesInDMA  = __HAL_DMA_GET_COUNTER(htim2.hdma[TIM_DMA_ID_CC1]);
    m_captureQ.setHead(availableQ - bytesInDMA);

    {
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
    }

    if(m_timer.isNextInterval()) {
        float freq = 0.0f;
        uint32_t numCaptures = m_numCaptures;
        m_numCaptures = 0;
        if(numCaptures > 0) {
            freq = calculateFrequency();
            if(freq > KNOTMETER_MAX_FREQ) {
                freq = 0.0f;
            }
        } else {
            resetCaptures();
        }
        float speed_m_per_sec = freq * m_scale + m_offset;
        m_speed_kts = speed_m_per_sec * 1.94384f;
        float speed_km_per_hour = speed_m_per_sec * 3.6f;
        if(m_pdbg) {
            m_pdbg->print(__FILE__, __LINE__, 1, (uint32_t) (freq * 10.0f), (uint32_t) (m_speed_kts * 10.0f), "Freq, knots");
        }
        generateNmeaSentence(m_speed_kts, speed_km_per_hour);
    }
}

void Knotmeter::init() {
    PulseCapture::init();
}

void Knotmeter::generateNmeaSentence(float speedKnots, float speedKmH) {
    // Format: $VHW,x.x,T,x.x,M,x.x,N,x.x,K*hh<CR><LF>
    char nmeaBuf[70];

    uint16_t knotsInt = (uint16_t) speedKnots;
    uint16_t knotsTenths = (uint16_t) (speedKnots * 10.0f);
    knotsTenths = knotsTenths % 10;

    uint16_t kmInt = (uint16_t) speedKmH;
    uint16_t kmTenths = (uint16_t) (speedKmH * 10.0f);
    kmTenths = kmTenths % 10;

    snprintf(nmeaBuf, 70, "DMVHW,,T,,M,%0u.%0u,N,%0u.%0u,K", knotsInt, knotsTenths, kmInt, kmTenths);

    uint8_t checksum = 0;
    uint8_t len = strlen(nmeaBuf);
    for(uint8_t i=0; i < len; i++) {
        checksum ^= nmeaBuf[i];
    }
    snprintf(m_nmeaSentence, 83, "$%s*%02X\r\n", nmeaBuf, checksum);

    len = strlen(m_nmeaSentence);
    if(m_outQ.spaceAvailable(len)) {
        for(uint8_t i=0; i < len; i++) {
            m_outQ.put(m_nmeaSentence[i]);
        }
    }
    if(m_pdbg) {
        m_pdbg->print(__FILE__, __LINE__, 1, nmeaBuf);
    }
}