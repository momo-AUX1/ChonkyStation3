#pragma once

#include <common.hpp>
#include <logger.hpp>
#include <BEField.hpp>

#include <CellTypes.hpp>


// Circular dependency
class PlayStation3;

using namespace CellTypes;

static constexpr u32 CELL_SYSCACHE_ID_SIZE  = 32;
static constexpr u32 CELL_SYSCACHE_PATH_MAX = 1055;

class CellSysCache {
public:
    CellSysCache(PlayStation3* ps3) : ps3(ps3) {}
    PlayStation3* ps3;

    struct CellSysCacheParam {
        u8 cache_id[CELL_SYSCACHE_ID_SIZE];
        u8 cache_path[CELL_SYSCACHE_PATH_MAX];
        BEField<u32> reserved;
    };

    fs::path cache_path;

    u64 cellSysCacheMount();
    u64 cellSysCacheClear();

private:
    MAKE_LOG_FUNCTION(log, cellSysCache);
};