#include <Syscall.hpp>
#include "PlayStation3.hpp"


MAKE_LOG_FUNCTION(log_sys_cond, sys_cond);

u64 Syscall::sys_cond_create() {
    const u32 cond_id_ptr = ARG0;
    const u32 mutex_id = ARG1;
    const u32 cond_attr_ptr = ARG2;
    log_sys_cond("sys_cond_create(cond_id_ptr: 0x%08x, mutex_id: %d, cond_attr_ptr: 0x%08x)\n", cond_id_ptr, mutex_id, cond_attr_ptr);

    Lv2Cond* cond = ps3->lv2_obj.create<Lv2Cond>();
    cond->mutex_id = mutex_id;
    ps3->mem.write<u32>(cond_id_ptr, cond->handle());

    return Result::CELL_OK;
}

u64 Syscall::sys_cond_wait() {
    const u32 cond_id = ARG0;
    const u64 timeout = ARG1;
    log_sys_cond("sys_cond_wait(cond_id: %d, timeout: %lld)\n", cond_id, timeout);

    Lv2Cond* cond = ps3->lv2_obj.get<Lv2Cond>(cond_id);
    cond->wait();

    return Result::CELL_OK;
}

u64 Syscall::sys_cond_signal() {
    const u32 cond_id = ARG0;
    log_sys_cond("sys_cond_signal(cond_id: %d)\n", cond_id);

    Lv2Cond* cond = ps3->lv2_obj.get<Lv2Cond>(cond_id);
    cond->signal();

    return Result::CELL_OK;
}

u64 Syscall::sys_cond_signal_all() {
    const u32 cond_id = ARG0;
    log_sys_cond("sys_cond_signal_all(cond_id: %d)\n", cond_id);

    Lv2Cond* cond = ps3->lv2_obj.get<Lv2Cond>(cond_id);
    cond->signalAll();

    return Result::CELL_OK;
}