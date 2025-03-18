#include "Lv2EventQueue.hpp"
#include "PlayStation3.hpp"


void Lv2EventQueue::receive() {
    Thread* curr_thread = ps3->thread_manager.getCurrentThread();
    if (!events.empty()) {
        ps3->ppu->state.gprs[4] = events.front().source;
        ps3->ppu->state.gprs[5] = events.front().data1;
        ps3->ppu->state.gprs[6] = events.front().data2;
        ps3->ppu->state.gprs[7] = events.front().data3;
        events.pop();
    }
    else {
        // Wait for an event
        curr_thread->wait();
        wait_list.push(curr_thread->id);
    }
}

void Lv2EventQueue::send(Event event) {
    if (wait_list.empty()) {
        events.push(event);
    }
    else {
        // TODO: is this the correct behavior (sending the event to all waiting threads)?
        while (!wait_list.empty()) {
            // Wake up thread
            Thread* t = ps3->thread_manager.getThreadByID(wait_list.front());
            t->wakeUp();
            wait_list.pop();

            // Return event info in gprs[4] - gprs[7]
            const auto curr_thread = ps3->thread_manager.getCurrentThread()->id;
            ps3->thread_manager.contextSwitch(*t);
            ps3->ppu->state.gprs[4] = event.source;
            ps3->ppu->state.gprs[5] = event.data1;
            ps3->ppu->state.gprs[6] = event.data2;
            ps3->ppu->state.gprs[7] = event.data3;
            ps3->thread_manager.contextSwitch(*ps3->thread_manager.getThreadByID(curr_thread));
        }
    }
}