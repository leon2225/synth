/**
 * @file ui_song.h 
 * @author Leon Farchau (leon2225)
 * @brief Class that represents a song
 * @version 0.1
 * @date 07.08.2023
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include <stdint.h>
#include <vector>
#include <string>
#include <span>

#include "ui_tone.h"

/**
 * @brief Class that represents a song
 * 
 */
class ui_song{
public:
    ui_song() = default;
    ui_song(std::string name, uint32_t duration, uint16_t bpm, std::vector<std::string> channels, std::vector<AdsrProfile> adsrEnvelopes);
    ~ui_song() = default;

    // Meta data
    void loadMetaData();
    std::string getName() const { return name; }
    uint16_t getBpm() const { return bpm; }
    uint32_t getDuration() const { return duration; }
    int getChannels(std::vector<std::string> &channelIdx);

    // Dynamic data
    void setProgress(uint32_t progress) { this->progress = progress; }
    void updateProgress(uint32_t timeDelta);
    uint32_t getProgress() const { return progress; }
    int getTones(uint32_t startTime, uint32_t endTime,  std::span<ui_tone> &tonesOut);
    int getActiveTones(uint32_t timeHorizont,  std::span<ui_tone> &tonesOut);

    // Data acquisition
    void loadTones(std::vector<ui_tone> *tones, uint32_t startTime, uint32_t endTime);
    
private:
    std::string name;                               /**< Name of the song */
    uint16_t bpm;                                   /**< Beats per minute of the song */
    uint32_t duration;                              /**< Duration of the song in us */
    uint32_t progress;                              /**< Current progress of the song in us */
    std::vector<std::string> channels;              /**< Names of the channels */
    std::vector<AdsrProfile> adsrEnvelopes;         /**< Vector of adsr-envelopes */
    std::vector<ui_tone> tones;                     /**< Vector of tones */
};
