/**
 * @file ui_song.cpp
 * @author Leon Farchau (leon2225)
 * @brief Class that represents a song
 * @version 0.1
 * @date 07.08.2023
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "ui_song.h"
#include "songs/ui_song_interface.h"

ui_song::ui_song(std::string name, uint32_t duration, uint16_t bpm, std::vector<std::string> channels, std::vector<AdsrProfile> adsrEnvelopes)
{
    this->name = name;
    this->duration = duration;
    this->bpm = bpm;
    this->channels = channels;
    this->adsrEnvelopes = adsrEnvelopes;
    this->tones = std::vector<ui_tone>();
}

/**
 * @brief Returns the names of the channels
 * 
 * @param channelIdx   Pointer to the vector of channelIdx names
 * @return int      0 on success, -1 on failure
 */
int ui_song::getChannels(std::vector<std::string> &channelIdx)
{
    channelIdx = this->channels;
    return 0;
}

/**
 * @brief Returns the tones of the song that start in the given time range
 * 
 * @param startTime     Start time of the tones to get in us
 * @param endTime       End time of the tones to get in us
 * @return int          0 on success, -1 on failure
 */
int ui_song::getTones(uint32_t startTime, uint32_t endTime, std::span<ui_tone> &tonesOut)
{
    std::span<ui_tone> tones = CSongGetTones();

    int32_t start = 0;
    int32_t end = tones.size() - 1;
    uint32_t index = 0;

    for(auto tone : tones)
    {
        if(startTime <= tone.startTime && tone.startTime <= endTime)
        {
            if(start == 0)
            {
                start = index;
            }
        }
        else if(tone.startTime > endTime) // The tones are sorted by start time so we can break here
        {
            end = index;
            break;
        }
        index++;
    }
    tonesOut = tones.subspan(start, end - start);
    return 0;
}

/**
 * @brief Returns the tones of the song that are active in the given time range
 * 
 * @param timeHorizont  Time horizont in us in future to show tones
 * @param tonesOut      Pointer to the vector of tones
 * @return int          0 on success, -1 on failure
 */
int ui_song::getActiveTones(uint32_t timeHorizont,  std::span<ui_tone> &tonesOut)
{
    std::span<ui_tone> activeTones = CSongGetTones();
    uint32_t endTime = progress + timeHorizont;

    int32_t start = 0;
    int32_t end = activeTones.size() - 1;
    uint32_t index = 0;

    for(auto tone : activeTones)
    {
        if(progress <= tone.getEndTime() && tone.startTime <= endTime)
        {
            if(start == 0)
            {
                start = index;
            }
        }
        else if(tone.startTime > endTime) // The tones are sorted by start time so we can break here
        {
            end = index;
            break;
        }
        index++;
    }
    tonesOut = activeTones.subspan(start, end - start);
    return 0;
}

/**
 * @brief Updates the progress of the song by the given time delta
 * 
 * @param timeDelta   Time delta in us
 */
void ui_song::updateProgress(uint32_t timeDelta)
{
    progress += timeDelta;
    if(progress > duration)
    {
        progress = duration;
    }
}

/**
 * @brief Loads the tones of the song
 * 
 * @param tones     Pointer to the vector of tones
 * @param startTime Start time of the range to load in us
 * @param endTime   End time of the range to load in us
 */
void ui_song::loadTones(std::vector<ui_tone> *tones, uint32_t startTime, uint32_t endTime)
{
    this->tones = *tones;
}

/**
 * @brief Loads the meta data of the song
 * 
 */
void ui_song::loadMetaData()
{
    name = *CSongGetName();
    duration = *CSongGetDuration();
    bpm = *CSongGetBPM();
    channels = *CSongGetChannels();
    adsrEnvelopes = *CSongGetEnvelopes();
}