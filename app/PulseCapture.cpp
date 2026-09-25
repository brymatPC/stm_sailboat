#include "PulseCapture.h"

// STM / C Libraries
#include <stm32l0xx_hal_tim.h>
#include <string.h>

extern TIM_HandleTypeDef htim2;


uint16_t m_lastCapture;
uint16_t m_captures[MAX_NUM_CAPTURES];
uint32_t m_captureIndex;
uint32_t m_numCaptures;

uint16_t captureValue = 0;
uint8_t timerRollovers = 0;

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
        captureValue = (uint16_t) HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
        m_captures[m_captureIndex] = (captureValue - m_lastCapture);
        m_lastCapture = captureValue;
        m_captureIndex++;
        if(m_captureIndex >= MAX_NUM_CAPTURES) {
            m_captureIndex = 0;
        }
        m_numCaptures++;
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        // Code to run on timer rollover/overflow
        timerRollovers++;
    }
}

PulseCapture::PulseCapture() :
    m_pin(0),
    m_apbFreq(1),
    m_freq(0.0f),
    m_initialized(false)
{
    memset(m_captures, 0, MAX_NUM_CAPTURES * sizeof(uint16_t));
    m_timer.setInterval(5000);
}
PulseCapture::~PulseCapture() {}
void PulseCapture::init() {

    m_apbFreq = HAL_RCC_GetPCLK1Freq() / 32;

    HAL_TIM_Base_Start_IT(&htim2);
    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);

    m_initialized = true;
}
void PulseCapture::slice() {
    if(!m_initialized) return;
    if(m_timer.isNextInterval()) {
        float freq = 0.0f;
        uint32_t numCaptures = m_numCaptures;
        m_numCaptures = 0;
        if(numCaptures > 0) {
            freq = calculateFrequency();
        } else {
            resetCaptures();
        }
        m_freq = freq;
        //ESP_LOGI(TAG, "%d - Est freq %.2f, numCaptures %lu", m_group, freq, numCaptures);
    }
}

float PulseCapture::calculateFrequency() {
    uint32_t averageInterval = 0;
    for(uint32_t i=0; i < MAX_NUM_CAPTURES; i++) {
        averageInterval += m_captures[i];
    }
    if(averageInterval > 0) {
        float average = ((float) averageInterval) / ((float) MAX_NUM_CAPTURES);
        float clkFreq = (float) m_apbFreq;
        return clkFreq / average;
    } else {
        return 0;
    }
}

void PulseCapture::resetCaptures() {
    memset(m_captures, 0, MAX_NUM_CAPTURES * sizeof(uint32_t));
    m_captureIndex = 0;
}