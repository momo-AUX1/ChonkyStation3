#include "CellGcmSys.hpp"
#include "PlayStation3.hpp"


u64 CellGcmSys::cellGcmGetTiledPitchSize() {
    const u32 size = ARG0;
    log("cellGcmGetTiledPitchSize(size: %d)\n", size);

    for (int i = 0; i < 32; i++) {
        if (size > tiled_pitches[i] && size <= tiled_pitches[i + 1])
            return tiled_pitches[i + 1];
    }

    return 0;
}

u64 CellGcmSys::cellGcmGetDisplayInfo() {
    log("cellGcmGetDisplayInfo()\n");
    return buffer_info_addr;
}

u64 CellGcmSys::cellGcmInitBody() {
    const u32 ctx_ptr = ARG0;
    const u32 cmd_size = ARG1;
    const u32 io_size = ARG2;
    const u32 io_addr = ARG3;
    log("cellGcmInitBody(ctx_ptr: 0x%08x, cmd_size: 0x%08x, io_size: 0x%08x, io_addr: 0x%08x)\n", ctx_ptr, cmd_size, io_size, io_addr);

    gcm_config.local_size = 249_MB;
    gcm_config.local_addr = ps3->mem.rsx.alloc(gcm_config.local_size)->vaddr;
    gcm_config.io_addr = io_addr;
    gcm_config.io_size = io_size;
    gcm_config.memFreq = 650000000;
    gcm_config.coreFreq = 500000000;

    std::memset(ps3->mem.getPtr(gcm_config.local_addr), 0, gcm_config.local_size);

    ctx_addr = ps3->mem.alloc(sizeof(CellGcmContextData))->vaddr;
    ctx = (CellGcmContextData*)ps3->mem.getPtr(ctx_addr);
    ps3->mem.write<u32>(ctx_ptr, ctx_addr);

    // Setup callback
    u32 callback_addr = ps3->mem.alloc(4 * 4)->vaddr;
    ps3->mem.write<u32>(callback_addr +  0, callback_addr + 4);
    ps3->mem.write<u32>(callback_addr +  4, 0x39600400);     // li r11, 0x400
    ps3->mem.write<u32>(callback_addr +  8, 0x44000002);     // sc
    ps3->mem.write<u32>(callback_addr + 12, 0x4E800020);     // blr

    ctx->begin = io_addr;
    ctx->end =  io_addr + io_size;
    ctx->current = io_addr;
    ctx->callback = callback_addr;

    dma_ctrl_addr = ps3->mem.rsx.alloc(3_MB)->vaddr;
    std::memset(ps3->mem.getPtr(dma_ctrl_addr), 0, 3_MB);
    ctrl_addr = dma_ctrl_addr + 0x40;
    ctrl = (CellGcmControl*)ps3->mem.getPtr(ctrl_addr);
    ctrl->put = 0;
    ctrl->get = 0;
    ctrl->ref = -1;

    // Allocate display buffer info
    buffer_info_addr = ps3->mem.alloc(sizeof(CellGcmDisplayInfo) * 8)->vaddr;
    std::memset(ps3->mem.getPtr(buffer_info_addr), 0, sizeof(CellGcmDisplayInfo) * 8);
    
    // Empty dummy reports area
    reports_addr = ps3->mem.alloc(1_MB)->vaddr;
    std::memset(ps3->mem.getPtr(reports_addr), 0, 1_MB);

    // Memory watchpoint to tell the RSX to check if there are commands to run when put is written
    ps3->mem.watchpoints_w[ctrl_addr] = std::bind(&RSX::runCommandList, &ps3->rsx, std::placeholders::_1);
    ps3->mem.markAsSlowMem(ctrl_addr >> PAGE_SHIFT, false, true);   // Only need to make writes take the slow path

    label_addr = dma_ctrl_addr + 2_MB;

    // Initialize offset table
    // The io_addr table has 3072 entries; the ea_addr table has 256 entries unless we are emulating a 512MB RSX unit.
    static constexpr size_t table_size = (3072 + 512) * sizeof(u16);
    const u32 offset_table_addr = ps3->mem.alloc(table_size)->vaddr;
    io_table_ptr = offset_table_addr;
    ea_table_ptr = offset_table_addr + 3072 * sizeof(u16);
    std::memset(ps3->mem.getPtr(offset_table_addr), 0xff, table_size); // The table is initialized to all FFs

    for (u32 i = 0; i < (gcm_config.io_size >> 20); i++)
        mapEaIo(gcm_config.io_addr + (i << 20), i << 20);

    ps3->rsx.setEaTableAddr(ea_table_ptr);

    return Result::CELL_OK;
}

// Maps a 1MB page from main memory to IO address space 
void CellGcmSys::mapEaIo(u32 ea, u32 io) {
    ps3->mem.write<u16>(ea_table_ptr + ((io >> 20) * 2), ea >> 20);
    ps3->mem.write<u16>(io_table_ptr + ((ea >> 20) * 2), io >> 20);

    log("Mapped addr 0x%08x to IO offset 0x%08x\n", ea, io);
}

// Unmaps a 1MB page from main memory from IO address space 
void CellGcmSys::unmapEaIo(u32 ea, u32 io) {
    ps3->mem.write<u16>(ea_table_ptr + ((io >> 20) * 2), 0xffff);
    ps3->mem.write<u16>(io_table_ptr + ((ea >> 20) * 2), 0xffff);

    log("Unmapped addr 0x%08x from IO offset 0x%08x\n", ea, io);
}

// TODO: This is broken, need to go through the read/write functions instead of using raw pointers due to endianness
void CellGcmSys::printOffsetTable() {
    u16* ea_table = (u16*)ps3->mem.getPtr(ea_table_ptr);
    u16* io_table = (u16*)ps3->mem.getPtr(io_table_ptr);

    bool logged_once = false;
    log("Offset table (ea -> io):");
    for (u32 i = 0; i < 3072; i++) {
        if (io_table[i] != 0xffff) {
            logged_once = true;
            logNoPrefix("\n");
            log("0x%08x -> 0x%08x", i << 20, io_table[i] << 20);
        }
    }
    if (!logged_once) printf(" [empty]");
    printf("\n");

    logged_once = false;
    log("Offset table (io -> ea):");
    for (u32 i = 0; i < 256; i++) {
        if (ea_table[i] != 0xffff) {
            logged_once = true;
            logNoPrefix("\n");
            log("0x%08x -> 0x%08x", i << 20, ea_table[i] << 20);
        }
    }
    if (!logged_once) printf(" [empty]");
    printf("\n");
}

// Returns whether the given offset in IO memory is mapped to an address in main memory
bool CellGcmSys::isIoOffsMapped(u32 io) {
    return ps3->mem.read<u16>(ea_table_ptr + ((io >> 20) * 2)) != 0xffff;
}

u64 CellGcmSys::_cellGcmSetFlipCommand() {
    const u32 context_addr = ARG0;
    const u32 buf_id = ARG1;
    //log("_cellGcmSetFlipCommand(ctx_addr: 0x%08x, buf_id: %d)\n", context_addr, buf_id);

    CellGcmContextData* context = (CellGcmContextData*)ps3->mem.getPtr(context_addr);
    if (ctx->current + 8 >= ctx->end) cellGcmCallback();
    ps3->mem.write<u32>(context->current, RSX::GCM_FLIP_COMMAND | (1 << 18));   // 1 is argc
    ps3->mem.write<u32>(context->current + 4, buf_id);
    context->current = context->current + 8;

    return Result::CELL_OK;
}

u64 CellGcmSys::cellGcmAddressToOffset() {
    const u32 addr = ARG0;
    const u32 offs_ptr = ARG1;
    //log("cellGcmAddressToOffset(addr: 0x%08x, offs_ptr: 0x%08x)", addr, offs_ptr);

    u32 offs = 0;
    // Check if the address is in RSX memory
    if (Helpers::inRange<u32>(addr, gcm_config.local_addr, gcm_config.local_addr + gcm_config.local_size - 1)) {
        offs = addr - gcm_config.local_addr;
    }
    // Check if it's mapped to IO
    else {
        u16* io_table = (u16*)ps3->mem.getPtr(io_table_ptr);
        const u32 page = ps3->mem.read<u16>(io_table_ptr + ((addr >> 20) * 2));
        if (page != 0xffff)
            offs = (page << 20) | (addr & 0xfffff);
        else {
            Helpers::panic("\ncellGcmAddressToOffset: addr is not in rsx memory or io memory (0x%08x)\n", addr);
            ps3->mem.write<u32>(offs_ptr, 0);
            return 0x802100ff;  // CELL_GCM_ERROR_FAILURE
        }
    }

    //logNoPrefix(" [offs: 0x%08x]\n", offs);
    ps3->mem.write<u32>(offs_ptr, offs);
    
    return Result::CELL_OK;
}

u64 CellGcmSys::cellGcmGetOffsetTable() {
    const u32 table_ptr = ARG0;
    log("cellGcmGetOffsetTable(table_ptr: 0x%08x)\n", table_ptr);

    CellGcmOffsetTable* table = (CellGcmOffsetTable*)ps3->mem.getPtr(table_ptr);
    table->ea_addr_ptr = ea_table_ptr;
    table->io_addr_ptr = io_table_ptr;

    return Result::CELL_OK;
}

u64 CellGcmSys::cellGcmBindTile() {
    const u8 idx = ARG0;
    log("cellGcmBindTile(idx: %d) UNIMPLEMENTED\n", idx);

    return Result::CELL_OK;
}

u64 CellGcmSys::cellGcmSetFlipMode() {
    const u32 mode = ARG0;
    log("cellGcmSetFlipMode(mode: 0x%08x) UNIMPLEMENTED\n", mode);

    return Result::CELL_OK;
}

u64 CellGcmSys::cellGcmSetSecondVFrequency() {
    log("cellGcmSetSecondVFrequency() UNIMPLEMENTED\n");

    return Result::CELL_OK;
}

u64 CellGcmSys::cellGcmSetDebugOutputLevel() {
    const s32 level = ARG0;
    log("cellGcmSetDebugOutputLevel(level: %d)\n", level);

    return Result::CELL_OK;
}

u64 CellGcmSys::cellGcmMapEaIoAddressWithFlags() {
    const u32 ea = ARG0;
    const u32 io = ARG1;
    const u32 size = ARG2;
    const u32 flags = ARG3;
    log("cellGcmMapEaIoAddressWithFlags(ea: 0x%08x, io: 0x%08x, size: 0x%08x, flags: 0x%08x)\n", ea, io, size, flags);

    const u32 n_pages = size >> 20;
    log("Mapping %d pages\n", n_pages);
    for (u32 i = 0; i < n_pages; i++)
        mapEaIo(ea + (i << 20), io + (i << 20));
    mapping_sizes[io >> 20] = n_pages;

    //printOffsetTable();
    return Result::CELL_OK;
}

u64 CellGcmSys::cellGcmMapEaIoAddress() {
    const u32 ea = ARG0;
    const u32 io = ARG1;
    const u32 size = ARG2;
    log("cellGcmMapEaIoAddress(ea: 0x%08x, io: 0x%08x, size: 0x%08x)\n", ea, io, size);

    const u32 n_pages = size >> 20;
    log("Mapping %d pages\n", n_pages);
    for (u32 i = 0; i < n_pages; i++)
        mapEaIo(ea + (i << 20), io + (i << 20));
    mapping_sizes[io >> 20] = n_pages;

    return Result::CELL_OK;
}

u64 CellGcmSys::cellGcmGetFlipStatus() {
    log("cellGcmGetFlipStatus()\n");
    return flip;
}

u64 CellGcmSys::cellGcmGetReportDataAddressLocation() {
    const u32 idx = ARG0;
    const u32 loc = ARG1;
    // TODO: We just use a random, empty location for now... will fix when I implement reports
    log("cellGcmGetReportDataAddressLocation(idx: %d, loc: %d)\n", idx, loc);

    return reports_addr;
}

u64 CellGcmSys::cellGcmSetWaitFlip() {
    const u32 mode = ARG0;
    log("cellGcmSetWaitFlip(mode: 0x%08x)\n", mode);

    // A NOP because we don't have a multithreaded RSX.
    // When this function is called, the RSX will have already flipped.
    //ps3->thread_manager.getCurrentThread()->sleepForCycles(CPU_FREQ - ps3->curr_block_cycles - ps3->cycle_count);
    return Result::CELL_OK;
}

u64 CellGcmSys::cellGcmBindZcull() {
    const u8 idx = ARG0;
    const u32 offs = ARG1;
    const u32 width = ARG2;
    const u32 height = ARG3;
    const u32 cull_start = ARG4;
    const u32 z_format = ARG5;
    const u32 aa_format = ARG6;
    const u32 z_cull_dir = ARG7;
    //const u32 s_func = ARG8;
    //const u32 s_ref = ARG9;
    //const u32 s_mask = ARG10;
    log("cellGcmBindZcull() UNIMPLEMENTED\n");

    return Result::CELL_OK;
}

u64 CellGcmSys::cellGcmMapMainMemory() {
    const u32 ea = ARG0;
    const u32 size = ARG1;
    const u32 offs_ptr = ARG2;
    log("cellGcmMapMainMemory(ea: 0x%08x, size: 0x%08x, offs_ptr: 0x%08x)\n", ea, size, offs_ptr);

    // Find an area big enough to allocate enough pages to fit size bytes
    int free_pages = 0;
    u32 offs;
    for (offs = 0; offs < (RAM_SIZE >> 20); offs++) {
        if (!isIoOffsMapped(offs << 20))
            free_pages++;
        else
            free_pages = 0;

        if (free_pages == (size >> 20)) {
            offs -= free_pages - 1;
            const u32 n_pages = size >> 20;
            log("Mapping %d pages\n", n_pages);
            for (u32 i = 0; i < n_pages; i++)
                mapEaIo(ea + (i << 20), (offs + i) << 20);
            mapping_sizes[offs] = n_pages;
            break;
        }
    }
    //printOffsetTable();
    ps3->mem.write<u32>(offs_ptr, offs << 20);

    return Result::CELL_OK;
}

u64 CellGcmSys::cellGcmSetFlipHandler() {
    const u32 func_addr = ARG0;
    log("cellGcmSetFlipHandler(func_addr: 0x%08x)\n", func_addr);
    
    flip_callback = func_addr;
    return Result::CELL_OK;
}

u64 CellGcmSys::cellGcmSetDisplayBuffer() {
    const u8 buf_id = ARG0;
    const u32 offs = ARG1;
    const u32 pitch = ARG2;
    const u32 width = ARG3;
    const u32 height = ARG4;
    log("cellGcmSetDisplayBuffer(buf_id: %d, offs: 0x%08x, pitch: %d, width: %d, height: %d)\n", buf_id, offs, pitch, width, height);
    Helpers::debugAssert(buf_id < 8, "cellGcmSetDisplayBuffer: invalid buf_id (%d)\n", buf_id);

    CellGcmDisplayInfo* info = (CellGcmDisplayInfo*)ps3->mem.getPtr(buffer_info_addr + sizeof(CellGcmDisplayInfo) * buf_id);
    info->width = width;
    info->height = height;
    info->pitch = pitch;

    return Result::CELL_OK;
}

u64 CellGcmSys::cellGcmGetControlRegister() {
    //log("cellGcmGetControlRegister()\n");
    return ctrl_addr;
}

u64 CellGcmSys::cellGcmSetVBlankHandler() {
    const u32 handler_ptr = ARG0;
    log("cellGcmSetVBlankHandler(handler_ptr: 0x%08x)\n", handler_ptr);

    vblank_handler = handler_ptr;
    return Result::CELL_OK;
}

u64 CellGcmSys::cellGcmResetFlipStatus() {
    log("cellGcmResetFlipStatus()\n");

    flip = 1;

    return Result::CELL_OK;
}

u64 CellGcmSys::cellGcmSetDefaultCommandBuffer() {
    log("cellGcmSetDefaultCommandBuffer()\n");

    ctx = (CellGcmContextData*)ps3->mem.getPtr(ctx_addr);
    return Result::CELL_OK;
}

u64 CellGcmSys::cellGcmSetTileInfo() {
    const u8 idx = ARG0;
    const u8 location = ARG1;
    const u32 offs = ARG2;
    const u32 size = ARG3;
    const u32 pitch = ARG4;
    const u8 compression = ARG5;
    const u16 base = ARG6;
    const u8 bank = ARG7;
    log("cellGcmSetTileInfo(idx: %d, location: %d, offs: 0x%08x, size: 0x%08x, pitch: 0x%08x, compression: %d, base: 0x%04x, bank: %d) UNIMPLEMENTED\n", idx, location, offs, size, pitch, compression, base, bank);

    return Result::CELL_OK;
}

u64 CellGcmSys::cellGcmInitDefaultFifoMode() {
    const s32 mode = ARG0;
    log("cellGcmInitDefaultFifoMode(mode: %d)\n", mode);

    // TODO
    return Result::CELL_OK;
}

u64 CellGcmSys::cellGcmSetGraphicsHandler() {
    log("cellGcmSetGraphicsHandler() UNIMPLEMENTED\n");

    // TODO
    return Result::CELL_OK;
}

u64 CellGcmSys::cellGcmSetTile() {
    log("cellGcmSetTile() UNIMPLEMENTED\n");

    // TODO
    return Result::CELL_OK;
}

u64 CellGcmSys::cellGcmSetZcull() {
    log("cellGcmSetZcull() UNIMPLEMENTED\n");

    // TODO
    return Result::CELL_OK;
}

u64 CellGcmSys::cellGcmUnmapIoAddress() {
    const u32 io = ARG0;
    log("cellGcmUnmapIoAddress(io: 0x%08x)\n", io);

    u16* ea_table = (u16*)ps3->mem.getPtr(ea_table_ptr);
    u16* io_table = (u16*)ps3->mem.getPtr(io_table_ptr);
    const u32 ea = ea_table[io >> 20];
    const u32 n_pages = mapping_sizes[io >> 20];

    log("Unmapping %d pages\n", n_pages);
    for (u32 i = 0; i < n_pages; i++)
        unmapEaIo((ea + i) << 20, io + (i << 20));
    mapping_sizes[io >> 20] = 0;

    //printOffsetTable();
    return Result::CELL_OK;
}

u64 CellGcmSys::cellGcmSetFlip() {
    const u32 context_addr = ARG0;
    const u32 buf_id = ARG1;
    log("cellGcmSetFlip(ctx_addr: 0x%08x, buf_id: %d)\n", context_addr, buf_id);

    CellGcmContextData* context = (CellGcmContextData*)ps3->mem.getPtr(context_addr);
    if (ctx->current + 8 >= ctx->end) cellGcmCallback();
    ps3->mem.write<u32>(context->current, RSX::GCM_FLIP_COMMAND | (1 << 18));   // 1 is argc
    ps3->mem.write<u32>(context->current + 4, buf_id);
    context->current = context->current + 8;
    if (context_addr == ctx_addr) {
        ctrl->put = ctrl->put + 8;
        ps3->rsx.runCommandList();
    }

    return Result::CELL_OK;
}

u64 CellGcmSys::cellGcmGetConfiguration() {
    const u32 config_ptr = ARG0;
    log("cellGcmGetConfiguration(config_ptr: 0x%08x)\n", config_ptr);
    
    CellGcmConfig* config = (CellGcmConfig*)ps3->mem.getPtr(config_ptr);
    config->local_addr = gcm_config.local_addr;
    config->io_addr = gcm_config.io_addr;
    config->local_size = gcm_config.local_size;
    config->io_size = gcm_config.io_size;
    config->memFreq = gcm_config.memFreq;
    config->coreFreq = gcm_config.coreFreq;

    log("Got config:\n");
    log("ptr: 0x%08x\n", config_ptr);
    log("local_addr: 0x%08x\n", (u32)config->local_addr);
    log("io_addr   : 0x%08x\n", (u32)config->io_addr);
    log("local_size: 0x%08x\n", (u32)config->local_size);
    log("io_size   : 0x%08x\n", (u32)config->io_size);

    return Result::CELL_OK;
}

u64 CellGcmSys::cellGcmGetLabelAddress() {
    const u8 idx = ARG0;
    log("cellGcmGetLabelAddress(idx: 0x%02x)\n", idx);

    return label_addr + 0x10 * idx;
}

u64 CellGcmSys::cellGcmSetVBlankFrequency() {
    log("cellGcmSetVBlankFrequency() UNIMPLEMENTED\n");

    return Result::CELL_OK;
}


// Resets the command buffer
// If there are any remaining commands to be executed, copy them back at the start
// Update context and fifo control accordingly
u64 CellGcmSys::cellGcmCallback() {
    log("cellGcmCallback()\n");
    log("begin: 0x%08x, end: 0x%08x, current: 0x%08x\n", (u32)ctx->begin, (u32)ctx->end, (u32)ctx->current);
    log("get: 0x%08x, put: 0x%08x\n", (u32)ctrl->get, (u32)ctrl->put);

    const int bytes_queued = ctx->current - ctx->begin;
    const int bytes_remaining = bytes_queued - ctrl->put;

    if (bytes_remaining > 0)
        std::memcpy(ps3->mem.getPtr(ctx->begin), ps3->mem.getPtr(ctx->current) - bytes_remaining, bytes_remaining);

    ctx->current = ctx->begin + bytes_remaining;
    std::memset(ps3->mem.getPtr(ctx->current), 0, ctx->end - ctx->current);

    ctrl->put = bytes_remaining;
    ctrl->get = 0;

    return Result::CELL_OK;
}