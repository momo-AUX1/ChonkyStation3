#include "Lv2Semaphore.hpp"
#include "PlayStation3.hpp"


void Lv2Semaphore::post(u32 val) {
    this->val = val;

    while (wait_list.size()) {
        if (val == 0) break;
        // Wake up thread
        Thread* t = ps3->thread_manager.getThreadByID(wait_list.front());
        t->wakeUp();
        wait_list.pop();
        // Decrement semaphore
        val--;
    }
}

void Lv2Semaphore::wait() {
    if (val > 0)
        val--;
    else {
        ps3->thread_manager.getCurrentThread()->wait();
        wait_list.push(ps3->thread_manager.getCurrentThread()->id);
    } 
}