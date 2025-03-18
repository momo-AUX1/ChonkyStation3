#pragma once

#include <common.hpp>


// Circular dependency
class Lv2Object;
class PlayStation3;

class Lv2Base {
public:
    Lv2Base(Lv2Object* obj);

    Lv2Object* obj;
    PlayStation3* ps3;

    u64 handle();
};