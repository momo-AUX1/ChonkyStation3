#pragma once

#include <common.hpp>
#include <logger.hpp>
#include <BEField.hpp>

#include <CellTypes.hpp>


// Circular dependency
class PlayStation3;

using namespace CellTypes;

static constexpr u8 CELL_AUDIO_OUT_OUTPUT_STATE_ENABLED    = 0;
static constexpr u8 CELL_AUDIO_OUT_OUTPUT_STATE_DISABLED   = 1;
static constexpr u8 CELL_AUDIO_OUT_OUTPUT_STATE_PREPARING  = 2;

class CellAudioOut {
public:
    CellAudioOut(PlayStation3* ps3) : ps3(ps3) {}
    PlayStation3* ps3;

    u64 cellAudioOutGetState();

private:
    MAKE_LOG_FUNCTION(log, cellAudioOut);
};