#include "Lv2Base.hpp"
#include "Lv2Object.hpp"


Lv2Base::Lv2Base(Lv2Object* obj) : obj(obj) {
    this->ps3 = obj->ps3;
}

u64 Lv2Base::handle() {
    return obj->handle;
}