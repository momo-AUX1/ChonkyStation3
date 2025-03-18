#pragma once

#include <common.hpp>

#include <Lv2Object.hpp>
#include <Lv2Base.hpp>
#include <HandleManager.hpp>


// Circular dependency
class PlayStation3;

class Lv2ObjectManager {
public:
    Lv2ObjectManager(PlayStation3* ps3, HandleManager* handle_manager) : ps3(ps3), handle_manager(handle_manager) {}
    PlayStation3* ps3;
    HandleManager* handle_manager;

    template<typename T> requires std::is_base_of_v<Lv2Base, T>
    T* create() {
        const auto handle = handle_manager->request();
        Lv2Object new_obj = Lv2Object(handle, ps3);
        objs.push_back(new_obj);
        objs.back().create<T>();

        log("Created obj with handle %d\n", handle);
        return objs.back().get<T>();
    };

    template<typename T> requires std::is_base_of_v<Lv2Base, T>
    T* get(u64 handle) {
        for (auto& i : objs) {
            if (i.handle == handle)
                return i.get<T>();
        }
        Helpers::panic("Object with handle %d does not exist\n", handle);
    }

    std::vector<Lv2Object> objs;

private:
    MAKE_LOG_FUNCTION(log, lv2_obj);
};