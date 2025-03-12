#include "SPUThreadManager.hpp"
#include "PlayStation3.hpp"


SPUThread* SPUThreadManager::createThread(std::string name) {
    threads.push_back({ ps3, name });
    return &threads.back();
}

void SPUThreadManager::contextSwitch(SPUThread& thread) {
    ps3->spu->enabled = true;
    SPUThread* current_thread = getCurrentThread();
    if (current_thread && current_thread->id == thread.id) return;

    if (current_thread)
        log("Switched from thread %d \"%s\" to thread %d \"%s\"\n", current_thread->id, current_thread->name.c_str(), thread.id, thread.name.c_str());
    else
        log("Switched to thread \"%s\"\n", thread.name.c_str());

    if (current_thread)
        current_thread->state = ps3->spu->state;
    
    ps3->spu->state = thread.state;
    ps3->spu->ls = thread.ls;
    current_thread_id = thread.id;
}

SPUThread* SPUThreadManager::getCurrentThread() {
    return getThreadByID(current_thread_id);
}

SPUThread* SPUThreadManager::getThreadByID(u32 id) {
    SPUThread* thread = nullptr;
    for (auto& i : threads) {
        if (i.id == id) {
            thread = &i;
            break;
        }
    }
    return thread;
}

void SPUThreadManager::reschedule() {
    // No thread is currently active - find the first running thread and execute it
    if (current_thread_id == 0) {
        for (auto& i : threads) {
            if (i.status == SPUThread::ThreadStatus::Running) {
                contextSwitch(i);
                break;
            }
        }
        return;
    }

    // Find index of current thread
    int curr_thread = 0;
    while (threads[curr_thread].id != getCurrentThread()->id) curr_thread++;

    // Try to switch to the next running thread
    bool found_thread = false;
    for (int i = 0; i < threads.size(); i++) {
        SPUThread& t = threads[(curr_thread + 1 + i) % threads.size()];    // +1 because we want to begin searching from the thread after the current thread

        if (t.status == SPUThread::ThreadStatus::Running) {
            contextSwitch(t);
            found_thread = true;
            break;
        }
    }
    
    // No available threads, disable SPU
    if (!found_thread) {
        ps3->spu->enabled = false;
        current_thread_id = 0;
    }
}