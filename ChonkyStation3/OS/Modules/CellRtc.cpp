#include "CellRtc.hpp"
#include "PlayStation3.hpp"


u64 CellRtc::cellRtcGetCurrentClockLocalTime() {
    const u32 clock_ptr = ARG0;
    log("cellRtcGetCurrentClockLocalTime(clock_ptr: 0x%08x)\n");

    CellRtcDateTime* clock = (CellRtcDateTime*)ps3->mem.getPtr(clock_ptr);
    
    const auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    // Convert to local time
    std::tm local_tm;
    #if defined(_MSC_VER) || defined(__MINGW32__) || defined(__XBOX_BUILD)
    localtime_s(&local_tm, &now_time);
    #else
    localtime_r(&now_time, &local_tm);
    #endif

    const int year      = local_tm.tm_year + 1900;
    const int month     = local_tm.tm_mon  + 1;
    const int day       = local_tm.tm_mday;
    const int hour      = local_tm.tm_hour;
    const int minute    = local_tm.tm_min;
    const int second    = local_tm.tm_sec;
    const auto us       = (std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000000).count();

    clock->day = day;
    clock->month = month;
    clock->year = year;
    clock->hour = hour;
    clock->minute = minute;
    clock->second = second;
    clock->microsecond = us;

    log("Time is %d/%d/%d %d:%d:%d:%d\n", (u32)clock->day, (u32)clock->month, (u32)clock->year, (u32)clock->hour, (u32)clock->minute, (u32)clock->second, (u32)clock->microsecond);
    return Result::CELL_OK;
}

u64 CellRtc::cellRtcGetCurrentTick() {
    const u32 tick_ptr = ARG0;
    log("cellRtcGetCurrentTick(tick_ptr: 0x%08x)\n", tick_ptr);

    ps3->mem.write<u64>(tick_ptr, ps3->scheduler.time);
    
    return Result::CELL_OK;
}

u64 CellRtc::cellRtcGetTime_t() {
    const u32 clock_ptr = ARG0;
    const u32 time_ptr = ARG1;  // time is s64

    CellRtcDateTime* clock = (CellRtcDateTime*)ps3->mem.getPtr(clock_ptr);
    // TODO: This is an approximation
    // We are supposed to turn clock into a time_t, but because this function is probably going to be called
    // right after getting the current time, we can just get the current tick again here with std::chrono
    ps3->mem.write<u64>(time_ptr, std::chrono::system_clock::now().time_since_epoch().count() / 1000000);

    return Result::CELL_OK;
}