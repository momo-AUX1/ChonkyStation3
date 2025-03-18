#include "SysLwCond.hpp"
#include "PlayStation3.hpp"


u64 SysLwCond::sysLwCondWait() {
    const u32 lwcond_ptr = ARG0;
    const u64 timeout = ARG1;
    log("sysLwCondWait(lwcond_ptr: 0x%08x, timeout: %d)\n", lwcond_ptr, timeout);

    LwCond* cond = (LwCond*)ps3->mem.getPtr(lwcond_ptr);
    Lv2LwCond* lv2_lwcond = ps3->lv2_obj.get<Lv2LwCond>(cond->id);
    const auto res = lv2_lwcond->wait();
    if (!res) {
        log("WARNING: Tried to wait on a cond variable, but the current thread was not the mutex's owner\n");
        return Result::CELL_EPERM;
    }

    return Result::CELL_OK;
}

u64 SysLwCond::sysLwCondCreate() {
    const u32 lwcond_ptr = ARG0;
    const u32 lwmutex_ptr = ARG1;
    const u32 attr_ptr = ARG2;
    Lv2LwCond* lv2_lwcond = ps3->lv2_obj.create<Lv2LwCond>();
    log("sysLwCondCreate(lwcond_ptr: 0x%08x, lwmutex_ptr: 0x%08x, attr_ptr: 0x%08x)", lwcond_ptr, lwmutex_ptr, attr_ptr);

    LwCond* cond = (LwCond*)ps3->mem.getPtr(lwcond_ptr);
    LwCondAttr* attr = (LwCondAttr*)ps3->mem.getPtr(attr_ptr);
    
    cond->id = lv2_lwcond->handle();
    cond->lwmutex_ptr = lwmutex_ptr;
    lv2_lwcond->lwcond_ptr = lwcond_ptr;

    lv2_lwcond->name.resize(8);
    std::strncpy(lv2_lwcond->name.data(), (char*)attr->name, 8);
    logNoPrefix(" [name: %s]\n", lv2_lwcond->name.c_str());

    return Result::CELL_OK;
}

u64 SysLwCond::sysLwCondSignalAll() {
    const u32 lwcond_ptr = ARG0;
    log("sysLwCondSignalAll(lwcond_ptr: 0x%08x)\n", lwcond_ptr);

    LwCond* cond = (LwCond*)ps3->mem.getPtr(lwcond_ptr);
    Lv2LwCond* lv2_lwcond = ps3->lv2_obj.get<Lv2LwCond>(cond->id);
    if (!lv2_lwcond->signalAll()) return ps3->ppu->state.gprs[3];   // Locking failed and this thread went to sleep - do not update R3 yet

    return Result::CELL_OK;
}

u64 SysLwCond::sysLwCondSignal() {
    const u32 lwcond_ptr = ARG0;
    log("sysLwCondSignal(lwcond_ptr: 0x%08x)\n", lwcond_ptr);

    LwCond* cond = (LwCond*)ps3->mem.getPtr(lwcond_ptr);
    Lv2LwCond* lv2_lwcond = ps3->lv2_obj.get<Lv2LwCond>(cond->id);
    if (!lv2_lwcond->signal()) return ps3->ppu->state.gprs[3];   // Locking failed and this thread went to sleep - do not update R3 yet

    return Result::CELL_OK;
}