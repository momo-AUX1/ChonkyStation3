#include "Scheduler.hpp"


void Scheduler::tick(u64 cycles) {
    time += cycles;

    while (events.size() && time >= events.top().time) {
        auto event = events.top();
        events.pop();
        event.func();
    }
}

u64 Scheduler::tickToNextEvent() {
    if (events.empty()) {
        Helpers::panic("Tried to tick to the next scheduler event, but there are none in the queue\n");
    }

    u64 elapsed = events.top().time - time;
    time = events.top().time;
    auto event = events.top();
    events.pop();

    //printf("Skipped to event \"%s\"\n", event.name.c_str());
    event.func();

    return elapsed;
}

void Scheduler::push(std::function<void(void)> const& func, u64 time, std::string name) {
    Helpers::debugAssert(events.size() < schedulerMaxEntries, "Scheduler: queued more than %d scheduler events\n", schedulerMaxEntries);

    events.push({ func, this->time + time, name, next_event_id++});
}

void Scheduler::deleteAllEventsOfName(std::string name) {
    auto eventList = events.get_container();

    for (int i = 0; i < events.size(); i++) {
        for (auto& i : eventList) {
            if (i.name == name) events.remove(i);
        }
    }
}

u64 Scheduler::uSecondsToCycles(double us) {
    return CPU_FREQ * 0.000001 * us;
}