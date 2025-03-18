#pragma once

#include <common.hpp>

#include <queue>

#include <Lv2Base.hpp>
#include <Lv2Objects/Lv2Mutex.hpp>


class Lv2LwCond : public virtual Lv2Base {
public:
    Lv2LwCond(Lv2Object* obj) : Lv2Base(obj) {}

    u32 lwcond_ptr = 0;
    std::string name;
    std::queue<u64> wait_list;
    bool signalled = false;

    bool signal();
    bool signalAll();
    bool wait();
};