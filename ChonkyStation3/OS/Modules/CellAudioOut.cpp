#include "CellAudioOut.hpp"
#include "PlayStation3.hpp"


u64 CellAudioOut::cellAudioOutGetState() {
    const u32 audio_out = ARG0;
    const u32 dev_idx = ARG1;
    const u32 state_ptr = ARG2;
    log("cellAudioOutGetState(audio_out: %d, dev_idx: %d, state_ptr: 0x%08x) STUBBED\n", audio_out, dev_idx, state_ptr);

    ps3->mem.write<u8>(state_ptr, CELL_AUDIO_OUT_OUTPUT_STATE_ENABLED);

    return Result::CELL_OK;
}