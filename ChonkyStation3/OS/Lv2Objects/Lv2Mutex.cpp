#include "Lv2Mutex.hpp"
#include "PlayStation3.hpp"


bool Lv2Mutex::isFree() {
    return owner == -1;
}

void Lv2Mutex::lock() {
    Thread* curr_thread = ps3->thread_manager.getCurrentThread();
    if (isFree())
        owner = curr_thread->id;
    else if (owner != curr_thread->id) {
        curr_thread->wait();
        wait_list.push(curr_thread->id);
        //printf("\"%s\" tried to lock mutex owned by \"%s\", waiting...\n", curr_thread->name.c_str(), ps3->thread_manager.getThreadByID(owner)->name.c_str());
    }
}

void Lv2Mutex::unlock() {
    Thread* curr_thread = ps3->thread_manager.getCurrentThread();
    if (isFree() || owner == curr_thread->id) {
        if (wait_list.size()) {
            // Wake up thread
            Thread* t = ps3->thread_manager.getThreadByID(wait_list.front());
            t->wakeUp();
            wait_list.pop();
            owner = t->id;
        } else
            owner = -1; // free
    }
    else
        Helpers::panic("Tried to unlock a mutex, but the current thread is not the mutex's owner (owner was thread %d \"%s\")\n", owner, ps3->thread_manager.getThreadByID(owner)->name.c_str());
}