#include "Lv2RwLock.hpp"
#include "PlayStation3.hpp"


void Lv2RwLock::initLocks() {
    read_id = ps3->lv2_obj.create<Lv2Mutex>()->handle();
    write_id = ps3->lv2_obj.create<Lv2Mutex>()->handle();
}

void Lv2RwLock::rlock() {
    Lv2Mutex* read_mtx = ps3->lv2_obj.get<Lv2Mutex>(read_id);
    read_mtx->lock();
}

void Lv2RwLock::runlock() {
    Lv2Mutex* read_mtx = ps3->lv2_obj.get<Lv2Mutex>(read_id);
    read_mtx->unlock();
}

void Lv2RwLock::wlock() {
    Lv2Mutex* write_mtx = ps3->lv2_obj.get<Lv2Mutex>(write_id);
    write_mtx->lock();
}

void Lv2RwLock::wunlock() {
    Lv2Mutex* write_mtx = ps3->lv2_obj.get<Lv2Mutex>(write_id);
    write_mtx->unlock();
}