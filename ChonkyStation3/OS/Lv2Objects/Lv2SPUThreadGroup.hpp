#pragma once

#include <common.hpp>

#include <queue>

#include <Lv2Base.hpp>
#include <OS/Syscalls/sys_spu.hpp>


using namespace sys_spu;

class Lv2SPUThreadGroup : public virtual Lv2Base {
public:
    Lv2SPUThreadGroup(Lv2Object* obj) : Lv2Base(obj) {}

    sys_spu_thread_group_attribute* attr;
    std::vector<u32> threads;
    bool started = false;
    u32 run_event_queue_id = 0;

    bool start();
    std::string getName();
};