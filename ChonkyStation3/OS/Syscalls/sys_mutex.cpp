#include <Syscall.hpp>
#include "PlayStation3.hpp"


MAKE_LOG_FUNCTION(log_sys_mutex, sys_mutex);

u64 Syscall::sys_mutex_create() {
    const u32 mutex_id_ptr = ARG0;
    const u32 attr_ptr = ARG1;
    log_sys_mutex("sys_mutex_create(mutex_id_ptr: 0x%08x, attr_ptr: 0x%08x)\n", mutex_id_ptr, attr_ptr);

    Lv2Mutex* mutex = ps3->lv2_obj.create<Lv2Mutex>();
    ps3->mem.write<u32>(mutex_id_ptr, mutex->handle());

    return Result::CELL_OK;
}

u64 Syscall::sys_mutex_lock() {
    const u32 mutex_id = ARG0;
    const u64 timeout = ARG1;
    log_sys_mutex("sys_mutex_lock(mutex_idx: %d, timeout: %d)\n", mutex_id, timeout);

    Lv2Mutex* mutex = ps3->lv2_obj.get<Lv2Mutex>(mutex_id);
    mutex->lock();

    return Result::CELL_OK;
}

u64 Syscall::sys_mutex_unlock() {
    const u32 mutex_id = ARG0;
    log_sys_mutex("sys_mutex_unlock(mutex_id: %d)\n", mutex_id);

    Lv2Mutex* mutex = ps3->lv2_obj.get<Lv2Mutex>(mutex_id);
    mutex->unlock();

    return Result::CELL_OK;
}