#include "Lv2SPUThreadGroup.hpp"
#include "Lv2EventQueue.hpp"
#include "PlayStation3.hpp"


// Returns true if the group was started successfully, false otherwise
bool Lv2SPUThreadGroup::start() {
    if (started) return false;

    // TODO: Check if all the threads in this group are ready
    for (auto& i : threads) {
        ps3->spu_thread_manager.getThreadByID(i)->status = SPUThread::ThreadStatus::Running;
        if (ps3->spu_thread_manager.getThreadByID(i)->name != "highCellSpursKernel5") ps3->spu_thread_manager.getThreadByID(i)->status = SPUThread::ThreadStatus::Terminated;
    }

    ps3->spu_thread_manager.reschedule();
    started = true;

    // Send run event
    if (run_event_queue_id) {
        Lv2EventQueue* queue = (Lv2EventQueue*)ps3->lv2_obj.get<Lv2EventQueue>(run_event_queue_id);
        queue->send({ SYS_SPU_THREAD_GROUP_EVENT_RUN_KEY, handle(), 0, 0 });
    }
    return true;
}

std::string Lv2SPUThreadGroup::getName() {
    return Helpers::readString(ps3->mem.getPtr(attr->name_ptr));
}