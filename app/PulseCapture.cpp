#include "PulseCapture.h"

// STM / C Libraries
#include <stm32l0xx_hal_tim.h>
#include <string.h>

extern TIM_HandleTypeDef htim2;

PulseCapture::PulseCapture() :
    m_initialized(false),
    m_apbFreq(1),
    m_lastCapture(0),
    m_captureIndex(0),
    m_numCaptures(0),
    m_captureQ()
{
    memset(m_captures, 0, MAX_NUM_CAPTURES * sizeof(uint16_t));
    m_timer.setInterval(5000);
}
PulseCapture::~PulseCapture() {}
void PulseCapture::init() {
    m_apbFreq = HAL_RCC_GetPCLK1Freq() / 64;
    HAL_TIM_IC_Start_DMA(&htim2, TIM_CHANNEL_1, (uint32_t *) m_captureQ.getBuffer(), m_captureQ.size());
    m_initialized = true;
}
uint32_t PulseCapture::calculateFrequency() {
    uint32_t averageInterval = 0;
    for(uint32_t i=0; i < MAX_NUM_CAPTURES; i++) {
        averageInterval += (uint32_t) (m_captures[i]);
    }
    if(averageInterval > 0) {
        uint32_t average = ((uint32_t) averageInterval) / ((uint32_t) MAX_NUM_CAPTURES);
        return m_apbFreq / average;
    } else {
        return 0;
    }
}

void PulseCapture::resetCaptures() {
    memset(m_captures, 0, MAX_NUM_CAPTURES * sizeof(uint16_t));
    m_captureIndex = 0;
}