#include "Lv2LwCond.hpp"
#include "PlayStation3.hpp"


bool Lv2LwCond::signal() {
    SysLwCond::LwCond* lwcond = (SysLwCond::LwCond*)ps3->mem.getPtr(lwcond_ptr);
    SysLwMutex::LwMutex* lwmutex = (SysLwMutex::LwMutex*)ps3->mem.getPtr(lwcond->lwmutex_ptr);
    Lv2Mutex* mtx = ps3->lv2_obj.get<Lv2Mutex>(lwmutex->sleep_queue);

    if (!wait_list.empty()) {
        // Wake up thread
        Thread* t = ps3->thread_manager.getThreadByID(wait_list.front());
        t->wakeUp();
        wait_list.pop();
        // Make the thread reacquire the mutex
        // Temporarily switch to the other thread to lock the mutex
        const auto curr_thread = ps3->thread_manager.getCurrentThread()->id;
        ps3->thread_manager.contextSwitch(*t);
        mtx->lock();
        ps3->thread_manager.contextSwitch(*ps3->thread_manager.getThreadByID(curr_thread));
    }
    else {
        //signalled = true;
    }

    return true;
}

bool Lv2LwCond::signalAll() {
    SysLwCond::LwCond* lwcond = (SysLwCond::LwCond*)ps3->mem.getPtr(lwcond_ptr);
    SysLwMutex::LwMutex* lwmutex = (SysLwMutex::LwMutex*)ps3->mem.getPtr(lwcond->lwmutex_ptr);
    Lv2Mutex* mtx = ps3->lv2_obj.get<Lv2Mutex>(lwmutex->sleep_queue);

    while (!wait_list.empty()) {
        // Wake up thread
        Thread* t = ps3->thread_manager.getThreadByID(wait_list.front());
        t->wakeUp();
        wait_list.pop();

        // Make the thread reacquire the mutex
        // Temporarily switch to the other thread to lock the mutex
        const auto curr_thread = ps3->thread_manager.getCurrentThread()->id;
        ps3->thread_manager.contextSwitch(*t);
        mtx->lock();
        ps3->thread_manager.contextSwitch(*ps3->thread_manager.getThreadByID(curr_thread));
    }

    return true;
}

bool Lv2LwCond::wait() {
    if (signalled) {
        signalled = false;
    }
    else {
        SysLwCond::LwCond* lwcond = (SysLwCond::LwCond*)ps3->mem.getPtr(lwcond_ptr);
        SysLwMutex::LwMutex* lwmutex = (SysLwMutex::LwMutex*)ps3->mem.getPtr(lwcond->lwmutex_ptr);
        Lv2Mutex* mtx = ps3->lv2_obj.get<Lv2Mutex>(lwmutex->sleep_queue);
        Thread* curr_thread = ps3->thread_manager.getCurrentThread();

        // Check if the current thread is the owner of the mutex
        if (mtx->owner != curr_thread->id)
            return false;

        curr_thread->wait();
        wait_list.push(curr_thread->id);

        // Release the mutex while we are waiting
        mtx->unlock();

        //printf("Thread %d \"%s\" is waiting on cond variable \"%s\"...\n", curr_thread->id, curr_thread->name.c_str(), name.c_str());
    }

    return true;
}