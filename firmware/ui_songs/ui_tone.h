/**
 * @file ui_tone.h
 * @author Leon Farchau (leon2225)
 * @brief Struct that represents a single tone
 * @version 0.1
 * @date 07.08.2023
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include <stdint.h>
#include <../ToneSheduler.h>


/**
 * @brief Struct that represents a single tone
 * 
 */
struct ui_tone
{
    float frequency;        /**< Frequency of the tone */
    uint32_t duration;      /**< Duration of the tone */
    uint32_t startTime;     /**< Start time of the tone */
    uint16_t channelIdx;    /**< Channel index of the tone */
    uint16_t velocity;      /**< Velocity of the tone */
    AdsrProfile *adsr;      /**< Pointer to ADSR-Profile */

    /**
     * @brief Construct a new ui tone object
     * 
     * @param frequency Frequency of the tone
     * @param duration Duration of the tone
     * @param startTime Start time of the tone
     * @param channelIdx Channel of the tone
     * @param velocity Velocity of the tone
     * @param adsr ADSR index of the tone
     */
    ui_tone(float frequency, uint32_t duration, uint32_t startTime, uint16_t channelIdx = 0, uint16_t velocity = 128, AdsrProfile *adsr = nullptr)
        : frequency(frequency), duration(duration), startTime(startTime), channelIdx(channelIdx), velocity(velocity), adsr(adsr) {}
    

    uint32_t getEndTime() const { return startTime + duration; }
};

