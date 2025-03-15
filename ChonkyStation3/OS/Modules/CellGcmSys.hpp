#pragma once

#include <common.hpp>
#include <logger.hpp>
#include <BEField.hpp>

#include <functional>

#include <CellTypes.hpp>


// Circular dependency
class PlayStation3;

using namespace CellTypes;

class CellGcmSys {
public:
    CellGcmSys(PlayStation3* ps3) : ps3(ps3) {}
    PlayStation3* ps3;

    struct CellGcmConfig {
        BEField<u32> local_addr;    // Pointer to RSX local memory
        BEField<u32> io_addr;
        BEField<u32> local_size;    // Size of RSX memory
        BEField<u32> io_size;
        BEField<u32> memFreq;
        BEField<u32> coreFreq;
    };

    struct CellGcmContextData {
        BEField<u32> begin;
        BEField<u32> end;
        BEField<u32> current;
        BEField<u32> callback;
    };

    struct CellGcmControl {
        BEField<u32> put;
        BEField<u32> get;
        BEField<u32> ref;
    };

    struct CellGcmDisplayInfo {
        BEField<u32> offset;
        BEField<u32> pitch;
        BEField<u32> width;
        BEField<u32> height;
    };

    struct CellGcmOffsetTable {
        BEField<u32> io_addr_ptr;
        BEField<u32> ea_addr_ptr;
    };

    static constexpr u32 tiled_pitches[] = {
        0x00000000, 0x00000200, 0x00000300, 0x00000400,
        0x00000500, 0x00000600, 0x00000700, 0x00000800,
        0x00000A00, 0x00000C00, 0x00000D00, 0x00000E00,
        0x00001000, 0x00001400, 0x00001800, 0x00001A00,
        0x00001C00, 0x00002000, 0x00002800, 0x00003000,
        0x00003400, 0x00003800, 0x00004000, 0x00005000,
        0x00006000, 0x00006800, 0x00007000, 0x00008000,
        0x0000A000, 0x0000C000, 0x0000D000, 0x0000E000,
        0x00010000
    };

    u32 ctx_addr = 0;
    CellGcmContextData* ctx;
    u32 ctrl_addr = 0;
    CellGcmControl* ctrl;

    CellGcmConfig gcm_config;
    u32 dma_ctrl_addr = 0;
    u32 io_table_ptr = 0;
    u32 ea_table_ptr = 0;
    u32 mapping_sizes[4096] = {0};
    u32 label_addr = 0;
    u32 buffer_info_addr = 0;
    u32 reports_addr = 0;
    u32 flip = 0;
    u32 flip_callback = 0;
    u32 vblank_handler = 0;

    void mapEaIo(u32 ea, u32 io);
    void unmapEaIo(u32 ea, u32 io);
    bool isIoOffsMapped(u32 io);
    void printOffsetTable();

    u64 cellGcmGetTiledPitchSize();
    u64 cellGcmGetDisplayInfo();
    u64 cellGcmInitBody();
    u64 _cellGcmSetFlipCommand();
    u64 cellGcmAddressToOffset();
    u64 cellGcmGetOffsetTable();
    u64 cellGcmBindTile();
    u64 cellGcmSetFlipMode();
    u64 cellGcmSetDebugOutputLevel();
    u64 cellGcmMapEaIoAddressWithFlags();
    u64 cellGcmMapEaIoAddress();
    u64 cellGcmGetFlipStatus();
    u64 cellGcmGetReportDataAddressLocation();
    u64 cellGcmSetWaitFlip();
    u64 cellGcmBindZcull();
    u64 cellGcmMapMainMemory();
    u64 cellGcmSetFlipHandler();
    u64 cellGcmSetDisplayBuffer();
    u64 cellGcmGetControlRegister();
    u64 cellGcmSetVBlankHandler();
    u64 cellGcmResetFlipStatus();
    u64 cellGcmSetDefaultCommandBuffer();
    u64 cellGcmSetTileInfo();
    u64 cellGcmInitDefaultFifoMode();
    u64 cellGcmSetGraphicsHandler();
    u64 cellGcmSetTile();
    u64 cellGcmSetZcull();
    u64 cellGcmUnmapIoAddress();
    u64 cellGcmSetFlip();
    u64 cellGcmGetConfiguration();
    u64 cellGcmGetLabelAddress();

    u64 cellGcmCallback();

private:
    MAKE_LOG_FUNCTION(log, cellGcmSys);
};