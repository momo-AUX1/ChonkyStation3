#pragma once

#include <common.hpp>

#include <queue>

#include <Lv2Base.hpp>
#include <Lv2Objects/Lv2Mutex.hpp>


class Lv2RwLock : public virtual Lv2Base {
public:
    Lv2RwLock(Lv2Object* obj) : Lv2Base(obj) {}

    u64 read_id;
    u64 write_id;

    void initLocks();
    void rlock();
    void runlock();
    void wlock();
    void wunlock();
};