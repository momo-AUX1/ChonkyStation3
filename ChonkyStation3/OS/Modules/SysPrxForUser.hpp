#pragma once

#include <common.hpp>
#include <logger.hpp>
#include <BEField.hpp>
#include <Loaders/SPU/SPULoader.hpp>

#include <CellTypes.hpp>

#include <chrono>


// Circular dependency
class PlayStation3;

using namespace CellTypes;

class SysPrxForUser {
public:
    SysPrxForUser(PlayStation3* ps3) : ps3(ps3) {}
    PlayStation3* ps3;

    u64 sysProcessExit();
    u64 sysProcessAtExitSpawn();
    u64 sysStrlen();
    u64 sysGetSystemTime();
    u64 sysProcess_At_ExitSpawn();
    u64 sysSpinlockInitialize();
    u64 sysStrcpy();
    u64 sysSpinlockLock();
    u64 sysSpinlockUnlock();
    u64 sysProcessIsStack();
    u64 sysPrintf();
    u64 sysStrcat();
    u64 sysStrncat();
    u64 sysStrncpy();
    u64 sysMemset();
    u64 sysMemcpy();
    u64 sysMalloc();
    u64 sysMemcmp();

    // sys_spu (forwarded to lv2 syscalls)
    u64 sys_spu_image_import();

private:
    MAKE_LOG_FUNCTION(log, sysPrxForUser);
};