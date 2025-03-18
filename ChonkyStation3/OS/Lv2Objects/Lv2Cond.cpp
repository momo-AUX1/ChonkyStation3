#include "Lv2Cond.hpp"
#include "PlayStation3.hpp"


bool Lv2Cond::signal() {
    Lv2Mutex* mtx = ps3->lv2_obj.get<Lv2Mutex>(mutex_id);

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

bool Lv2Cond::signalAll() {
    Lv2Mutex* mtx = ps3->lv2_obj.get<Lv2Mutex>(mutex_id);

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

bool Lv2Cond::wait() {
    if (signalled) {
        signalled = false;
    }
    else {
        Lv2Mutex* mtx = ps3->lv2_obj.get<Lv2Mutex>(mutex_id);
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