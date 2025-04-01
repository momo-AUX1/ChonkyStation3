#pragma once

#include <common.hpp>
#include <BEField.hpp>


// We break our naming conventions here to follow CellOS's instead
namespace sys_spu {

static constexpr u32 SYS_SPU_THREAD_EVENT_USER = 1;

static constexpr u32 SYS_SPU_THREAD_EVENT_USER_KEY = 0xFFFFFFFF53505501;

static constexpr u32 SYS_SPU_THREAD_GROUP_EVENT_RUN             = 1;
static constexpr u32 SYS_SPU_THREAD_GROUP_EVENT_EXCEPTION       = 2;
static constexpr u32 SYS_SPU_THREAD_GROUP_EVENT_SYSTEM_MODULE   = 4;

static constexpr u64 SYS_SPU_THREAD_GROUP_EVENT_RUN_KEY             = 0xFFFFFFFF53505500;
static constexpr u64 SYS_SPU_THREAD_GROUP_EVENT_EXCEPTION_KEY       = 0xFFFFFFFF53505503;
static constexpr u64 SYS_SPU_THREAD_GROUP_EVENT_SYSTEM_MODULE_KEY   = 0xFFFFFFFF53505504;

static constexpr s32 SYS_SPU_SEGMENT_TYPE_COPY = 0x0001;
static constexpr s32 SYS_SPU_SEGMENT_TYPE_FILL = 0x0002;
static constexpr s32 SYS_SPU_SEGMENT_TYPE_INFO = 0x0004;

struct sys_spu_segment {
    BEField<s32> type;       // Segment type (COPY / FILL / INFO)
    BEField<u32> ls_addr;    // Addr in LS that this segment will be loaded to
    BEField<s32> size;
    union {
        BEField<u32> addr;   // Address of the segment in memory, for COPY type segments
        BEField<u32> val;    // The value to fill the segment with, for FILL type segments
        BEField<u64> pad;
    } src;
};

struct sys_spu_image {
    BEField<u32> type;
    BEField<u32> entry;
    BEField<u32> segs_ptr;  // segs is sys_spu_segment[n_segs]
    BEField<u32> n_segs;
};

struct sys_spu_thread_group_attribute {
    BEField<u32> name_len;
    BEField<u32> name_ptr;
    BEField<u32> type;
    BEField<u32> ct;    // Memory container id
};

struct sys_spu_thread_attribute {
    BEField<u32> name_ptr;
    BEField<u32> name_len;
    BEField<u32> option;
};

struct sys_spu_thread_argument {
    BEField<u64> arg1;
    BEField<u64> arg2;
    BEField<u64> arg3;
    BEField<u64> arg4;
};

}   // End namespace sys_spu