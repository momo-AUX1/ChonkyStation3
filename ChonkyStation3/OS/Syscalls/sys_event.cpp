#include <Syscall.hpp>
#include "PlayStation3.hpp"
#include <Lv2Objects/Lv2EventQueue.hpp>


MAKE_LOG_FUNCTION(log_sys_event, sys_event);

u64 Syscall::sys_event_queue_create() {
    const u32 queue_id_ptr = ARG0;
    const u32 attr_ptr = ARG1;
    const u64 ipc_key = ARG2;
    const s32 size = ARG3;
    log_sys_event("sys_event_queue_create(queue_id_ptr: 0x%08x, attr_ptr: 0x%08x, ipc_key: 0x%016x, size: 0x%08x)\n", queue_id_ptr, attr_ptr, ipc_key, size);

    Lv2EventQueue* queue = ps3->lv2_obj.create<Lv2EventQueue>();
    queue->size = size;

    ps3->mem.write<u32>(queue_id_ptr, queue->handle());
    return Result::CELL_OK;
}

u64 Syscall::sys_event_queue_receive() {
    const u32 queue_id = ARG0;
    const u32 event_ptr = ARG1; // Unused
    const u64 timeout = ARG2;
    log_sys_event("sys_event_queue_receive(queue_id: %d, event_ptr: 0x%08x, timeout: %d)\n", queue_id, event_ptr, timeout );

    Lv2EventQueue* queue = ps3->lv2_obj.get<Lv2EventQueue>(queue_id);
    queue->receive();    // queue->receive will return the event data in gprs[4] - gprs[7]

    return Result::CELL_OK;
}

u64 Syscall::sys_event_port_create() {
    const u32 port_id_ptr = ARG0;
    const s32 port_type = ARG1;
    const u64 name = ARG2;
    log_sys_event("sys_event_port_create(port_id_ptr: 0x%08x, port_type: %d, name: 0x%016x) STUBBED\n", port_id_ptr, port_type, name);

    ps3->mem.write<u32>(port_id_ptr, ps3->handle_manager.request());
    return Result::CELL_OK;
}

u64 Syscall::sys_event_port_connect_local() {
    const u32 port_id = ARG0;
    const u32 queue_id = ARG1;
    log_sys_event("sys_event_port_connect_local(port_id: %d, queue_id: %d) STUBBED\n", port_id, queue_id);

    return Result::CELL_OK;
}