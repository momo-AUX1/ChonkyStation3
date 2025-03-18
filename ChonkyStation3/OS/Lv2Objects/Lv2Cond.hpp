#pragma once

#include <common.hpp>

#include <queue>

#include <Lv2Base.hpp>
#include <Lv2Objects/Lv2Mutex.hpp>


class Lv2Cond : public virtual Lv2Base {
public:
    Lv2Cond(Lv2Object* obj) : Lv2Base(obj) {}

    u32 mutex_id;
    std::queue<u64> wait_list;
    bool signalled = false;;

    bool signal();
    bool signalAll();
    bool wait();
};