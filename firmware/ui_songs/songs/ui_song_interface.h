/**
 * @file ui_song_interface.h
 * @author Leon Farchau (leon2225)
 * @brief Header that defines the interface to get the song to be compiled
 * @version 0.1
 * @date 08.08.2023
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include "../../ToneSheduler.h"
#include "../ui_tone.h"
#include <stdint.h>
#include <vector>
#include <string>
#include <span>


std::span<ui_tone> CSongGetTones();
const std::vector<std::string>* CSongGetChannels();
const std::vector<AdsrProfile>* CSongGetEnvelopes();
const std::string* CSongGetName();
const uint16_t* CSongGetBPM();
const uint32_t* CSongGetDuration();