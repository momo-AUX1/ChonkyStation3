#pragma once

#include <common.hpp>

#include <queue>

#include <Lv2Base.hpp>


class Lv2EventQueue : public virtual Lv2Base {
public:
    Lv2EventQueue(Lv2Object* obj) : Lv2Base(obj) {}

    struct Event {
        u64 source;
        u64 data1;
        u64 data2;
        u64 data3;
    };

    u32 size = 0;
    std::queue<u64> wait_list;
    std::queue<Event> events;

    void receive();
    void send(Event event);
};