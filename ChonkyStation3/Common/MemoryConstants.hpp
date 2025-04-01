#pragma once


static constexpr u64 PAGE_BITS = 16;
static constexpr u64 PAGE_SHIFT = (32 - PAGE_BITS);
static constexpr u64 PAGE_COUNT = 1 << PAGE_BITS;
static constexpr u64 PAGE_SIZE = 1 << PAGE_SHIFT;
static constexpr u64 PAGE_MASK = PAGE_SIZE - 1;
static constexpr u64 RAM_SIZE = 256_MB;
static constexpr u64 RAM_START = 0x00010000;
static constexpr u64 RAM_END = RAM_START + RAM_SIZE;
static constexpr u64 STACK_REGION_START = 0xD0000000;
static constexpr u64 STACK_REGION_SIZE = 0x10000000;
static constexpr u64 DEFAULT_STACK_SIZE = 0x40000;
static constexpr u64 RSX_VIDEO_MEM_SIZE = 256_MB;
static constexpr u64 RSX_VIDEO_MEM_START = 0xC0000000;
static constexpr u64 RSX_VIDEO_MEM_END = RSX_VIDEO_MEM_START + RSX_VIDEO_MEM_SIZE;
static constexpr u64 SPU_MEM_SIZE = 512_KB; // Should be 256KB (size of spu local storage I think?), but things break if I make it 256KB. I'll figure it out once I implement SPUs.
static constexpr u64 SPU_MEM_START = 0xE0000000;
static constexpr u64 SPU_MEM_END = RSX_VIDEO_MEM_START + RSX_VIDEO_MEM_SIZE;