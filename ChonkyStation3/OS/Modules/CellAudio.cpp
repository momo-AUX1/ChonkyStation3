#include "CellAudio.hpp"
#include "PlayStation3.hpp"


u64 CellAudio::cellAudioGetPortConfig() {
    u32 port_num = ARG0;
    u32 config_ptr = ARG1;
    log("cellAudioGetPortConfig(port_num: %d, config_ptr: 0x%08x)\n", port_num, config_ptr);

    Helpers::debugAssert(port_num < 8, "cellAudioGetPortConfig: port_num is invalid (%d)\n", port_num);
    Port& port = ports[port_num];
    CellAudioPortConfig* config = (CellAudioPortConfig*)ps3->mem.getPtr(config_ptr);

    config->read_idx_addr = port.addr;  // TODO
    config->status = port.status;
    config->n_channels = port.n_channels;
    config->n_blocks = port.n_blocks;
    config->port_size = port.size;
    config->port_addr = port.addr;

    return Result::CELL_OK;
}

u64 CellAudio::cellAudioPortStart() {
    u32 port_num = ARG0;
    log("cellAudioPortStart(port_num: %d)\n", port_num);

    Helpers::debugAssert(port_num < 8, "cellAudioPortStart: port_num is invalid (%d)\n", port_num);
    Port& port = ports[port_num];
    port.status = CELL_AUDIO_STATUS_RUN;

    return Result::CELL_OK;
}

u64 CellAudio::cellAudioPortOpen() {
    const u32 param_ptr = ARG0;
    const u32 port_ptr = ARG1;
    log("cellAudioPortOpen(param_ptr: 0x%08x, port_ptr: 0x%08x)\n", param_ptr, port_ptr);

    CellAudioPortParam* param = (CellAudioPortParam*)ps3->mem.getPtr(param_ptr);

    // Find an available port and open it
    u32 port_num;
    Port* port = nullptr;
    for (port_num = 0; port_num < 8; port_num++) {
        if (ports[port_num].status == CELL_AUDIO_STATUS_CLOSE) {
            port = &ports[port_num];
            port->status = CELL_AUDIO_STATUS_READY;
            port->n_channels = param->n_channels;
            port->n_blocks = param->n_blocks;
            break;
        }
    }

    if (!port) {    // No port was available
        Helpers::panic("cellAudioPortOpen: no ports available\n");
    }

    // Allocate memory
    port->size = port->n_channels * port->n_blocks * 256 * sizeof(float);
    port->addr = ps3->mem.alloc(port->size)->vaddr;

    ps3->mem.write<u32>(port_ptr, port->id);
    log("Opened port %d (channels: %d, blocks: %d)\n", port->id, port->n_channels, port->n_blocks);

    return Result::CELL_OK;
}